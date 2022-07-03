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
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <SPIFFS.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

// This ESP_VS1053_Library
#include <VS1053.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "modules/playerTask.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define mqtt_base_topic "ESP32Mqtt-Streamer"
#define mqtt_all_topics mqtt_base_topic "/#"
#define mqtt_status_topic mqtt_base_topic "/status"
#define mqtt_play_topic mqtt_base_topic "/play"
#define mqtt_stream_topic mqtt_base_topic "/stream"
#define mqtt_stop_topic mqtt_base_topic "/stop"
#define mqtt_volume_topic mqtt_base_topic "/volume"
#define mqtt_pauseplay_topic mqtt_base_topic "/pauseplay"
#define mqtt_file_topic mqtt_base_topic "/file"
#define mqtt_playtime_topic mqtt_base_topic "/playtime"

// WiFi settings example, substitute your own
char mqtt_server[40];
char mqtt_port[6] = "8080";
char mqtt_user[40];
char mqtt_passwd[40];
char api_token[34] = "YOUR_API_TOKEN";
bool shouldSaveConfig = false;

const char *path = "http://streams.radiopsr.de/psr-live/mp3-64/mediaplayer";
// const char *path = "http://mdr-284330-0.cast.mdr.de/mdr/284330/0/mp3/high/stream.mp3";
// const char *path = "http://mdr-284330-0.cast.mdr.de/mdr/284330/0/mp3/low/stream.mp3";
// const char *path = "https://dispatcher.rndfnk.com/br/br3/live/mp3/low";
// const char *path = "https://streams.bcs-systems.de/hrrtl/live/chemnitz/mp3/web";
// const char *path = "http://streams.radiopsr.de/psr-live/aac-64/mediaplayer";

// The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.

WiFiClient WIFIClient;
PubSubClient MQTTClient(WIFIClient);

// callback notifying us of the need to save config
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

long lastReconnectAttempt = 0;

boolean reconnect()
{
    if (MQTTClient.connect(mqtt_base_topic, mqtt_user, mqtt_passwd, mqtt_status_topic, 1, true, "offline"))
    {
        // Once connected, publish an announcement...
        MQTTClient.publish(mqtt_status_topic, "online", true);
        // ... and resubscribe
        MQTTClient.subscribe(mqtt_all_topics);
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
    else
    {
        Serial.print("[Topic]");
        Serial.print(topic);
        Serial.print("[Payload]");
        Serial.write(payload, length);
        Serial.println();
    }
}

void setup()
{
    // disable brownout detection
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.println();

    // clean FS, for testing
    // SPIFFS.format();

    // read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            // file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);

                DynamicJsonDocument json(1024);
                auto deserializeError = deserializeJson(json, buf.get());
                serializeJson(json, Serial);
                if (!deserializeError)
                {

                    Serial.println("\nparsed json");
                    strcpy(mqtt_server, json["mqtt_server"]);
                    strcpy(mqtt_user, json["mqtt_user"]);
                    strcpy(mqtt_passwd, json["mqtt_passwd"]);
                    strcpy(mqtt_port, json["mqtt_port"]);
                    strcpy(api_token, json["api_token"]);
                }
                else
                {
                    Serial.println("failed to load json config");
                }
                configFile.close();
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    // end read

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length
    WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_user("user", "MQTT User", mqtt_user, 40);
    WiFiManagerParameter custom_mqtt_passwd("passwd", "MQTT Passwort", mqtt_passwd, 40);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt_port, 6);
    WiFiManagerParameter custom_api_token("apikey", "API token", api_token, 32);

    // WiFiManager
    // Local initialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    // set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // add all your parameters here
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_passwd);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_api_token);

    // reset settings - for testing
    // wifiManager.resetSettings();

    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"
    // and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect("ESP32MP3Streamer"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        // reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
    }

    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    // read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_passwd, custom_mqtt_passwd.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(api_token, custom_api_token.getValue());
    Serial.println("The values in the file are: ");
    Serial.println("\tmqtt_server : " + String(mqtt_server));
    Serial.println("\tmqtt_user : " + String(mqtt_user));
    Serial.println("\tmqtt_passwd : " + String(mqtt_passwd));
    Serial.println("\tmqtt_port : " + String(mqtt_port));
    Serial.println("\tapi_token : " + String(api_token));

    // save the custom parameters to FS
    if (shouldSaveConfig)
    {
        Serial.println("saving config");
        DynamicJsonDocument json(1024);
        json["mqtt_server"] = mqtt_server;
        json["mqtt_user"] = mqtt_user;
        json["mqtt_passwd"] = mqtt_passwd;
        json["mqtt_port"] = mqtt_port;
        json["api_token"] = api_token;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            Serial.println("failed to open config file for writing");
        }

        serializeJson(json, Serial);
        serializeJson(json, configFile);
        configFile.close();
        // end save
    }

    Serial.println("local ip");
    Serial.println(WiFi.localIP());

    MQTTClient.setCallback(callback);
    MQTTClient.setServer(mqtt_server, 1883);

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
