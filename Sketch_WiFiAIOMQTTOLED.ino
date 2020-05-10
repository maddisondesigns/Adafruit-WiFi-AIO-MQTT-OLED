/***************************************************
  The below code based on various examples provided by Adafruit
  This has been tested with Adafruit Feather HUZZAH with ESP8266 (https://www.adafruit.com/product/2821)
  and the Adafruit FeatherWing 128x32 OLED (https://www.adafruit.com/product/2900)

  MIT License


  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products from Adafruit!
 ****************************************************/

// Library for communicating with I2C / TWI devices
#include <Wire.h>

// The Wi-Fi library for ESP8266 modules
#include <ESP8266WiFi.h>

// WiFiManager Local DNS Server used for redirecting all requests to the configuration portal
#include <DNSServer.h>

// WiFiManager Local WebServer used to serve the configuration portal
#include <ESP8266WebServer.h>

// WiFiManager Configuration Magic (https://github.com/tzapu/WiFiManager)
#include <WiFiManager.h>

// Arduino library for MQTT support, including access to Adafruit IO
#include "Adafruit_MQTT.h"

// Arduino library for MQTT support, including access to Adafruit IO
#include "Adafruit_MQTT_Client.h"

// Core graphics library for all Adafruit displays
#include <Adafruit_GFX.h>

// Core library for Adafruit Monochrome OLEDs based on SSD1306 drivers
#include <Adafruit_SSD1306.h>

//Define our OLED size
#define PX_WIDTH 128
#define PX_HEIGHT 32

// Initialise OLED Display
Adafruit_SSD1306 display = Adafruit_SSD1306( PX_WIDTH, PX_HEIGHT, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_FEATHER52832)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif

// Define WiFi Access Point details for WiFi Manager
// Adafruit Feather Huzzah supports (2.4GHz b/g/n)
// WiFi Manger will prompt for the SSID/PW for the WiFi you wish to connect to, if it can't AutoConnect
// The AP Name/PW below are for the AP your device creates when prompting for your WiFi Details
#define WIFIMGR_AP "FeatherAP"
#define WIFIMGR_APPW "=- PASSWORD for AP -="

// Define Adafruit IO connection
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 8883 // use 1883 for Insecure connection or 8883 for SSL connection
#define AIO_USERNAME "=- ADAFRUIT IO Username -="
#define AIO_KEY "=- ADAFRUIT IO Key -="

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;
//WiFiClientSecure for SSL/TLS support
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

// Setup Adafruit IO Subscribe Feeds
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe woocommerceSales = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/woocommerce-sales");

// io.adafruit.com SHA1 fingerprint
static const char *fingerprint PROGMEM = "77 00 54 2D DA E7 D8 03 27 31 23 99 EB 27 DB CB A5 4C 57 18";

// Define our variables
int x = 0;
int maxStringLen;
bool messageFinished;
String prefixStr = "";
String productStr = "";

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin( 115200 );
  delay(10);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  Serial.println();
  Serial.println( F("OLED started") );

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay( 1000 );

  display.setTextSize( 1 );
  display.setCursor( 0, 0 );
  display.print( F("Connecting to WiFi...") );
  display.display();

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if( !wifiManager.autoConnect( WIFIMGR_AP, WIFIMGR_APPW ) ) {
    Serial.println( F("failed to connect and hit timeout") );
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay( 1000 );
  } 

  // Connect to our WiFi network
  Serial.println();
  Serial.print( F("Connecting to ") );
  Serial.println( WiFi.SSID() );
  Serial.println();
  Serial.println( F("WiFi connected") );  
  Serial.println( F("IP address: ") );
  Serial.println( WiFi.localIP() );
  
  // check the fingerprint of io.adafruit.com's SSL cert
  client.setFingerprint( fingerprint );

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  //text display tests
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  // Setup MQTT subscription for onoff & slider feed.
  mqtt.subscribe(&woocommerceSales);

  displayStrings( "WiFi connected to:", WiFi.SSID(), 1000, 0 );
  display.clearDisplay();
  display.display();

}

void loop() {
  String messageMQTT;

  // Display the WiFi SSDI if Button A is pressed
  if( !digitalRead( BUTTON_A ) ) {
    displayStrings( "WiFi connected to:", WiFi.SSID(), 1000, 0 );
    display.clearDisplay();
    display.display();
  }
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).
  // See the MQTT_connect function definition further below.
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ( ( subscription = mqtt.readSubscription( 1000 ) ) ) {
    // Check if its the WooCommerce Sales feed
    if ( subscription == &woocommerceSales ) {
      messageMQTT = (char *)woocommerceSales.lastread;

      prefixStr = messageMQTT.substring( 0, messageMQTT.indexOf( "|" ) );
      productStr = messageMQTT.substring( messageMQTT.indexOf( "|" ) + 1 );

      Serial.print( prefixStr );
      Serial.println( productStr );

      x = 0;
      // Display our message and pause
      displayStrings( prefixStr, productStr, 1000, x );
      
      // Scroll our message ascross the display
      maxStringLen = productStr.length() * 10; // No. of characters x 10px character width (TextSize 1=5px, 2=10px etc.)
      while( abs( x ) < ( maxStringLen + PX_WIDTH ) ) {
        messageFinished = true;
        displayStrings( prefixStr, productStr, 0, x );        
        x = x - 2;
      }

      // Display our message one last time then clear the display
      if ( abs( x ) >= ( maxStringLen + PX_WIDTH ) && messageFinished ) {
        delay(1000);
        displayStrings( prefixStr, productStr, 2000, 0 );
        display.clearDisplay();
        display.display();
        messageFinished = false;
      }
    }
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if ( mqtt.connected() ) {
    return;
  }

  Serial.print( "Connecting to MQTT... " );

  uint8_t retries = 3;
  while ( ( ret = mqtt.connect() ) != 0 ) { // connect will return 0 for connected
    Serial.println( mqtt.connectErrorString( ret ) );
    Serial.println( "Retrying MQTT connection in 5 seconds..." );
    mqtt.disconnect();
    delay( 5000 );  // wait 5 seconds
    retries--;
    if ( retries == 0 ) {
      // basically die and wait for WDT to reset me
      while ( 1 );
    }
  }
  Serial.println( "MQTT Connected!" );
}

void displayStrings( String staticString, String scrollingString, int pause, int positionX ) {
  display.clearDisplay();
  
  display.setTextSize( 1 );
  display.setCursor( 0, 0 );
  display.print( staticString );
  
  display.setTextSize( 2 );
  display.setCursor( positionX, 10 );
  display.print( scrollingString );
  
  display.display();
  if ( pause > 0 ) {
    delay( pause );  
  }
}
