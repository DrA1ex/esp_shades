#pragma once

#include <cstdint>
#include <lib/network/wifi.h>
#include <lib/utils/enum.h>

#include "credentials.h"
#include "constants.h"

MAKE_ENUM_AUTO(AppState, uint8_t,
    UNINITIALIZED,
    INITIALIZATION,
    STAND_BY,
    HOMING,
    MOVING
);

typedef char ConfigString[CONFIG_STRING_SIZE];

struct __attribute ((packed)) SysConfig {
    ConfigString mdns_name{MDNS_NAME};

    WifiMode wifi_mode = WIFI_MODE;
    ConfigString wifi_ssid{WIFI_SSID};
    ConfigString wifi_password{WIFI_PASSWORD};

    uint32_t wifi_connection_check_interval = WIFI_CONNECTION_CHECK_INTERVAL;
    uint32_t wifi_max_connection_attempt_interval = WIFI_MAX_CONNECTION_ATTEMPT_INTERVAL;

    uint8_t stepper_pin_1 = STEPPER_PIN_1;
    uint8_t stepper_pin_2 = STEPPER_PIN_2;
    uint8_t stepper_pin_3 = STEPPER_PIN_3;
    uint8_t stepper_pin_4 = STEPPER_PIN_4;
    uint8_t stepper_pin_en = STEPPER_PIN_EN;

    uint8_t endstop_pin = ENDSTOP_PIN;
    bool endstop_high_state = ENDSTOP_HIGH_STATE;

    float time_zone = TIME_ZONE;

    bool mqtt = MQTT;
    ConfigString mqtt_host = MQTT_HOST;
    uint16_t mqtt_port = MQTT_PORT;
    ConfigString mqtt_user = MQTT_USER;
    ConfigString mqtt_password = MQTT_PASSWORD;
};

struct __attribute ((packed)) NightModeConfig {
    bool enabled = false;

    uint32_t start_time = 0;
    uint32_t end_time = (uint32_t) 10 * 60 * 60;
};

struct __attribute ((packed)) StepperCalibrationConfig {
    int16_t offset = 100;
    int32_t open_position = STEPPER_RESOLUTION * 10;
};

struct __attribute ((packed)) StepperConfig {
    bool reverse = false;

    uint16_t resolution = STEPPER_RESOLUTION;

    uint16_t open_speed = 300;
    uint16_t close_speed = 500;
    uint16_t acceleration = 300;

    uint16_t homing_speed = 300;
    uint16_t homing_speed_second = 100;

    int32_t homing_steps = 300;
    int32_t homing_steps_max = STEPPER_RESOLUTION * 10;
};

enum class Speed: uint8_t {
    SLOW   = 0,
    NORMAL = 1,
    FAST   = 2
};

struct __attribute ((packed)) Config {
    Speed speed = Speed::NORMAL;

    StepperCalibrationConfig stepper_calibration{};
    NightModeConfig night_mode{};

    StepperConfig stepper_config{};
    SysConfig sys_config{};
};

struct __attribute ((packed)) RuntimeInfo {
    bool homed = false;
    bool moving = false;
    int32_t position = 0;

    float position_target = 0;
    int16_t offset = 0;

    float speed = 1;
    int32_t speed_steps = 0;
};
