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

// This ESP_VS1053_Library
#include <VS1053.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "modules/playerTask.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define mqtt_base_topic "ESP32Mqtt-Streamer"
#define mqtt_status_topic mqtt_base_topic "/status"
#define mqtt_play_topic mqtt_base_topic "/play"
#define mqtt_stream_topic mqtt_base_topic "/stream"
#define mqtt_stop_topic mqtt_base_topic "/stop"
#define mqtt_volume_topic mqtt_base_topic "/volume"
#define mqtt_pauseplay_topic mqtt_base_topic "/pauseplay"
#define mqtt_file_topic mqtt_base_topic "/file"
#define mqtt_playtime_topic mqtt_base_topic "/playtime"

// WiFi settings example, substitute your own
const char *ssid = "SSID";
const char *password = "PASSWD";

const char *mqtt_server = "192.168.222.98";
const char *mqtt_user = "openhabian";
const char *mqtt_password = "openhabian";

const char *path = "http://streams.radiopsr.de/psr-live/mp3-64/mediaplayer";
// const char *path = "http://mdr-284330-0.cast.mdr.de/mdr/284330/0/mp3/high/stream.mp3";
// const char *path = "http://mdr-284330-0.cast.mdr.de/mdr/284330/0/mp3/low/stream.mp3";
// const char *path = "https://dispatcher.rndfnk.com/br/br3/live/mp3/low";
// const char *path = "https://streams.bcs-systems.de/hrrtl/live/chemnitz/mp3/web";
// const char *path = "http://streams.radiopsr.de/psr-live/aac-64/mediaplayer";

// The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.

WiFiClient WIFIClient;
PubSubClient MQTTClient(WIFIClient);

long lastReconnectAttempt = 0;

boolean reconnect()
{
    if (MQTTClient.connect(mqtt_base_topic, mqtt_status_topic, 1, true, "offline"))
    {
        // Once connected, publish an announcement...
        MQTTClient.publish(mqtt_status_topic, "online", true);
        // ... and resubscribe
        MQTTClient.subscribe(mqtt_play_topic);
        MQTTClient.subscribe(mqtt_play_topic);
        MQTTClient.subscribe(mqtt_stop_topic);
        MQTTClient.subscribe(mqtt_pauseplay_topic);
        MQTTClient.subscribe(mqtt_stream_topic);
        MQTTClient.subscribe(mqtt_file_topic);
        MQTTClient.subscribe(mqtt_volume_topic);
    }
    return MQTTClient.connected();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    char buffer[300];
    memcpy(buffer, payload, length);
    buffer[length] = '\0';
    
    if (!strcmp(topic, mqtt_stream_topic))
    {
        playerTask::setStream(buffer);
        playerTask::stop();
        playerTask::start();
    }
    else if (!strcmp(topic, mqtt_file_topic))
    {
        playerTask::setFile(buffer);
        playerTask::stop();
        playerTask::start();
    }
    else if (!strcmp(topic, mqtt_pauseplay_topic))
    {
        playerTask::playpause();
    }
    else if (!strcmp(topic, mqtt_play_topic))
    {
        playerTask::start();
    }
    else if (!strcmp(topic, mqtt_stop_topic))
    {
        playerTask::stop();
    }
    else if (!strcmp(topic, mqtt_volume_topic))
    {
        playerTask::setVolume(atoi(buffer));
    }
    else{
        Serial.print("[Topic]");
        Serial.print(topic);
        Serial.print("[Payload]");
        Serial.write(payload, length);
        Serial.println();
    }
}

void setup()
{
    //disable brownout detection
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.print("Connecting to SSID ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    MQTTClient.setServer(mqtt_server, 1883);
    MQTTClient.setCallback(callback);

    playerTask::init();
}

void loop()
{
    if (!MQTTClient.connected())
    {
        long now = millis();
        if (now - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnect())
            {
                lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        // Client connected
        MQTTClient.loop();
    }
}
