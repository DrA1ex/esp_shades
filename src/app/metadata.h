#pragma once

#include <lib/base/metadata.h>
#include <lib/utils/metadata.h>

#include "app/config.h"
#include "cmd.h"

DECLARE_META_TYPE(AppMetaProperty, PacketType)

DECLARE_META(StepperCalibrationConfigMeta, AppMetaProperty,
    MEMBER(Parameter<uint16_t>, offset),
    MEMBER(Parameter<int32_t>, open_position),
)

DECLARE_META(StepperConfigMeta, AppMetaProperty,
    MEMBER(Parameter<bool>, reverse),
    MEMBER(Parameter<uint16_t>, resolution),
    MEMBER(Parameter<uint16_t>, open_speed),
    MEMBER(Parameter<uint16_t>, close_speed),
    MEMBER(Parameter<uint16_t>, acceleration),
    MEMBER(Parameter<uint16_t>, homing_speed),
    MEMBER(Parameter<uint16_t>, homing_speed_second),
    MEMBER(Parameter<int32_t>, homing_steps),
    MEMBER(Parameter<int32_t>, homing_steps_max),
)

DECLARE_META(NightModeConfigMeta, AppMetaProperty,
    MEMBER(Parameter<bool>, enabled),
    MEMBER(Parameter<uint32_t>, start_time),
    MEMBER(Parameter<uint32_t>, end_time),
)

DECLARE_META(SysConfigMeta, AppMetaProperty,
    MEMBER(FixedString, mdns_name),
    MEMBER(Parameter<uint8_t>, wifi_mode),
    MEMBER(FixedString, wifi_ssid),
    MEMBER(FixedString, wifi_password),
    MEMBER(Parameter<uint32_t>, wifi_connection_check_interval),
    MEMBER(Parameter<uint32_t>, wifi_max_connection_attempt_interval),
    MEMBER(Parameter<uint8_t>, stepper_pin_1),
    MEMBER(Parameter<uint8_t>, stepper_pin_2),
    MEMBER(Parameter<uint8_t>, stepper_pin_3),
    MEMBER(Parameter<uint8_t>, stepper_pin_4),
    MEMBER(Parameter<uint8_t>, stepper_pin_en),
    MEMBER(Parameter<uint8_t>, endstop_pin),
    MEMBER(Parameter<bool>, endstop_high_state),
    MEMBER(Parameter<float>, time_zone),
    MEMBER(Parameter<bool>, mqtt),
    MEMBER(FixedString, mqtt_host),
    MEMBER(Parameter<uint16_t>, mqtt_port),
    MEMBER(FixedString, mqtt_user),
    MEMBER(FixedString, mqtt_password),
)

DECLARE_META(DataConfigMeta, AppMetaProperty,
    MEMBER(ComplexParameter<Config>, config),
    MEMBER(ComplexParameter<RuntimeInfo>, state),

    MEMBER(Parameter<bool>, homed),
    MEMBER(Parameter<bool>, moving),
    MEMBER(Parameter<int32_t>, position),
    MEMBER(Parameter<float>, position_target),
)

DECLARE_META(ConfigMetadata, AppMetaProperty,
    SUB_TYPE(StepperCalibrationConfigMeta, stepper_calibration),
    SUB_TYPE(StepperConfigMeta, stepper_config),
    SUB_TYPE(NightModeConfigMeta, night_mode),
    SUB_TYPE(SysConfigMeta, sys_config),

    SUB_TYPE(DataConfigMeta, data),
)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

