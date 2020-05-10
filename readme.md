# Adafruit WiFi AIO MQTT OLED  
Contributors: ahortin  
Stable tag: 1.0  

Arduino Sketch for connecting and subscribing to a feed on Adafruit IO


## Description

This Arduino sketch is for subscribing to a Feed on Adafruit IO using MQTT. Adafruit IO is the easiest way to stream, log, and interact with your data using internet-enabled devices.

Tested on an [Adafruit Feather HUZZAH with ESP8266](https://www.adafruit.com/product/2821), with a [FeatherWing 128x32 OLED](https://www.adafruit.com/product/2900), this sketch uses the [WiFiManager](https://github.com/tzapu/WiFiManager) to help you connect to your local WiFi. Using SSL/TLS it then connects to the Adafruit IO MQTT Server to subscribe to a feed. Once a packet is received, it is then displayed on the 128x32 OLED.

The Adafruit Feather HUZZAH with ESP8266 is an an ESP8266 WiFi microcontroller clocked at 80 MHz and at 3.3V logic, with built in USB and battery charging. It can be powered by USB or can be completely mobile by connecting a [3.7v Lithium Ion Polymer Battery](https://www.adafruit.com/product/1578).

This sketch has been written for use with my [Adafruit IO Connector for WooCommerce](https://github.com/maddisondesigns/adafruit-io-connector-for-woocommerce) WordPress plugin which will send product sale information from your WooCommerce site, to an Adafruit IO feed.

### Changelog

= 1.0 =
- Initial commit
