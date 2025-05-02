#pragma once

#include "sys_constants.h"

#include "lib/bootstrap.h"
#include "lib/misc/button.h"
#include "lib/misc/ntp_time.h"
#include "lib/async/promise.h"

#include "config.h"
#include "metadata.h"
#include "cmd.h"
#include "misc/night_mode.h"

#include <GyverStepper2.h>

class Application {
    std::unique_ptr<Bootstrap<Config, PacketType>> _bootstrap = nullptr;
    std::unique_ptr<ConfigMetadata> _metadata = nullptr;
    std::unique_ptr<NightModeManager> _night_mode_manager = nullptr;
    std::unique_ptr<NtpTime> _ntp_time = nullptr;
    std::unique_ptr<Button> _endstop = nullptr;
    std::unique_ptr<GStepper2<STEPPER4WIRE>> _stepper = nullptr;

    RuntimeInfo _runtime_info{};

    bool _initialized = false;
    volatile bool _endstop_pressed = false;

    unsigned long _state_change_time = 0;
    AppState _state = AppState::UNINITIALIZED;

    std::map<const AbstractParameter *, PacketType> _parameter_to_packet{};

public:
    [[nodiscard]] Config &config() const { return _bootstrap->config(); }
    [[nodiscard]] SysConfig &sys_config() const { return config().sys_config; }

    void begin();
    void event_loop();

    void load();
    void update();

    void open();
    void close();

    void move_to(float value);

    void restart() { _bootstrap->restart(); }

protected:
    void change_state(AppState s);

    void emergency_stop();
    void move_to_step(int32_t pos);

    Future<void> homing_async();
    Future<bool> homing_move_async(bool detect_endstop = true);

    void endstop_triggered();
    void endstop_release();

private:
    void _setup();

    void _notify_changes();

    void _bootstrap_state_changed(void *sender, BootstrapState state, void *arg);

    void _service_loop();

    void _handle_property_change(const AbstractParameter *param);
};
