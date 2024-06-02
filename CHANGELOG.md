## Roc-MQTT-Display Changelog

### Roc-MQTT-Display release 1.12
2024-06-02 - tested with Arduino IDE 1.8.19 with latest versions of included libraries 

-   Displays - Tested constructor for 128x32 0.87" OLED I2C Display with SSD1316 controller
-   Successfully tested and documented compatibility with the OLED Adaptors from this project: https://wiki.mobaledlib.de/anleitungen/oled/oled-adapter
-   Displays - Added constructor for 64x32 0.49" OLED I2C Display with SSD1316 controller. Not tested yet.
-   Displays - Added constructor for 72x40 0.42" OLED I2C Display with SSD1316 controller. Not tested yet.
-   Added support for ESP32 boards (do NOT update "esp32" in Arduino IDE Boardmanager to version 3.0, this version has bugs that break WiFi code!)
-   Webinterface - Added Demo Mode, shows different display modes and content, ends automatically after one run through
-   Webinterface - share statistics of hardware for future development focus
-   Webinterface - Several improvements
-   Updates - OTA Network Port will be enabled only when the Update site in the Webinterface has been opened
-   Updates - Preparation for Web Upload of .bin files without Arduino IDE for the next release

### Roc-MQTT-Display release 1.11
You didn't miss anything, I never published this version.

### Roc-MQTT-Display release 1.10
2023-01-29 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   fixed an issue with scrolltext message field positioning - Thanks to Markus smn346
-   added the option to use variable {rrtime} (Rocrail time) also in the departure field - Thanks to Markus smn346 for suggesting it!
-   added station field to configuration, so every physical display can be assigned to a station, for future use
-   added communication with other RM modules (for future use)
-   Template - added ÖBB design templates - Thanks to Markus smn346 for sharing them!
-   Hardware - added 3D printer data for Controller - Thanks to by Peter psuser1 for designing and sharing!

### Roc-MQTT-Display release 1.09
2022-11-14 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   OTA default password set to 'RMDOTA'
-   Webinterface - optimized
-   Webinterface - added Station and Track info for each display in configuration.
-   Fixes - for Issues #31, #32

### Roc-MQTT-Display release 1.08
2022-10-30 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   added MQTT broker connection test to debug mode
-   improved debug messages in serial monitor
-   Added constructor for 128x32 0.87" OLED I2C Display with SSD1316 controller. Not tested yet.
-   Template - added Invert switch to templates. Inverts the whole display
-   Template - increased number of logos from 10 to 20
-   Template - added NS logo (thanks ksw2404)
-   Webinterface - added update check
-   Wiki - Troubleshooting page added
-   3D printer data for 128x32 display case

### Roc-MQTT-Display release 1.07
2022-04-24 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   fixed issues with JSON handling
-   fixed some compiler warnings
-   added option to run only one display without multiplexer
-   Webinterface - optimized config site and help texts
-   Wiki - Wiring updated
-   Wiki - Wiring for setup with one display added
-   Wiki - Schnellstart Anleitung deutsch improved

### Roc-MQTT-Display release 1.06
2022-04-22 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   code optimized
-   Template - ÖBB logo optimized, can be called with ÖBB now instead of OBB
-   Changed wiring, VCC of multiplexer and displays now connected to 5V instead of 3.3V, see wiring in Wiki. Some displays work more reliable with 5V.

### Roc-MQTT-Display release 1.05
2022-04-04 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   changed serial speed from 38400 to 115200 baud
-   scrolltext optimized
-   Template - template for 96x16 display with one line only scrolltext added
-   Template - added SBB and ÖBB logo
-   Webinterface - contrast setting can be changed now on config site
-   Webinterface - display width and height can be set on config site, should match with selected display constructor
-   Webinterface - added more help texts
-   Webinterface - import template as JSON string added to configuration
-   Webinterface - stability improved
-   Wiki - quickstart description in German language added

### Roc-MQTT-Display release 1.04
2022-02-05 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   added alternative separator option in configuration. MQTT payload can be separated by # or by other characters, for example ";" "," " . ", up to 3 characters incl. space
-   screenshot option for display 1, write display buffer to serial out as XBM image

### Roc-MQTT-Display release 1.03
2022-01-10 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   fixes in default configuration for MQTT server, NTP server
-   added two more variables {rrdate}, {ntpdate}

### Roc-MQTT-Display release 1.02
2022-01-09 - tested with Arduino IDE 1.8.19 with latest versions of included libraries

-   code cleanup

### Roc-MQTT-Display release 1.01
2021-10-16 - tested with Arduino IDE 1.8.15 with latest versions of included libraries

-   Added display constructor for 96x16 displays to source code
-   Show display resolution in config HTML site

### Roc-MQTT-Display release 1.00
2021-05-20 - tested with Arduino IDE 1.8.13 with latest versions of included libraries

-   Added more error handling and solved some compiler warnings
-   Improved configuration HTML sites
-   Added download function in webinterface to save the configuration/templates as files
-   Added 3 more fonts to default configuration
-   Adjusted some settings in default templates
-   MQTT topics added to configuration
-   Stability fixes
-   GitHub Wiki, added more examples and documentation  

### Roc-MQTT-Display release 0.99
2021-04-12 - tested with Arduino IDE 1.8.13 with latest versions of included libraries

-    Initial release  
