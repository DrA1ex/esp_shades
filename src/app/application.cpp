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

    _stepper = std::make_unique<GStepper2<STEPPER4WIRE>>(
        4096,
        sys_config.stepper_pin_1,
        sys_config.stepper_pin_2,
        sys_config.stepper_pin_3,
        sys_config.stepper_pin_4,
        sys_config.stepper_pin_en
    );

    _endstop = std::make_unique<Button>(sys_config.endstop_pin, sys_config.endstop_high_state);
    _endstop->set_on_hold([this](auto) {
        _stepper->enable();
        _stepper->reset();
        _stepper->setTarget(1000);
    });

    _endstop->set_on_hold_release([this](auto) {
        _stepper->brake();
        _stepper->disable();
    });

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

    _metadata = std::make_unique<ConfigMetadata>(build_metadata(config()));
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

    ws_server->register_data_request(PacketType::GET_CONFIG, _metadata->data.config);
    ws_server->register_command(PacketType::RESTART, [this] { _bootstrap->restart(); });
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

void Application::set_power(bool on, bool skip_animation) {
    config().power = on;

    D_PRINTF("Turning Power: %s\r\n", on ? "ON" : "OFF");
    if (!skip_animation && _state != AppState::INITIALIZATION) {
        change_state(on ? AppState::TURNING_ON : AppState::TURNING_OFF);
    } else {
        change_state(AppState::STAND_BY);
        load();
    }

    _bootstrap->save_changes();
    NotificationBus::get().notify_parameter_changed(this, _metadata->power.get_parameter());
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
