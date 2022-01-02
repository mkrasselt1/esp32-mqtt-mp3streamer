#include "player.h"
VS1053 HWPlayer();                          // Instance of the player library

player::player(uint8_t _cs_pin, uint8_t _dcs_pin, uint8_t _dreq_pin){
    HWPlayer = VS1053(_cs_pin, _dcs_pin, _dreq_pin);
};

void player::begin(){
    HWPlayer.begin();
    if (HWPlayer.isChipConnected()) {
        if (HWPlayer.getChipVersion() == 4) { // Only perform an update if we really are using a VS1053, not. eg. VS1003
            HWPlayer.loadDefaultVs1053Patches(); 
        }
        HWPlayer.switchToMp3Mode();
        HWPlayer.streamModeOn();
    }
    
}

void player::playChunk(int sizeToPlay, uint8_t* playChunk){
    //player.playChunk(sampleMp3, sizeof(sampleMp3));
    HWPlayer.playChunk(playChunk, sizeToPlay);
}

void player::loop(){
  for(;;){ // infinite loop

    // Pause the task again for 50ms
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void player::setVolume(int volume){
    HWPlayer.setVolume(volume);
}

// getBalance
// getVolume
// setTone
// setBalance
// setVolume
// stopSong
// playChunk
// startSong

// uint16_t seconds = player.getDecodedTime();
// player.clearDecodedTime()