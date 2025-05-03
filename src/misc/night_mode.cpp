#include "night_mode.h"

void NightModeManager::update() {
    const auto &night_mode = _config.night_mode;

    if (night_mode.enabled && !_ntp_time.available()) {
        D_PRINT("Night mode: time not available");
        _set_state(NightModeState::NO_TIME);

        _night_mode_timer = _timer.add_timeout(
                [=, this](auto) {
                    _night_mode_timer_handler();
                }, 1000);

        return;
    }

    if (_night_mode_timer != -1ul) {
        D_PRINT("Night mode: Clear existing schedule");

        _timer.clear_timeout(_night_mode_timer);
        _night_mode_timer = -1ul;
    }

    if (!night_mode.enabled) {
        _set_state(NightModeState::KILLED);
        return;
    }

    const auto start_offset = min(NtpTime::SECONDS_PER_DAY, night_mode.start_time);
    const auto end_offset = min(NtpTime::SECONDS_PER_DAY, night_mode.end_time);

    auto now = _ntp_time.epoch_tz();
    auto start_day = _ntp_time.today_tz();

    auto next_end_time = start_day + end_offset;
    if (now > next_end_time) {
        start_day += NtpTime::SECONDS_PER_DAY;
        next_end_time += NtpTime::SECONDS_PER_DAY;
    }

    _next_start_night_time = start_day + start_offset;
    _next_end_night_time = next_end_time;

    if (start_offset > end_offset) _next_start_night_time -= NtpTime::SECONDS_PER_DAY;

    D_PRINTF("Night mode: time %s - %s\r\n", D_TIME_STRING(_next_start_night_time), D_TIME_STRING(_next_end_night_time));

    bool active = now >= _next_start_night_time && now <= _next_end_night_time;
    _set_state(active ? NightModeState::ACTIVE : NightModeState::WAITING);

    unsigned long interval = (active ? _next_end_night_time : _next_start_night_time) - now + 1;

    D_PRINTF("Night mode: schedule update after %lu sec\r\n", interval);

    _night_mode_timer = _timer.add_timeout(
            [=, this](auto) {
                _night_mode_timer_handler();
            }, interval * 1000);
}

void NightModeManager::_set_state(NightModeState new_state) {
    if (_state == new_state) return;

    D_PRINTF("Night mode: %s\r\n", __debug_enum_str(new_state));

    _state = new_state;
    _e_night_mode_state.publish(this, _state);
}

void NightModeManager::_night_mode_timer_handler() {
    _night_mode_timer = -1ul;
    update();
}
