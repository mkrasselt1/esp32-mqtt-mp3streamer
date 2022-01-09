#ifndef PLAYER_TASK_H
#define PLAYER_TASK_H
#include <VS1053.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 35

enum playerStates{
    PLAYER_STARTUP,
    PLAYER_IDLE,
    PLAYER_PLAY,
    PLAYER_PAUSE,
    PLAYER_STOP
};

class playerTask{
    public:
        static void init();
        static int getVolume();
        static void setVolume(int volume);
        static void setStream(const char* streamURL);
        static void setFile(const char* fileURL);
        static void setMQTTInstance(PubSubClient* newMQTTClient);
        static void start();
        static void pause();
        static void playpause();
        static void stop();
        // getBalance
        // getVolume
        // setTone
        // setBalance
    private:
        static void run(void* arg);
        static int volume;
        static int state;
        static char streamURL[300];
        static char fileURL[300];
        static uint8_t dataBuff[64];
        static int webLen;
        static long lastStatusUpdate;
        static WiFiClient *stream;  
        static PubSubClient *MQTTClient;      
};
#endif