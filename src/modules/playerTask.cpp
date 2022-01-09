#include "playerTask.h"
#include <VS1053.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

VS1053 HWplayer(VS1053_CS, VS1053_DCS, VS1053_DREQ);
HTTPClient http;

int playerTask::volume = 0;
int playerTask::state = PLAYER_STARTUP;
char playerTask::streamURL[300];
char playerTask::fileURL[300];
uint8_t playerTask::dataBuff[64];
int playerTask::webLen = 0;
long playerTask::lastStatusUpdate = 0;
WiFiClient *playerTask::stream = nullptr;   
PubSubClient *playerTask::MQTTClient = nullptr;

#define mqtt_base_topic "ESP32Mqtt-Streamer"
#define mqtt_playtime_topic mqtt_base_topic "/playtime"
#define mqtt_status_topic mqtt_base_topic "/status"

void playerTask::init()
{
    state = PLAYER_STARTUP;
    memccpy(streamURL, "\0", 1, sizeof(char));
    memccpy(fileURL, "\0", 1, sizeof(char));
    stream = nullptr;
    webLen = 0;
    
    volume = 80;
    SPI.begin();
    HWplayer.begin();
    HWplayer.switchToMp3Mode(); // optional, some boards require this
    HWplayer.setVolume(volume);
    state = PLAYER_IDLE;
    xTaskCreatePinnedToCore(run, "Player Task", 20000, nullptr, 1, nullptr, 1);  
}

void playerTask::setMQTTInstance(PubSubClient* newMQTTClient){
    MQTTClient = newMQTTClient;
}

void playerTask::run(void* arg)
{
    while(true)
    {
        if (http.connected() && (webLen > 0 || webLen == -1) && stream != nullptr && state == PLAYER_PLAY)
            {
                size_t size = stream->available();
                if (size)
                {
                    int c = stream->readBytes(dataBuff, ((size > sizeof(dataBuff)) ? sizeof(dataBuff) : size));
                    HWplayer.playChunk(dataBuff, c);

                    if (webLen > 0)
                    {
                        webLen -= c;
                    }
                }
        }
        if(MQTTClient != nullptr && (millis()-lastStatusUpdate)>=1000){
            lastStatusUpdate = millis();
            int playTime = HWplayer.getDecodedTime();
            char buffer[10];
            itoa(playTime, buffer, 10);
            MQTTClient->publish(mqtt_playtime_topic, buffer);
            itoa(state, buffer, 10);
            MQTTClient->publish(mqtt_status_topic, buffer);

        }

        // vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void playerTask::start()
{
    Serial.print("[Player]Start:");
    Serial.print(streamURL);
    Serial.println("-");
    if(strlen(fileURL))
    {
        http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        http.setTimeout(5000);
        http.begin(fileURL);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            webLen = http.getSize();
            stream = http.getStreamPtr();
            state = PLAYER_PLAY;
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }else if(strlen(streamURL))
    {
        http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        http.setTimeout(5000);
        http.begin(streamURL);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
            webLen = http.getSize();
            stream = http.getStreamPtr();
            state = PLAYER_PLAY;
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
}

void playerTask::stop()
{
    state = PLAYER_STOP;
    Serial.println("[Player]Stop");
}

void playerTask::pause()
{
    state = PLAYER_PAUSE;
    Serial.println("[Player]Pause");
}

void playerTask::playpause()
{
    if(state == PLAYER_PAUSE)
    {
        playerTask::start();
        state = PLAYER_PLAY;
    }
    else if (state == PLAYER_PLAY)
    {
        state = PLAYER_PAUSE;    
    }
    Serial.println("[Player]PlayPause");
}


int playerTask::getVolume()
{
    return volume;
}

void playerTask::setVolume(int newVolume)
{
    Serial.print("[Player]Volume:");
    Serial.println(newVolume);
    volume = newVolume;
    HWplayer.setVolume(newVolume);
}

void playerTask::setStream(const char* newStreamURL)
{
    Serial.println("[Player]Stream:");
    memcpy(streamURL, newStreamURL, strlen(newStreamURL)*sizeof(char));
    streamURL[strlen(newStreamURL)]='\0';
}

void playerTask::setFile(const char* newFileURL)
{
    Serial.println("[Player]File:");
    memcpy(streamURL, newFileURL, strlen(newFileURL)*sizeof(char));    
}