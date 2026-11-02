#pragma once

#include <Arduino.h>
#include <GyverStepper2.h>
#include <driver/timer.h>
#include "sys_constants.h"

// ESP32-C3, Arduino-ESP32 2.x, timer group 0 / timer 0 (80 MHz / 80 = 1 MHz).
// Schedule one interrupt per winding step, not periodic polling.
class TimedStepper : public GStepper2<STEPPER_TYPE> {
    static constexpr timer_group_t GROUP = TIMER_GROUP_0;
    static constexpr timer_idx_t TIMER = TIMER_0;
    static constexpr uint32_t MIN_PERIOD_US = 150;

    hw_timer_t *_timer = nullptr;
    portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
    uint64_t _due = 0;
    bool _active = false;
    inline static TimedStepper *_instance = nullptr;

    void scheduleLocked(bool fromIsr) {
        if (GStepper2<STEPPER_TYPE>::getStatus() == 0) {
            _active = false;
            timer_group_set_counter_enable_in_isr(GROUP, TIMER, TIMER_PAUSE);
            return;
        }
        uint32_t period = GStepper2<STEPPER_TYPE>::getPeriod();
        if (period < MIN_PERIOD_US) period = MIN_PERIOD_US;
        const uint64_t now = timer_group_get_counter_value_in_isr(GROUP, TIMER);
        // Deadlines are anchored to the preceding deadline, not ISR completion.
        // A late ISR must not generate a burst of catch-up winding steps.
        uint64_t next = fromIsr ? _due + period : now + period;
        if (next <= now + MIN_PERIOD_US) next = now + MIN_PERIOD_US;
        _due = next;
        _active = true;
        timer_group_set_alarm_value_in_isr(GROUP, TIMER, _due);
        timer_group_enable_alarm_in_isr(GROUP, TIMER);
        timer_group_set_counter_enable_in_isr(GROUP, TIMER, TIMER_START);
    }

    static void IRAM_ATTR onTimer() {
        TimedStepper *self = _instance;
        if (!self) return;
        portENTER_CRITICAL_ISR(&self->_lock);
        if (self->_active) {
            self->GStepper2<STEPPER_TYPE>::tickManual();
            self->scheduleLocked(true);
        }
        portEXIT_CRITICAL_ISR(&self->_lock);
    }

public:
    using GStepper2<STEPPER_TYPE>::GStepper2;

    bool beginTimer() {
        if (_instance) return false;
        _timer = timerBegin(0, 80, true);
        if (!_timer) return false;
        _instance = this;
        timerStop(_timer);
        // Do NOT allocate as ESP_INTR_FLAG_IRAM: GyverStepper2 and digitalWrite
        // are not guaranteed to be IRAM-safe while flash cache is disabled.
        timerAttachInterrupt(_timer, &TimedStepper::onTimer, false);
        return true;
    }

    ~TimedStepper() {
        if (_timer) {
            portENTER_CRITICAL(&_lock);
            _active = false;
            timer_group_set_counter_enable_in_isr(GROUP, TIMER, TIMER_PAUSE);
            portEXIT_CRITICAL(&_lock);
            timerDetachInterrupt(_timer);
            timerEnd(_timer);
            _instance = nullptr;
        }
    }

    void setTarget(int32_t target, GS_posType mode = ABSOLUTE) {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::setTarget(target, mode);
        if (_timer) scheduleLocked(false);
        portEXIT_CRITICAL(&_lock);
    }

    void setMaxSpeed(double speed) {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::setMaxSpeed(speed);
        portEXIT_CRITICAL(&_lock);
    }

    void setAcceleration(uint16_t acceleration) {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::setAcceleration(acceleration);
        portEXIT_CRITICAL(&_lock);
    }

    void setCurrent(int32_t position) {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::setCurrent(position);
        portEXIT_CRITICAL(&_lock);
    }

    void reset() { setCurrent(0); }

    int32_t getCurrent() {
        portENTER_CRITICAL(&_lock);
        int32_t value = GStepper2<STEPPER_TYPE>::getCurrent();
        portEXIT_CRITICAL(&_lock);
        return value;
    }

    int32_t getTarget() {
        portENTER_CRITICAL(&_lock);
        int32_t value = GStepper2<STEPPER_TYPE>::getTarget();
        portEXIT_CRITICAL(&_lock);
        return value;
    }

    uint8_t getStatus() {
        portENTER_CRITICAL(&_lock);
        uint8_t value = GStepper2<STEPPER_TYPE>::getStatus();
        portEXIT_CRITICAL(&_lock);
        return value;
    }

    void brake() {
        portENTER_CRITICAL(&_lock);
        _active = false;
        if (_timer) timer_group_set_counter_enable_in_isr(GROUP, TIMER, TIMER_PAUSE);
        GStepper2<STEPPER_TYPE>::brake();
        portEXIT_CRITICAL(&_lock);
    }

    void enable() {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::enable();
        portEXIT_CRITICAL(&_lock);
    }

    void disable() {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::disable();
        portEXIT_CRITICAL(&_lock);
    }

    void reverse(bool reversed) {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::reverse(reversed);
        portEXIT_CRITICAL(&_lock);
    }

    void autoPower(bool enabled) {
        portENTER_CRITICAL(&_lock);
        GStepper2<STEPPER_TYPE>::autoPower(enabled);
        portEXIT_CRITICAL(&_lock);
    }
};
