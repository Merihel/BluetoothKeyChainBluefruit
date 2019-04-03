/**
 * TEST TO RECEIVE BLUETOOTH PACKET
 */

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#define PIN                     5
#define NUMPIXELS               7

#define MANUFACTURER_APPLE         "0x004C"
#define MANUFACTURER_NORDIC        "0x0059"

#define BEACON_MANUFACTURER_ID     MANUFACTURER_NORDIC
#define BEACON_UUID                "01-12-23-34-45-56-67-78-89-9A-AB-BC-CD-DE-EF-F0"
#define BEACON_MAJOR               "0x0000"
#define BEACON_MINOR               "0x0000"
#define BEACON_RSSI_1M             "-54"

//Init du NeoPixel : variable "pixel"
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN);

#define LOADING_COLOR           pixel.Color(14,79,183)
#define LOADING_ANIM_DELAY      500

//Init de la variable bluetooth contexte
/**
 * param1 : BLUEFRUIT_SPI_CS
 * param2 : BLUEFRUIT_SPI_IRQ
 * param3 : BLUEFRUIT_SPI_RST
 */
Adafruit_BluefruitLE_SPI ble(8, 7, 4);

/** Fonctions util BLE **/
// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

void setup() {
  // put your setup code here, to run once:

  //Requires on setup
  while (!Serial);  // required for Flora & Micro
  delay(500);

  pixel.begin();

  for(uint8_t i=0; i<NUMPIXELS; i++) {
    pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
    Serial.println(F("Turning off neopixel #"));
    Serial.println(i);
  }
  pixel.show();

  Serial.begin(115200);

  /* Bluefruit BLE init */
  Serial.println(F("Ready to init BLE..."));
  if ( !ble.begin(true) )
  {
    Serial.println(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  //Performing factory reset
  Serial.println(F("Performing a factory reset: "));
  if ( ! ble.factoryReset() ){
    Serial.println(F("Couldn't factory reset"));
  }

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Setting beacon configuration details: "));

  // AT+BLEBEACON=0x004C,01-12-23-34-45-56-67-78-89-9A-AB-BC-CD-DE-EF-F0,0x0000,0x0000,-54
  ble.print("AT+BLEBEACON="        );
  ble.print(BEACON_MANUFACTURER_ID ); ble.print(',');
  ble.print(BEACON_UUID            ); ble.print(',');
  ble.print(BEACON_MAJOR           ); ble.print(',');
  ble.print(BEACON_MINOR           ); ble.print(',');
  ble.print(BEACON_RSSI_1M         );
  ble.println(); // print line causes the command to execute

  // check response status
  if (! ble.waitForOK() ) {
    Serial.println(F("Didn't get the OK"));
  }

  Serial.println();
  Serial.println(F("Open your beacon app to test"));


  Serial.println(F("Waiting for device connection"));
  Serial.println();
  
  while(!ble.isConnected()) {
    //animateLoading(pixel, LOADING_ANIM_DELAY);
  }
  
  //Setting bluefruit in data mode
  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));

  Serial.println(F("DEVICE CONNECTED, going in loop"));
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t len = readPacket(&ble, 500);

  ble.print(BEACON_RSSI_1M         );
  
  if (len == 0) return;
  printHex(packetbuffer, len);

  // Color
  Serial.println(F("PacketBuffer: "));
  Serial.println(packetbuffer[0]);
  Serial.println(packetbuffer[1]);
  Serial.println(packetbuffer[2]);
  Serial.println(packetbuffer[3]);
  Serial.println(packetbuffer[4]);
  Serial.println();
  
  if (packetbuffer[1] == 'C') {
    uint8_t red = packetbuffer[2];
    uint8_t green = packetbuffer[3];
    uint8_t blue = packetbuffer[4];
    Serial.print ("RGB #");
    if (red < 0x10) Serial.print("0");
    Serial.print(red, HEX);
    if (green < 0x10) Serial.print("0");
    Serial.print(green, HEX);
    if (blue < 0x10) Serial.print("0");
    Serial.println(blue, HEX);

    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixel.setPixelColor(i, pixel.Color(red,green,blue));
    }
    pixel.show(); // This sends the updated pixel color to the hardware.
  }
}

void animateLoading(Adafruit_NeoPixel pixel, int animDelay) {
  while(!ble.isConnected()) {
    pixel.setPixelColor(0, LOADING_COLOR);
    pixel.setPixelColor(1, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(1, pixel.Color(0,0,0));
    pixel.setPixelColor(2, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(2, pixel.Color(0,0,0));
    pixel.setPixelColor(3, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(3, pixel.Color(0,0,0));
    pixel.setPixelColor(4, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(4, pixel.Color(0,0,0));
    pixel.setPixelColor(5, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(5, pixel.Color(0,0,0));
    pixel.setPixelColor(6, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(6, pixel.Color(0,0,0));
    pixel.setPixelColor(1, LOADING_COLOR);
    pixel.show(); 
  }

  pixel.setPixelColor(0, pixel.Color(0,0,0));
  pixel.setPixelColor(1, pixel.Color(0,0,0));
  pixel.setPixelColor(2, pixel.Color(0,0,0));
  pixel.setPixelColor(3, pixel.Color(0,0,0));
  pixel.setPixelColor(4, pixel.Color(0,0,0));
  pixel.setPixelColor(5, pixel.Color(0,0,0));
  pixel.setPixelColor(6, pixel.Color(0,0,0));
  pixel.show();
  animateDeviceConnected(pixel, 80);
}

void animateDeviceConnected(Adafruit_NeoPixel pixel, int animDelay) {
  for(uint8_t i=0; i<15; i++) {
    pixel.setPixelColor(0, pixel.Color(0,0,0));
    pixel.show();
    delay(animDelay);
    pixel.setPixelColor(0, LOADING_COLOR);
    pixel.show();
    delay(animDelay);
  }
  pixel.setPixelColor(0, pixel.Color(0,0,0));
  pixel.show();
}
