import {PacketType} from "./cmd.js";

/**@type {PropertiesConfig} */
export const PropertyConfig = [{
    key: "status", section: "Status", props: [
        {
            key: "status.homed", type: "label", kind: 'Boolean', cmd: PacketType.HOMED,
            displayConverter: (value) => {
                return ["Homed", ["NO", "YES"][+value]];
            }
        },
        {
            key: "status.moving", type: "label", kind: 'Boolean', cmd: PacketType.MOVING,
            visibleIf: "status.homed",
            displayConverter: (value) => {
                return ["Moving", ["NO", "YES"][+value]];
            }
        }, {
            key: "status.position", type: "label", kind: "Int32",
            cmd: PacketType.POSITION, visibleIf: "status.homed",
            displayConverter: (value) => [
                "Position",
                `${value} (${(value / window.__app.app.config.stepperCalibration.openPosition * 100).toFixed(0)}%)`
            ]
        },
    ]
}, {
    key: "general", section: "General", props: [
        {key: "power", title: "Power", type: "trigger", kind: "Boolean", cmd: PacketType.POWER},

        {type: "title", label: "Actions", extra: {m_top: true}},
        {key: "do_homing", type: "button", label: "Homing", visibleIf: "status.homed", visibilityInvert: true, cmd: PacketType.HOMING},
        {key: "do_open", type: "button", label: "Open", visibleIf: "status.homed", cmd: PacketType.OPEN},
        {key: "do_close", type: "button", label: "Close", visibleIf: "status.homed", cmd: PacketType.CLOSE},
    ],
}, {
    key: "calibration", section: "Calibration", collapse: true, props: [
        {key: "stepperCalibration.offset", title: "Offset", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CALIBRATION_OFFSET},
        {key: "stepperCalibration.openPosition", title: "Open Position", type: "int", kind: "Int32", cmd: PacketType.STEPPER_CALIBRATION_OPEN_POSITION},
    ]
}, {
    key: "night_mode", section: "Night Mode", collapse: true, props: [
        {key: "nightMode.enabled", title: "Enabled", type: "trigger", kind: "Boolean", cmd: PacketType.NIGHT_MODE_ENABLED},
        {key: "nightMode.startTime", title: "Start Time", type: "time", kind: "Uint32", cmd: PacketType.NIGHT_MODE_START},
        {key: "nightMode.endTime", title: "End Time", type: "time", kind: "Uint32", cmd: PacketType.NIGHT_MODE_END},
    ]
}, {
    key: "stepper", section: "Stepper", collapse: true, props: [
        {key: "stepperConfig.reverse", title: "Reverse Direction", type: "trigger", kind: "Boolean", cmd: PacketType.STEPPER_CONFIG_REVERSE},
        {key: "stepperConfig.resolution", title: "Resolution", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CONFIG_RESOLUTION},

        {type: "title", label: "Speed Settings"},
        {key: "stepperConfig.openSpeed", title: "Open Speed", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CONFIG_OPEN_SPEED},
        {key: "stepperConfig.closeSpeed", title: "Close Speed", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CONFIG_CLOSE_SPEED},
        {key: "stepperConfig.acceleration", title: "Acceleration", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CONFIG_ACCELERATION},

        {type: "title", label: "Homing Settings"},
        {key: "stepperConfig.homingSpeed", title: "Homing Speed", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CONFIG_HOMING_SPEED},
        {key: "stepperConfig.homingSpeedSecond", title: "Secondary Homing Speed", type: "int", kind: "Uint16", cmd: PacketType.STEPPER_CONFIG_HOMING_SPEED_SECOND},
        {key: "stepperConfig.homingSteps", title: "Homing Steps", type: "int", kind: "Int32", cmd: PacketType.STEPPER_CONFIG_HOMING_STEPS},
        {key: "stepperConfig.homingStepsMax", title: "Max Homing Steps", type: "int", kind: "Int32", cmd: PacketType.STEPPER_CONFIG_HOMING_STEPS_MAX},

        {type: "title", label: "Actions", extra: {m_top: true}},
        {key: "apply_stepper_config", type: "button", label: "Apply"},
    ]
}, {
    key: "system", section: "System Settings", collapse: true, props: [
        {key: "sysConfig.mdnsName", title: "mDNS Name", type: "text", kind: "FixedString", maxLength: 32, cmd: PacketType.SYS_CONFIG_MDNS_NAME},

        {type: "title", label: "WiFi"},
        {key: "sysConfig.wifiMode", title: "Mode", type: "select", kind: "Uint8", cmd: PacketType.SYS_CONFIG_WIFI_MODE, list: "wifiMode"},
        {key: "sysConfig.wifiSsid", title: "SSID", type: "text", kind: "FixedString", maxLength: 32, cmd: PacketType.SYS_CONFIG_WIFI_SSID},
        {key: "sysConfig.wifiPassword", title: "Password", type: "password", kind: "FixedString", maxLength: 32, cmd: PacketType.SYS_CONFIG_WIFI_PASSWORD},

        {type: "title", label: "WiFi Advanced"},
        {key: "sysConfig.wifiConnectionCheckInterval", title: "Connection Check Interval", type: "int", kind: "Uint32", cmd: PacketType.SYS_CONFIG_WIFI_CONNECTION_CHECK_INTERVAL},
        {key: "sysConfig.wifiMaxConnectionAttemptInterval", title: "Max Connection Attempt Interval", type: "int", kind: "Uint32", cmd: PacketType.SYS_CONFIG_WIFI_MAX_CONNECTION_ATTEMPT_INTERVAL},

        {type: "title", label: "Stepper Pins"},
        {key: "sysConfig.stepperPin1", title: "Pin 1", type: "int", kind: "Uint8", cmd: PacketType.SYS_CONFIG_STEPPER_1_PIN},
        {key: "sysConfig.stepperPin2", title: "Pin 2", type: "int", kind: "Uint8", cmd: PacketType.SYS_CONFIG_STEPPER_2_PIN},
        {key: "sysConfig.stepperPin3", title: "Pin 3", type: "int", kind: "Uint8", cmd: PacketType.SYS_CONFIG_STEPPER_3_PIN},
        {key: "sysConfig.stepperPin4", title: "Pin 4", type: "int", kind: "Uint8", cmd: PacketType.SYS_CONFIG_STEPPER_4_PIN},
        {key: "sysConfig.stepperPinEn", title: "Enable Pin", type: "int", kind: "Uint8", cmd: PacketType.SYS_CONFIG_STEPPER_EN_PIN},

        {type: "title", label: "Endstop"},
        {key: "sysConfig.endstopPin", title: "Pin", type: "int", kind: "Uint8", cmd: PacketType.SYS_CONFIG_ENDSTOP_PIN},
        {key: "sysConfig.endstopHighState", title: "Active High", type: "trigger", kind: "Boolean", cmd: PacketType.SYS_CONFIG_ENDSTOP_HIGH_STATE},

        {type: "title", label: "Time"},
        {key: "sysConfig.timeZone", title: "Time Zone", type: "float", kind: "Float32", cmd: PacketType.SYS_CONFIG_TIME_ZONE},

        {type: "title", label: "MQTT"},
        {key: "sysConfig.mqtt", title: "Enabled", type: "trigger", kind: "Boolean", cmd: PacketType.SYS_CONFIG_MQTT_ENABLED},
        {key: "sysConfig.mqttHost", title: "Host", type: "text", kind: "FixedString", maxLength: 32, cmd: PacketType.SYS_CONFIG_MQTT_HOST},
        {key: "sysConfig.mqttPort", title: "Port", type: "int", kind: "Uint16", cmd: PacketType.SYS_CONFIG_MQTT_PORT},
        {key: "sysConfig.mqttUser", title: "User", type: "text", kind: "FixedString", maxLength: 32, cmd: PacketType.SYS_CONFIG_MQTT_USER},
        {key: "sysConfig.mqttPassword", title: "Password", type: "password", kind: "FixedString", maxLength: 32, cmd: PacketType.SYS_CONFIG_MQTT_PASSWORD},

        {type: "title", label: "Actions", extra: {m_top: true}},
        {key: "apply_sys_config", type: "button", label: "Apply"},
    ]
}];
