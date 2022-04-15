## Roc-MQTT-Display Changelog

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