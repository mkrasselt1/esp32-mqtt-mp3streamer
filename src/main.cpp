#include <Arduino.h>
#include <WiFi.h>
// #include <ArduinoHA.h>
#include "modules/player.h"

// Default volume
#define VOLUME  80
#define BROKER_ADDR         IPAddress(192,168,222,98)

/**
 * @brief Wiring:
  --------------------------------
  | VS1053  | ESP8266 |  ESP32   |
  --------------------------------
  |   SCK   |   D5    |   IO18   |
  |   MISO  |   D6    |   IO19   |
  |   MOSI  |   D7    |   IO23   |
  |   XRST  |   RST   |   EN     |
  |   CS    |   D1    |   IO5    |
  |   DCS   |   D0    |   IO16   |
  |   DREQ  |   D3    |   IO4    |
  |   5V    |   5V    |   5V     |
  |   GND   |   GND   |   GND    |
  --------------------------------
 */

WiFiClient client;
player player;

// WiFi settings example, substitute your own
const char *ssid = "IOT";
const char *password = "3KRBioErPciLZqD4Scp1";


void setup(){
    Serial.begin(115200);
    delay(3000);
    Serial.println("\n\nESP32-MQTT-MP3-Streamer");

    #define VS1053_CS     5
    #define VS1053_DCS    16
    #define VS1053_DREQ   4
    player.begin();

    // Unique ID must be set!
    byte mac[12];
    WiFi.macAddress(mac);

    Serial.print("Connecting to Wifi ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
}


void loop(){
    // mqtt.loop();

    if (!client.connected()) {
        //Should reconnect
    }

    // if (client.available() > 0) {
    //     // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
    //     uint8_t bytesread = client.read(mp3buff, 64);
    //     player.playChunk(mp3buff, bytesread);
    // }

    /* if mqtt command says so
    if (!client.connect(host, httpPort)) {
        return;
    }
    Serial.print("Requesting stream: ");
    Serial.println(path);

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    */

   /**
    * player.setVolume(VOLUME);
    */
}