#include "application.h"

void Application::begin() {
    D_PRINT("Starting application...");

    if (!LittleFS.begin()) {
        D_PRINT("Unable to initialize FS");
    }

    _bootstrap = std::make_unique<Bootstrap<Config, PacketType>>(&LittleFS);

    auto &sys_config = _bootstrap->config().sys_config;
    _bootstrap->begin({
        .mdns_name = sys_config.mdns_name,
        .wifi_mode = sys_config.wifi_mode,
        .wifi_ssid = sys_config.wifi_ssid,
        .wifi_password = sys_config.wifi_password,
        .wifi_connection_timeout = sys_config.wifi_max_connection_attempt_interval,
        .mqtt_enabled = sys_config.mqtt,
        .mqtt_host = sys_config.mqtt_host,
        .mqtt_port = sys_config.mqtt_port,
        .mqtt_user = sys_config.mqtt_user,
        .mqtt_password = sys_config.mqtt_password,
    });

    _ntp_time = std::make_unique<NtpTime>();
    _night_mode_manager = std::make_unique<NightModeManager>(*_ntp_time.get(), _bootstrap->timer(), _bootstrap->config());

    _night_mode_manager->event_night_mode().subscribe(this, [this](auto, auto, auto) {
        load();
    });

    _bootstrap->event_state_changed().subscribe(this, [this](auto sender, auto state, auto arg) {
        _bootstrap_state_changed(sender, state, arg);
    });

    _bootstrap->timer().add_interval([this](auto) { _app_loop(); }, APP_LOOP_INTERVAL);

    _bootstrap->event_state_changed().subscribe(this, BootstrapState::READY, [this, &sys_config](auto, auto, auto) {
        _ntp_time->begin(sys_config.time_zone);

        _ntp_time->update();
        _night_mode_manager->update();

        load();

        _bootstrap->timer().add_interval([this](auto) {
            if (_bootstrap->wifi_manager()->mode() == WifiMode::STA) {
                _ntp_time->update();
            }
        }, BOOTSTRAP_SERVICE_LOOP_INTERVAL);
    });

    auto &stepper_cfg = config().stepper_config;
    _stepper = std::make_unique<GStepper2<STEPPER4WIRE>>(
        (uint16_t) stepper_cfg.resolution,
        sys_config.stepper_pin_1,
        sys_config.stepper_pin_2,
        sys_config.stepper_pin_3,
        sys_config.stepper_pin_4,
        sys_config.stepper_pin_en
    );

    _stepper->setAcceleration(stepper_cfg.acceleration);
    _stepper->reverse(stepper_cfg.reverse);

    _endstop = std::make_unique<Button>(sys_config.endstop_pin, sys_config.endstop_high_state);
    _endstop->set_hold_repeat(false);
    _endstop->set_on_hold([this](auto) { endstop_triggered(); });
    _endstop->set_on_hold_release([this](auto) { endstop_release(); });

    _bootstrap->timer().add_interval([this](auto) { _service_loop(); }, APP_SERVICE_LOOP_INTERVAL);

    _setup();
    change_state(AppState::INITIALIZATION);
}

void Application::_setup() {
    NotificationBus::get().subscribe([this](auto sender, auto param) {
        if (sender != this) _handle_property_change(param);
    });

    auto &ws_server = _bootstrap->ws_server();
    auto &mqtt_server = _bootstrap->mqtt_server();

    _metadata = std::make_unique<ConfigMetadata>(build_metadata(config(), _runtime_info));
    _metadata->visit([this, &ws_server, &mqtt_server](AbstractPropertyMeta *meta) {
        auto binary_protocol = (BinaryProtocolMeta<PacketType> *) meta->get_binary_protocol();
        if (binary_protocol->packet_type.has_value()) {
            ws_server->register_parameter(*binary_protocol->packet_type, meta->get_parameter());
            VERBOSE(D_PRINTF("WebSocket: Register property %s\r\n", __debug_enum_str(*binary_protocol->packet_type)));
        }

        auto mqtt_protocol = meta->get_mqtt_protocol();
        if (mqtt_protocol->topic_in && mqtt_protocol->topic_out) {
            mqtt_server->register_parameter(mqtt_protocol->topic_in, mqtt_protocol->topic_out, meta->get_parameter());
            VERBOSE(D_PRINTF("MQTT: Register property %s <-> %s\r\n", mqtt_protocol->topic_in, mqtt_protocol->topic_out));
        } else if (mqtt_protocol->topic_out) {
            mqtt_server->register_notification(mqtt_protocol->topic_out, meta->get_parameter());
            VERBOSE(D_PRINTF("MQTT: Register notification -> %s\r\n", mqtt_protocol->topic_out));
        }

        if (binary_protocol->packet_type.has_value()) {
            _parameter_to_packet[meta->get_parameter()] = binary_protocol->packet_type.value();
        }
    });

    ws_server->register_notification(PacketType::HOMED, _metadata->data.homed);

    ws_server->register_data_request(PacketType::GET_CONFIG, _metadata->data.config);

    ws_server->register_command(PacketType::RESTART, [this] { _bootstrap->restart(); });
    ws_server->register_command(PacketType::HOMING, [this] { homing_async(); });
}