inline ConfigMetadata build_metadata(Config &config, RuntimeInfo &runtime_info) {
    return {
        .stepper_calibration = {
            .offset = {
                PacketType::STEPPER_CALIBRATION_OFFSET,
                &config.stepper_calibration.offset
            },
            .open_position = {
                PacketType::STEPPER_CALIBRATION_OPEN_POSITION,
                &config.stepper_calibration.open_position
            }
        },
        .stepper_config = {
            .reverse = {
                PacketType::STEPPER_CONFIG_REVERSE,
                &config.stepper_config.reverse
            },
            .resolution = {
                PacketType::STEPPER_CONFIG_RESOLUTION,
                &config.stepper_config.resolution
            },
            .open_speed = {
                PacketType::STEPPER_CONFIG_OPEN_SPEED,
                &config.stepper_config.open_speed
            },
            .close_speed = {
                PacketType::STEPPER_CONFIG_CLOSE_SPEED,
                &config.stepper_config.close_speed
            },
            .acceleration = {
                PacketType::STEPPER_CONFIG_ACCELERATION,
                &config.stepper_config.acceleration
            },
            .homing_speed = {
                PacketType::STEPPER_CONFIG_HOMING_SPEED,
                &config.stepper_config.homing_speed
            },
            .homing_speed_second = {
                PacketType::STEPPER_CONFIG_HOMING_SPEED_SECOND,
                &config.stepper_config.homing_speed_second
            },
            .homing_steps = {
                PacketType::STEPPER_CONFIG_HOMING_STEPS,
                &config.stepper_config.homing_steps
            },
            .homing_steps_max = {
                PacketType::STEPPER_CONFIG_HOMING_STEPS_MAX,
                &config.stepper_config.homing_steps_max
            }
        },
        .night_mode = {
            .enabled = {
                PacketType::NIGHT_MODE_ENABLED,
                MQTT_TOPIC_NIGHT_MODE, MQTT_OUT_TOPIC_NIGHT_MODE,
                &config.night_mode.enabled
            },
            .start_time = {
                PacketType::NIGHT_MODE_START,
                &config.night_mode.start_time
            },
            .end_time = {
                PacketType::NIGHT_MODE_END,
                &config.night_mode.end_time
            }
        },
        .sys_config = {
            .mdns_name = {
                PacketType::SYS_CONFIG_MDNS_NAME,
                {config.sys_config.mdns_name, CONFIG_STRING_SIZE}
            },
            .wifi_mode = {
                PacketType::SYS_CONFIG_WIFI_MODE,
                (uint8_t *) &config.sys_config.wifi_mode
            },
            .wifi_ssid = {
                PacketType::SYS_CONFIG_WIFI_SSID,
                {config.sys_config.wifi_ssid, CONFIG_STRING_SIZE}
            },
            .wifi_password = {
                PacketType::SYS_CONFIG_WIFI_PASSWORD,
                {config.sys_config.wifi_password, CONFIG_STRING_SIZE}
            },
            .wifi_connection_check_interval = {
                PacketType::SYS_CONFIG_WIFI_CONNECTION_CHECK_INTERVAL,
                &config.sys_config.wifi_connection_check_interval
            },
            .wifi_max_connection_attempt_interval = {
                PacketType::SYS_CONFIG_WIFI_MAX_CONNECTION_ATTEMPT_INTERVAL,
                &config.sys_config.wifi_max_connection_attempt_interval
            },
            .stepper_pin_1 = {
                PacketType::SYS_CONFIG_STEPPER_1_PIN,
                &config.sys_config.stepper_pin_1
            },
            .stepper_pin_2 = {
                PacketType::SYS_CONFIG_STEPPER_2_PIN,
                &config.sys_config.stepper_pin_2
            },
            .stepper_pin_3 = {
                PacketType::SYS_CONFIG_STEPPER_3_PIN,
                &config.sys_config.stepper_pin_3
            },
            .stepper_pin_4 = {
                PacketType::SYS_CONFIG_STEPPER_4_PIN,
                &config.sys_config.stepper_pin_4
            },
            .stepper_pin_en = {
                PacketType::SYS_CONFIG_STEPPER_EN_PIN,
                &config.sys_config.stepper_pin_en
            },
            .endstop_pin = {
                PacketType::SYS_CONFIG_ENDSTOP_PIN,
                &config.sys_config.endstop_pin
            },
            .endstop_high_state = {
                PacketType::SYS_CONFIG_ENDSTOP_HIGH_STATE,
                &config.sys_config.endstop_high_state
            },
            .time_zone = {
                PacketType::SYS_CONFIG_TIME_ZONE,
                &config.sys_config.time_zone
            },
            .mqtt = {
                PacketType::SYS_CONFIG_MQTT_ENABLED,
                &config.sys_config.mqtt
            },
            .mqtt_host = {
                PacketType::SYS_CONFIG_MQTT_HOST,
                {config.sys_config.mqtt_host, CONFIG_STRING_SIZE}
            },
            .mqtt_port = {
                PacketType::SYS_CONFIG_MQTT_PORT,
                &config.sys_config.mqtt_port
            },
            .mqtt_user = {
                PacketType::SYS_CONFIG_MQTT_USER,
                {config.sys_config.mqtt_user, CONFIG_STRING_SIZE}
            },
            .mqtt_password = {
                PacketType::SYS_CONFIG_MQTT_PASSWORD,
                {config.sys_config.mqtt_password, CONFIG_STRING_SIZE}
            },
        },

        .data{
            .config = ComplexParameter(&config),
            .state = ComplexParameter(&runtime_info),

            .homed = Parameter(&runtime_info.homed),
            .moving = Parameter(&runtime_info.moving),
            .position = Parameter(&runtime_info.position),
            .position_target = {
                PacketType::POSITION_TARGET,
                MQTT_TOPIC_POSITION, MQTT_OUT_TOPIC_POSITION,
                &runtime_info.position_target
            },
        },
    };
}

#pragma GCC diagnostic pop
