# esp-shades

Smart Roller Shades/Blinders Control for ESP32-C3

Other: [[RGB LED LAMP](https://github.com/DrA1ex/esp_led_lamp)] [[LED](https://github.com/DrA1ex/esp_led)] [[RELAY](https://github.com/DrA1ex/esp_relay)]

## UI

<img width="382" alt="image" src="https://github.com/user-attachments/assets/a2eba0ad-ee8d-491d-8ac3-008b8cd79ef4" />


## Features

- Web/Mobile Application (PWA)
- Integration with any Smart Home Assistant (such as Alise or SmartHome) via MQTT broker
- MQTT Protocol

## Connection

![image](https://github.com/user-attachments/assets/5ffc6556-a68d-44c3-a5da-f1e7bb6f1139)


To set up the ESP32-C3 for controlling smart roller shades or blinds, you will need a 4-phase stepper motor paired with a compatible driver. This motor will drive the mechanical movement of the shades. Additionally, you need incorporate an end-stop mechanism, such as an end-stop button or a Hall sensor, to detect the fully open position of the shades.

### Configuration
Connect the stepper motor, driver, and end-stop components to the ESP32-C3 as defined in the [constants.h](/src/constants.h) file.

Before uploading the firmware, verify the pin assignments in `constants.h` match your hardware setup. Incorrect pin configurations can lead to motor malfunctions or unresponsive end-stops. If you’re using a Hall sensor, ensure it is calibrated to detect the magnetic field at the desired end-stop position.

For detailed pin mappings and configuration options, check [constants.h](/src/constants.h).

## Installation

### Manual Build / OTA

1. Install [Platform.IO](https://platformio.org/install):
2. (Optional) Modify the `credentials.h` file and, if necessary, customize `constants.h`. You can change these parameters later through the Web UI.
3. Upload filesystem and firmware

**Note:** This repository contains a submodule. Please use the `--recursive` option when cloning.

```bash
git clone --recursive https://github.com/DrA1ex/esp_shades.git
cd esp_shades

# Make script executable
chmod +x ./upload_fs.sh

# Specify the platform: esp32-c3
PLATFORM=esp32-c3

# Set the environment: debug, release, or ota
ENV=release

# For OTA: set your ESP's address
ADDRESS=esp_shades.local

# Additional envs if OTA enabled
if [ "$ENV" = "ota" ]; then OTA=1 else OTA=0 ADDRESS= fi

pio run -t upload -e $PLATFORM-$ENV --upload-port "$ADDRESS"
./upload_fs.sh --upload-port "$ADDRESS"
```

## MQTT Protocol

| Topic In *       		         | Topic Out *                   | Type      | Values	 | Comments                        |
|------------------------------|-------------------------------|-----------|---------|-------------------------------------|
| `MQTT_TOPIC_OPEN`	           | `MQTT_OUT_TOPIC_OPEN`         | `uint8_t` | 0..1    | Fully Closed (0) / Opened (1)       |
| `MQTT_TOPIC_POSITION`	       | `MQTT_OUT_TOPIC_POSITION`     | `float32` | 0..100  | Open position (0) - Fully Closed / (100) - Fully Opened |
| `MQTT_TOPIC_SPEED`	       | `MQTT_OUT_TOPIC_SPEED`        | `uint8_t` | 0..2    | Speed Mode: Slow (0) / Medium (1) / Fast (2) |
| `MQTT_TOPIC_NIGHT_MODE`	   | `MQTT_OUT_TOPIC_NIGHT_MODE`   | `uint8_t` | 0..1    | Night mode state: ON (1) / OFF (0)  |

\* Actual topic values declared in `constants.h`

## Misc

### Configuring a Secure WebSocket Proxy with Nginx

If you're hosting a Web UI that uses SSL, you'll need to set up a Secure WebSocket (`wss://...`) server instead of the non-secure `ws://` provided by your ESP. Browsers require secure socket connections for WebSocket functionality, so this configuration is essential.

To achieve this, you can use Nginx as a proxy to create an SSL-enabled WebSocket connection.

#### Step 0: Install Nginx

```sh
apt install nginx
```

#### Step 1: Create the Nginx Configuration

Create a file at `/etc/nginx/conf.d/ws.locations` and add the following content:

```nginx
location /w_esp_shades/ws {
    proxy_pass http://<YOUR-ESP-IP-HERE_1>/ws; # Replace with your actual service address
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection keep-alive;
    proxy_set_header Host $host;
}

# You can create proxy for multiple hosts
location /w_esp_shades_2/ws {
    proxy_pass http://<YOUR-ESP-IP-HERE_2>/ws; # Replace with your actual service address
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection keep-alive;
    proxy_set_header Host $host;
}
```

#### Step 2: Reload the Nginx Configuration

After saving the configuration file, reload Nginx to apply the changes:

```sh
nginx -s reload
```

**Note**
Make sure to replace `<YOUR-ESP-IP-HERE_1>` and `<YOUR-ESP-IP-HERE_2>` with the actual IP addresses of your ESP devices.

#### Step 3: Check result

Open WebUi in browser https://dra1ex.github.io/esp_shades/?host=ADDRESS/w_esp_shades
