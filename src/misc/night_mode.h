#pragma once

#include "lib/debug.h"

#include "lib/misc/event_topic.h"
#include "lib/misc/ntp_time.h"
#include "lib/misc/timer.h"
#include "lib/utils/enum.h"

#include "app/config.h"

MAKE_ENUM(NightModeState, uint8_t,
    KILLED, 0,
    NO_TIME, 1,
    WAITING, 2,
    ACTIVE, 3,
)

class NightModeManager {
    NtpTime &_ntp_time;
    Timer &_timer;
    const Config &_config;

    unsigned long _next_start_night_time = 0;
    unsigned long _next_end_night_time = 0;

    unsigned long _night_mode_timer = -1ul;

    NightModeState _state = NightModeState::KILLED;
    EventTopic<NightModeState> _e_night_mode_state{};

public:
    NightModeManager(NtpTime &ntp_time, Timer &timer, const Config &config) :
        _ntp_time(ntp_time), _timer(timer), _config(config) {}

    auto &event_night_mode() { return _e_night_mode_state; }

    bool active() { return _state == NightModeState::ACTIVE; }

    void update();

private:
    void _set_state(NightModeState new_state);
    void _night_mode_timer_handler();
};