void Application::_notify_changes() {
    _bootstrap->ws_server()->send_notification(PacketType::POSITION);
    _bootstrap->ws_server()->send_notification(PacketType::HOMED);
    _bootstrap->mqtt_server()->send_notification(MQTT_OUT_TOPIC_POSITION);
}

void Application::event_loop() {
    _bootstrap->event_loop();
}

void Application::_handle_property_change(const AbstractParameter *parameter) {
    auto it = _parameter_to_packet.find(parameter);
    if (it == _parameter_to_packet.end()) return;

    auto type = it->second;
    if (type >= PacketType::NIGHT_MODE_ENABLED && type <= PacketType::NIGHT_MODE_END) {
        _night_mode_manager.reset();
        update();
    } else {
        update();
    }
}

void Application::load() {
    // TODO
}

void Application::update() {
    _bootstrap->save_changes();
    load();
}

void Application::change_state(AppState s) {
    _state_change_time = millis();
    _state = s;
    D_PRINTF("Change app state: %s\r\n", __debug_enum_str(s));
}

void Application::emergency_stop() {
    _stepper->brake();
    change_state(AppState::STAND_BY);
}

Future<void> Application::homing_async() {
    if (_state != AppState::STAND_BY) {
        D_PRINTF("Homing forbidden for state: %s\r\n", __debug_enum_str(_state));
        return Future<void>::errored();
    }

    change_state(AppState::HOMING);

    _runtime_info.position = 0;
    _runtime_info.homed = false;

    _notify_changes();

    auto &cfg = config().stepper_config;

    return Future<void>::successful()
        .then<bool>([this, &cfg](auto) {
            D_PRINT("Homing: First step");

            _stepper->enable();

            // First homing step
            _stepper->reset();
            _stepper->setMaxSpeed(cfg.homing_speed);
            _stepper->setTarget(-cfg.homing_steps_max);

            return homing_move_async();
        })
        .then<bool>([this, &cfg](auto &f) {
            if (!f.result()) {
                D_PRINT("Homing failed! Movement limit exceeded");
                return Future<bool>::errored();
            }

            D_PRINT("Homing: Rewind");

            // Go up a little
            _stepper->reset();
            _stepper->setTarget(cfg.homing_steps);

            return homing_move_async(false);
        })
        .then<bool>([this, &cfg](auto &f) {
            if (f.result()) {
                D_PRINT("Homing failed! Endstop did not reset");
                return Future<bool>::errored();
            }

            D_PRINT("Homing: Second step");

            // Second homing step
            _stepper->reset();
            _stepper->setMaxSpeed(cfg.homing_speed_second);
            _stepper->setTarget(-cfg.homing_steps * 1.5);

            return homing_move_async();
        })
        .then<void>([this](auto &f) {
            if (!f.result()) {
                D_PRINT("Homing failed! Second movement limit exceeded");
                return Future<void>::errored();
            }

            _runtime_info.homed = true;
            D_PRINT("Homing success!");

            return Future<void>::successful();
        })
        .finally([this] {
            _stepper->brake();
            _stepper->reset();
            _stepper->disable();

            _notify_changes();

            change_state(AppState::STAND_BY);
        });
}

Future<bool> Application::homing_move_async(bool detect_endstop) {
    auto promise = Promise<bool>::create();
    auto timer_id = _bootstrap->timer().add_interval([=](auto) {
        if (promise->finished()) return;

        if ((_endstop_pressed && detect_endstop) || !_stepper->tick()) {
            promise->set_success(_endstop_pressed);
        }
    }, APP_SERVICE_LOOP_INTERVAL);

    return Future{promise}.finally([this, timer_id](auto) {
        _bootstrap->timer().clear_interval(timer_id);
    });
}

void Application::endstop_triggered() {
    if (_endstop_pressed) return;

    _endstop_pressed = true;
    D_PRINT("Endstop triggered!");

    if (_state == AppState::MOVING) {
        D_PRINT("Endstop triggered during movement!");
        emergency_stop();
    }
}

void Application::endstop_release() {
    if (!_endstop_pressed) return;

    _endstop_pressed = false;
    D_PRINT("Endstop released!");
}

void Application::_app_loop() {
#if defined(DEBUG) && DEBUG_LEVEL <= __DEBUG_LEVEL_VERBOSE
    static unsigned long t = 0;
    static unsigned long ii = 0;
    if (ii % 10 == 0) D_PRINTF("App loop latency: %lu\r\n", millis() - t);

    t = millis();
    ++ii;
#endif

    switch (_state) {
        case AppState::UNINITIALIZED:
            break;

        case AppState::INITIALIZATION:
            break;

        case AppState::STAND_BY:
            break;
    }
}

void Application::_service_loop() {
    _endstop->handle();
    _stepper->tick();
}

void Application::_bootstrap_state_changed(void *sender, BootstrapState state, void *arg) {
    if (state == BootstrapState::INITIALIZING) {
        _ntp_time->begin(TIME_ZONE);

        change_state(AppState::INITIALIZATION);
        load();
    } else if (state == BootstrapState::READY && !_initialized) {
        _initialized = true;

        change_state(AppState::STAND_BY);
        load();
    }
}
