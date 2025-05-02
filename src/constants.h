#pragma once

#include "sys_constants.h"

#define WIFI_MODE                               (WIFI_AP_MODE)

#define WEB_AUTH                                (1)                     // Use basic auth for non-local connections

#define WIFI_CONNECTION_CHECK_INTERVAL          (5000u)                 // Interval (ms) between Wi-Fi connection check
#define WIFI_MAX_CONNECTION_ATTEMPT_INTERVAL    (120000u)               // Max time (ms) to wait for Wi-Fi connection before switch to AP mode
                                                                         // 0 - Newer switch to AP mode

#define MDNS_NAME                               "esp_shade"

#define STEPPER_PIN_1                           (9)
#define STEPPER_PIN_2                           (6)
#define STEPPER_PIN_3                           (7)
#define STEPPER_PIN_4                           (5)
#define STEPPER_PIN_EN                          (8)


#define ENDSTOP_PIN                              (20)
#define ENDSTOP_HIGH_STATE                       (false)

#define TIME_ZONE                               (5.f)                   // GMT +5:00

#define MQTT                                    (0)                     // Enable MQTT server

#define MQTT_CONNECTION_TIMEOUT                 (15000u)                // Connection attempt timeout to MQTT server
#define MQTT_RECONNECT_TIMEOUT                  (5000u)                 // Time before new reconnection attempt to MQTT server

#define MQTT_PREFIX                             ""
#define MQTT_TOPIC_POSITION                     MQTT_PREFIX "/position"
#define MQTT_TOPIC_NIGHT_MODE                   MQTT_PREFIX "/night_mode"

#define MQTT_OUT_PREFIX                         MQTT_PREFIX "/out"
#define MQTT_OUT_TOPIC_POSITION                 MQTT_OUT_PREFIX "/position"
#define MQTT_OUT_TOPIC_NIGHT_MODE               MQTT_OUT_PREFIX "/night_mode"

#include "_override/credentials.h"
