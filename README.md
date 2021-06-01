# Roc-MQTT-Display

Copyright (c) 2020-2021 Christian Heinrichs.
All rights reserved.


Dynamic Passenger Information system for Model Railroad Stations controlled by Rocrail.
Wireless communication with Rocrail or other sources via MQTT messages.
A Lolin/Wemos D1 mini ESP8266 controller and a TCA9548A I2C Multiplexer can drive up to eight 0.91" 128x32
I2C OLED displays. 
Several controllers can run in parallel so the total number of displays is not limited.

Latest Version 1.00 
May 20, 2021 

The goal of this project is to provide simple to use and inexpensive displays to run in a Model Railroad environment.

**Current features**  
- Messages can be static or dynamic
- Formatting is saved in templates so the user can focus on sending the right data to the display and just pick a template for formatting.
- The architecture is extremely scalable:
	* 1-8 displays per ESP8266 based controller
	* Theoretically unlimited number of controllers (didn't have so many for testing :-) )
	* Up to 10 different Fonts
	* Up to 10 Templates
	* Up to 10 Logos
- Scrolltext for static or dynamic content
- Rocrail dynamic variables support
- German Umlaut support
- Clock/Date available for all displays (NTP time or Rocrail time)
- Display rotation
- Screensaver
- Webinterface for configuration
- Updates via OTA

<img src="doc/InAction1.jpg" width="70%" alt="Trainstation"/><img src="doc/RMD002.jpg" width="30%" alt="Displays"/>

Architecture, Hard- and Software Requirements, Installation and Configuration are documented in the [**Wiki**](https://github.com/chrisweather/RocMQTTdisplay/wiki).

Several more ideas and features are on the to do list.
Please share your experience and ideas for improvements via GitHub Issues function.

Thank you for your interest in the Roc-MQTT-Display project.
If you like the project please consider donating if you want to support further development.
Donations are more than welcome and I will use them to buy new displays, controllers and sensors for development and testing and of course for a lot of coffee... :-)

[![PayPal donate button](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?hosted_button_id=XC7T2ECBQYNJ2)
