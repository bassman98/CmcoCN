#include "buzzer_tunes.h"
#include "config.h"
#include <Arduino.h>

// -------------------------
// Play a single tune
// -------------------------
void playTune(const Note *tune, uint8_t count) {
    randomSeed(count);
    uint8_t pinIndex = random(4);  // 0 to 3 inclusive
    uint8_t buzzerPin = PWM_PINS[pinIndex];
    
    for (uint8_t i = 0; i < count; i++) {
        pinIndex = random(4);  // 0 to 3 inclusive
        buzzerPin = PWM_PINS[pinIndex];
        
        if (tune[i].freq > 0.0f) {
            // Attach PWM to buzzer
            uint32_t freq = (uint32_t)tune[i].freq;
            ledcAttach(buzzerPin, freq, PWM_RESOLUTION);
            ledcChangeFrequency(buzzerPin, freq, PWM_RESOLUTION);
            ledcWrite(buzzerPin, DUTYCYCLE_ON);
        } else {
            // REST: silence
            ledcWrite(buzzerPin, DUTYCYCLE_OFF);
        }

        delay(tune[i].durationMs);

        // Stop note
        ledcWrite(buzzerPin, DUTYCYCLE_OFF);
        delay(20); // short pause between notes
    }
    
    // Detach PWM pin after playing tune to avoid conflicts
    ledcDetach(buzzerPin);
}

// -------------------------
// Predefined Tunes
// -------------------------
static const Note successTune[] = {
    { xC5, 120 }, { xREST, 50 },
    { xE5, 120 }, { xREST, 50 },
    { xG5, 200 }
};

static const Note errorTune[] = {
    { xG5, 150 }, { xREST, 50 },
    { xE5, 150 }, { xREST, 50 },
    { xC5, 300 }
};

// -------------------------
// Super Mario Bros Jingle
// Only the first few notes (intro melody)
// -------------------------
static const Note marioTune[] = {
    // Opening arpeggio
    { xE5, 125 }, { xE5, 125 }, { xREST, 125 }, { xE5, 125 },
    { xREST, 125 }, { xC5, 125 }, { xE5, 125 }, { xREST, 125 },
    { xG5, 375 }, { xREST, 375 },
    { xG4, 375 }, { xREST, 375 },
    
    // Main phrase start
    { xC5, 250 }, { xG4, 250 }, { xE4, 250 },
    // Flourish
    { xA4, 125 }, { xB4, 125 }, { xAs4, 125 }, { xA4, 125 },
    // Bridge
    { xG4, 187 }, { xE5, 125 }, { xG5, 125 }, { xA5, 250 },
    { xF5, 125 }, { xG5, 125 },
    
    // Partial repeat for "first few notes"
    { xREST, 125 }, { xE5, 125 }, { xC5, 125 }, { xG4, 125 },
    { xREST, 250 }
};

static const Note radRacerTune[] = {
    // Intro fanfare + main riff (Radio Station 1 lead)
    { xE5, 125 }, { xG5, 125 }, { xB5, 250 }, { xREST, 125 },
    { xA5, 125 }, { xG5, 250 }, { xF5, 125 }, { xE5, 250 },
    
    { xD5, 125 }, { xE5, 125 }, { xF5, 125 }, { xG5, 250 },
    { xREST, 125 }, { xA5, 125 }, { xG5, 250 }, { xF5, 125 },
    
    { xE5, 250 }, { xG5, 125 }, { xB5, 125 }, { xA5, 250 },
    { xG5, 375 }, { xREST, 375 },
    
    // Bridge/loop point
    { xC5, 125 }, { xE5, 125 }, { xG5, 250 }, { xF5, 125 },
    { xE5, 125 }, { xD5, 250 }, { xREST, 250 },
    
    { xE4, 250 }, { xG4, 125 }, { xB4, 125 }, { xREST, 125 }
};

// -------------------------
// Convenience Play Functions
// -------------------------
void playSuccess() {
    playTune(successTune, sizeof(successTune)/sizeof(successTune[0]));
}

void playError() {
    playTune(errorTune, sizeof(errorTune)/sizeof(errorTune[0]));
}

void playMario() {
    playTune(marioTune, sizeof(marioTune)/sizeof(marioTune[0]));
}

void playRadRacer() {
    playTune(radRacerTune, sizeof(radRacerTune)/sizeof(radRacerTune[0]));
}