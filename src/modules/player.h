#ifndef player_H
#define player_H

#include <Arduino.h>
#include <SPI.h>
#include <VS1053.h>


class player {
    private:
        uint8_t curvol;                         // Current volume setting 0..100%
        int8_t  curbalance = 0;                 // Current balance setting -100..100
                                                // (-100 = right channel silent, 100 = left channel silent)
        VS1053 HWPlayer;
        // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
        uint8_t mp3buff[64];

    protected:
        uint16_t wram_read(uint16_t address);

    public:
        // Constructor.  Only sets pin values.  Doesn't touch the chip.  Be sure to call begin()!
        player(uint8_t _cs_pin, uint8_t _dcs_pin, uint8_t _dreq_pin);

        // Begin operation.  Sets up Hardware
        void begin();
        
        //retrieves data and stuff
        void loop();
        
        void playChunk(int sizeToPlay, char* playChunk);

        void setVolume(int volume);
};

#endif