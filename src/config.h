// Roc-MQTT-Display Configuration
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_SSID ""                      // SSID
#define CONFIG_PW ""                        // Password
#define CONFIG_MQTTIP ""                    // MQTT broker IP adress
#define CONFIG_OPTIONAL_MQTTUSR ""          // Optional: MQTT broker username
#define CONFIG_OPTIONAL_MQTTPW ""           // Optional: MQTT broker password
#define CONFIG_MQTTPORT 1883                // MQTT broker port, default: 1883
#define CONFIG_DEVICENAME "RocMQTTdisplay"  // Wifi device name
#define CONFIG_MUX 0x70                     // TCA9548A I2C Multiplexer address
#define CONFIG_DISP 0x3C                    // Standard I2C OLED display address
#define CONFIG_NUMDISP 4                    // Number of connected I2C OLED displays 1-8

#endif
