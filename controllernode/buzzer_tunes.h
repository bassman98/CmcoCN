#ifndef BUZZER_TUNES_H
#define BUZZER_TUNES_H

#include <Arduino.h>
#include "piano_notes.h"

struct Note {
    float freq;
    uint16_t durationMs;
};

// Play any array of notes
void playTune(const Note *tune, uint8_t count);

// Convenience functions
void playSuccess();
void playError();
void playMario();
void playRadRacer();

// Legacy shim: forward to include/buzzer_tunes.h

#ifndef CONTROLLERNODE_BUZZER_TUNES_SHIM_H
#define CONTROLLERNODE_BUZZER_TUNES_SHIM_H

#include "../include/buzzer_tunes.h"

#endif // CONTROLLERNODE_BUZZER_TUNES_SHIM_H