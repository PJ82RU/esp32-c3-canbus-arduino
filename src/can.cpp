#include "canbus/can.h"
#include <esp32-hal-log.h>

namespace canbus
{
    void canWatchdogTask(void* params)
    {
        auto* can = static_cast<Can*>(params);
        constexpr TickType_t delay = 200 / portTICK_PERIOD_MS;
        while (true)
        {
            vTaskDelay(delay);
            can->handleWatchdog();
        }
    }

    void canReceiveTask(void* params)
    {
        const auto* can = static_cast<Can*>(params);
        while (true)
        {
            if (!can->handleReceive())
            {
                vTaskDelay(pdMS_TO_TICKS(CAN_RECEIVE_MS_TO_TICKS));
            }
        }
    }

    Can::Can(gpio_num_t txPin, gpio_num_t rxPin)
        : mWatchdogThread("CAN_WATCHDOG", 2048, 10),
          mReceiveThread("CAN_RECEIVE", 4096, 19),
          mSemaphore(true)
    {
        mDriverConfig = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
        mTimingConfig = TWAI_TIMING_CONFIG_125KBITS();
        mFilterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        clearFilters();
    }

    Can::~Can()
    {
        end();
    }

    bool Can::installAndStartDriver()
    {
        if (mDriverReady)
        {
            log_w("Driver already installed");
            return true;
        }

        if (twai_driver_install(&mDriverConfig, &mTimingConfig, &mFilterConfig) != ESP_OK)
        {
            log_e("Failed to install TWAI driver");
            return false;
        }

        if (twai_start() != ESP_OK)
        {
            log_e("Failed to start TWAI driver");
            twai_driver_uninstall();
            return false;
        }

        mDriverReady = true;
        log_i("TWAI driver started successfully");
        return true;
    }

    void Can::stopAndUninstallDriver()
    {
        if (!mDriverReady) return;

        mDriverReady = false;
        twai_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
        twai_driver_uninstall();
        log_i("TWAI driver stopped and uninstalled");
    }

    bool Can::begin(pj_tools::Callback* callback)
    {
        if (callback == nullptr || !mSemaphore.take()) return false;

        bool result = false;
        if (mDriverConfig.tx_io != GPIO_NUM_NC && mDriverConfig.rx_io != GPIO_NUM_NC)
        {
            if (mDriverReady) stopAndUninstallDriver();

            setSpeed(mSpeed);
            mCallback = callback;
            result = installAndStartDriver() &&
                mReceiveThread.start(&canReceiveTask, this) &&
                mWatchdogThread.start(&canWatchdogTask, this);
        }
        else
        {
            log_w("Invalid GPIO configuration");
        }

        (void)mSemaphore.give();
        return result;
    }

    void Can::end()
    {
        if (!mSemaphore.take()) return;

        if (mDriverReady)
        {
            mWatchdogThread.stop();
            mReceiveThread.stop();
            stopAndUninstallDriver();
        }

        (void)mSemaphore.give();
    }

    twai_state_t Can::getState() const
    {
        return mStatusInfo.state;
    }

