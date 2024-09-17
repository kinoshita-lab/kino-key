/**
 * @file ShiftregisterSwScanner.h
 * @brief
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2024 Kinoshita Lab. All rights reserved.
 *
 */

#pragma once
#ifndef SHIFTREGISTERSWSCANNER_H
#define SHIFTREGISTERSWSCANNER_H

#include <Arduino.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace kinoshita_lab
{

class ShiftregisterSwScanner25Keys
{
public:
    typedef void (*SwitchHandler)(uint32_t switch_index, const int off_on);
    enum
    {
        DEFAULT_NUM_SWITCH_EACH  = 16,
        DEFAULT_SCAN_PERIOD_MSEC = 4,
    };

    enum InternalState
    {
        Init,
        LoadStart,
        ReadEachBits,
        WaitNext,
        UnknownState = 0xff,
    };

    ShiftregisterSwScanner25Keys(
        const uint8_t npl_pin, const uint8_t clock_pin, const uint8_t output_pin1,
        const uint8_t output_pin2, SwitchHandler handler = nullptr,
        const size_t num_switches_each = DEFAULT_NUM_SWITCH_EACH,
        const unsigned int scan_period = DEFAULT_SCAN_PERIOD_MSEC)
        : pins_(npl_pin, clock_pin, output_pin1, output_pin2), handler_(handler),
          num_switches_(num_switches_each)
    {

        // initialize switch status
        for (auto j = 0u; j < 2; ++j) {
            for (auto i = 0u; i < num_switches_; ++i) {
                scan_buffers_[j][i]        = 1;
                former_scan_buffers_[j][i] = 1;
                switch_status_[j][i]       = 1;
            }
        }

        pinMode(npl_pin, OUTPUT);
        pinMode(clock_pin, OUTPUT);
        pinMode(output_pin1, INPUT);
        pinMode(output_pin2, INPUT);

        setState(Init);
    }

    virtual ~ShiftregisterSwScanner25Keys() {}

    void update()
    {
        switch (status_) {
        case Init:
            setState(LoadStart);
            break;
        case LoadStart:
            setState(ReadEachBits);
            break;
        case ReadEachBits: {
            for (auto i = 0u; i < num_switches_; ++i) {
                digitalWrite(pins_.clock_pin, LOW);

                const auto read_data1                   = digitalRead(pins_.output_pin1);
                scan_buffers_[0][num_switches_ - i - 1] = read_data1;

                const auto read_data2                   = digitalRead(pins_.output_pin2);
                scan_buffers_[1][num_switches_ - i - 1] = read_data2;

                digitalWrite(pins_.clock_pin, HIGH);
            }
            updateSwitchStatus();
            setState(WaitNext);
            break;
        case WaitNext: {
            const auto current = millis();
            const auto delta   = current - wait_start_;
            if (delta > scan_period_) {
                setState(LoadStart);
            }
        } break;
        default:
            break;
        }
        }
    };

protected:
    uint8_t status_          = UnknownState;
    size_t will_read_switch_ = 0;

    void setState(const int status)
    {
        status_ = status;
        assert(pins_.npl_pin != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.clock_pin != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.output_pin1 != Pins::INVALID_PIN_CONFIGURATION);
        assert(pins_.output_pin2 != Pins::INVALID_PIN_CONFIGURATION);

        switch (status_) {
        case Init:
            digitalWrite(pins_.npl_pin, HIGH);
            digitalWrite(pins_.clock_pin, LOW);
            break;
        case LoadStart:
            digitalWrite(pins_.npl_pin, LOW);
            digitalWrite(pins_.clock_pin, LOW);
            will_read_switch_ = 0;
            break;
        case ReadEachBits:
            digitalWrite(pins_.clock_pin, LOW);
            digitalWrite(pins_.npl_pin, HIGH);
            break;
        case WaitNext:
            wait_start_ = millis();
            digitalWrite(pins_.npl_pin, HIGH);
            digitalWrite(pins_.clock_pin, LOW);
            break;
        default:
            break;
        }
    }

    void updateSwitchStatus()
    {
        for (auto j = 0u; j < 2; ++j) {
            for (auto i = 0u; i < num_switches_; ++i) {
                if (scan_buffers_[j][i] == former_scan_buffers_[j][i]) {
                    const auto new_status = scan_buffers_[j][i];
                    auto current_status   = switch_status_[j][i];
                    if (current_status != new_status) {
                        switch_status_[j][i] = (new_status);
                        const auto notification_status =
                            !switch_status_[j][i]; // NOTE: inverted!! off = HIGH, on = LOW

                        if (handler_) {
                            if (j == 0) {
                                (*handler_)(i, notification_status);
                            } else {
                                (*handler_)(i + num_switches_, notification_status);
                            }
                        }
                    }
                }
                former_scan_buffers_[j][i] = scan_buffers_[j][i];
            }
        }
    }
    struct Pins
    {
        enum
        {
            INVALID_PIN_CONFIGURATION = 0xff,
        };
        Pins(const uint8_t npl_pin, const uint8_t clock_pin,
             const uint8_t input_pin1, const uint8_t input_pin2)
            : npl_pin(npl_pin), clock_pin(clock_pin), output_pin1(input_pin1),
              output_pin2(input_pin2) {}

        Pins() = default;

        uint8_t npl_pin     = INVALID_PIN_CONFIGURATION;
        uint8_t clock_pin   = INVALID_PIN_CONFIGURATION;
        uint8_t output_pin1 = INVALID_PIN_CONFIGURATION;
        uint8_t output_pin2 = INVALID_PIN_CONFIGURATION;
    };
    Pins pins_;
    SwitchHandler handler_ = nullptr;
    uint32_t num_switches_ = DEFAULT_NUM_SWITCH_EACH;

    uint8_t scan_buffers_[2][16];        // 0: 1st, 1: 2nd
    uint8_t former_scan_buffers_[2][16]; // 0: 1st, 1: 2nd
    uint8_t switch_status_[2][16];       // 0: 1st, 1: 2nd
    uint32_t wait_start_  = 0;
    uint32_t scan_period_ = 0;

private:
    ShiftregisterSwScanner25Keys(const ShiftregisterSwScanner25Keys&) {}
};

} // namespace kinoshita_lab
#endif // SHIFTREGISTERSWSCANNER_H