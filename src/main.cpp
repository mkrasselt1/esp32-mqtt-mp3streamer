#include <Arduino.h>
#include <WiFi.h>
#include <VS1053.h>
#define VS1053_CS     5
#define VS1053_DCS    16
#define VS1053_DREQ   4

// Default volume
#define VOLUME  80

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;

// WiFi settings example, substitute your own
const char *ssid = "WIFISSID";
const char *password = "WIFIPASSWD";

// The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
uint8_t mp3buff[64];

void setup(){
player.begin();
    Serial.begin(115200);
    delay(3000);

    Serial.println("\n\nESP32-MQTT-MP3-Streamer");

    player.begin();
    if (player.isChipConnected()) {
        player.loadDefaultVs1053Patches(); 
    }
    player.switchToMp3Mode();
    player.setVolume(VOLUME);

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

    if (!client.connected()) {
        //Should reconnect
    }

    if (client.available() > 0) {
        // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
        uint8_t bytesread = client.read(mp3buff, 64);
        player.playChunk(mp3buff, bytesread);
    }

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