    bool Can::waitRunning(const unsigned long timeout) const
    {
        const unsigned long stopTime = millis() + timeout;
        bool isRunning = false;

        while (!((isRunning = (mStatusInfo.state == TWAI_STATE_RUNNING))))
        {
            if (timeout != 0 && millis() >= stopTime) break;
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        return isRunning;
    }

    void Can::setSpeed(const CanSpeed speed)
    {
        if (!mSemaphore.take()) return;

        mSpeed = speed;
        switch (speed)
        {
        case CanSpeed::SPEED_25KBIT: mTimingConfig = TWAI_TIMING_CONFIG_25KBITS();
            break;
        case CanSpeed::SPEED_50KBIT: mTimingConfig = TWAI_TIMING_CONFIG_50KBITS();
            break;
        case CanSpeed::SPEED_100KBIT: mTimingConfig = TWAI_TIMING_CONFIG_100KBITS();
            break;
        case CanSpeed::SPEED_125KBIT: mTimingConfig = TWAI_TIMING_CONFIG_125KBITS();
            break;
        case CanSpeed::SPEED_250KBIT: mTimingConfig = TWAI_TIMING_CONFIG_250KBITS();
            break;
        case CanSpeed::SPEED_500KBIT: mTimingConfig = TWAI_TIMING_CONFIG_500KBITS();
            break;
        case CanSpeed::SPEED_800KBIT: mTimingConfig = TWAI_TIMING_CONFIG_800KBITS();
            break;
        case CanSpeed::SPEED_1MBIT: mTimingConfig = TWAI_TIMING_CONFIG_1MBITS();
            break;
        }
        log_i("CAN speed set to %d kbit", static_cast<int>(speed));

        (void)mSemaphore.give();
    }

    int Can::setFilter(const uint8_t index,
                       const uint32_t id,
                       const uint32_t mask,
                       const bool extended,
                       const int16_t callbackIndex)
    {
        if (index >= CAN_NUM_FILTER || !mSemaphore.take()) return -1;

        const auto filter = &mFilters[index];
        filter->configured = true;
        filter->extended = extended;
        filter->id = id & mask;
        filter->mask = mask;
        filter->callbackIndex = callbackIndex;
        log_d("Filter %d set: id=0x%X, mask=0x%X", index, id, mask);

        (void)mSemaphore.give();
        return index;
    }

    int Can::setFilter(const uint32_t id, const uint32_t mask, const bool extended, const int16_t callbackIndex)
    {
        for (int i = 0; i < CAN_NUM_FILTER; i++)
        {
            if (!mFilters[i].configured)
            {
                return setFilter(i, id, mask, extended, callbackIndex);
            }
        }
        log_w("No free filters available");
        return -1;
    }

    CanFilter Can::getFilter(const int16_t index) const
    {
        return (index >= 0 && index < CAN_NUM_FILTER) ? mFilters[index] : CanFilter{};
    }

    void Can::clearFilters()
    {
        if (!mSemaphore.take()) return;

        for (auto& filter : mFilters)
        {
            filter = CanFilter{};
        }
        log_i("All filters cleared");

        (void)mSemaphore.give();
    }

    void Can::processFrame(const twai_message_t& message) const
    {
        if (mCallback == nullptr) return;

        for (int i = 0; i < CAN_NUM_FILTER; i++)
        {
            const auto& filter = mFilters[i];
            if (filter.configured &&
                (message.identifier & filter.mask) == filter.id &&
                message.extd == filter.extended)
            {
                CanFrame frame;
                frame.id = message.identifier;
                frame.length = message.data_length_code;
                frame.rtr = message.rtr;
                frame.extended = message.extd;
                frame.filterIndex = i;
                memcpy(frame.data.bytes, message.data, CAN_FRAME_DATA_SIZE);

                mCallback->invoke(&frame, i);
                log_d("Frame 0x%X processed by filter %d", message.identifier, i);
                return;
            }
        }

        // Обработка кадров, не прошедших фильтрацию
        CanFrame frame;
        frame.id = message.identifier;
        frame.length = message.data_length_code;
        frame.rtr = message.rtr;
        frame.extended = message.extd;
        frame.filterIndex = -1;
        memcpy(frame.data.bytes, message.data, CAN_FRAME_DATA_SIZE);

        mCallback->invoke(&frame);
        log_d("Frame 0x%X received (no filter)", message.identifier);
    }

    bool Can::send(CanFrame& frame) const
    {
        if (!frame.hasData())
        {
            log_w("Invalid frame data");
            return false;
        }

        if (!mSemaphore.take()) return false;

        bool result = false;
        if (mDriverReady && mStatusInfo.state == TWAI_STATE_RUNNING)
        {
            const unsigned long currentTime = millis();
            if (frame.nextSendTime <= currentTime)
            {
                if (frame.frequency != 0)
                {
                    frame.nextSendTime = currentTime + frame.frequency;
                }

                twai_message_t message;
                message.identifier = frame.id;
                message.data_length_code = frame.length;
                message.rtr = frame.rtr;
                message.extd = frame.extended;
                memcpy(message.data, frame.data.bytes, CAN_FRAME_DATA_SIZE);

                const esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(CAN_SEND_MS_TO_TICKS));
                if (err == ESP_OK)
                {
                    log_d("Frame 0x%X sent successfully", frame.id);
                    result = true;
                }
                else
                {
                    log_w("Failed to send frame 0x%X: %d", frame.id, err);
                }
            }
            else
            {
                log_d("Frame 0x%X send delayed", frame.id);
            }
        }
        else
        {
            log_w("CAN interface not ready");
        }

        (void)mSemaphore.give();
        return result;
    }

    bool Can::receive(CanFrame& frame) const
    {
        return mCallback != nullptr && mCallback->read(&frame);
    }

    void Can::handleWatchdog()
    {
        if (twai_get_status_info(&mStatusInfo) == ESP_OK &&
            mStatusInfo.state == TWAI_STATE_BUS_OFF)
        {
            if (twai_initiate_recovery() != ESP_OK)
            {
                log_w("Bus recovery failed");
            }
        }
    }

    bool Can::handleReceive() const
    {
        if (!mDriverReady) return false;

        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(CAN_RECEIVE_MS_TO_TICKS)) == ESP_OK)
        {
            processFrame(message);
        }
        return true;
    }

    void Can::onResponse(void* value, void* params)
    {
        auto* frame = static_cast<CanFrame*>(value);
        const auto* can = static_cast<Can*>(params);
        can->send(*frame);
    }
} // namespace hardware
