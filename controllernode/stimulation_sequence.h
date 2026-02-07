#ifndef STIMULATION_SEQUENCE_H
#define STIMULATION_SEQUENCE_H

#include <Arduino.h>
// Legacy shim: forward to include/stimulation_sequence.h

#ifndef CONTROLLERNODE_STIMULATION_SEQUENCE_SHIM_H
#define CONTROLLERNODE_STIMULATION_SEQUENCE_SHIM_H

#include "../include/stimulation_sequence.h"

#endif // CONTROLLERNODE_STIMULATION_SEQUENCE_SHIM_H
public:
    StimulationSequence() = default;

    StimulationPeriod stimPeriods[NUM_PERIODS];

    void begin(uint32_t seed) {
        randomSeed(seed);
        buildSequence();
        reset();
    }

    void reset() {
        _currentPeriod = 0;
        _phase = Phase::PRE_DELAY;
        _phaseStartMs = millis();
        clearBuzzers();
    }

    void update() {
        // Smoothly pull syncOffset toward pendingOffset
        constexpr float pullRate = 0.02f; // 2% per update
        syncOffsetUs += (pendingOffsetUs - syncOffsetUs) * pullRate;

        // Convert syncOffsetUs to milliseconds
        float syncOffsetMs = syncOffsetUs / 1000.0f;

        // Adjust phase start time
        _phaseStartMs -= (uint32_t)syncOffsetMs;
        
        if (_currentPeriod >= NUM_PERIODS) return;

        StimulationPeriod& p = stimPeriods[_currentPeriod];
        uint32_t now = millis();
        float elapsed = (float)(now - _phaseStartMs);

        switch (_phase) {

        case Phase::PRE_DELAY:
            if (elapsed >= p.preDelayMs) {
                startPulse(p);
                _phase = Phase::PULSE;
                _phaseStartMs = now;
            }
            break;

        case Phase::PULSE:
            if (elapsed >= p.pulseWidthMs) {
                stopPulse(p);
                _phase = Phase::POST_DELAY;
                _phaseStartMs = now;
            }
            break;

        case Phase::POST_DELAY:
            if (elapsed >= p.postDelayMs) {
                _currentPeriod++;
                _phase = Phase::PRE_DELAY;
                _phaseStartMs = now;
            }
            break;
        }
    }

    bool isFinished() const {
        return _currentPeriod >= NUM_PERIODS;
    }

    bool isActive() const {
        return stimPeriods[_currentPeriod].active;
    }

    void setSyncOffset(float offsetUs) {
        constexpr float MAX_STEP_US = 2000.0f; // clamp extreme corrections

        float delta = offsetUs - pendingOffsetUs;

        if (delta > MAX_STEP_US) delta = MAX_STEP_US;
        if (delta < -MAX_STEP_US) delta = -MAX_STEP_US;

        pendingOffsetUs += delta;
    }


private:
    // ---------------- INTERNAL ----------------

    enum class Phase { PRE_DELAY, PULSE, POST_DELAY };

    bool _buzzerStates[NUM_FINGERS] = { false, false, false, false };

    float syncOffsetUs = 0.0f;       // current applied offset
    float pendingOffsetUs = 0.0f;    // target offset for smoothing
    uint8_t  _currentPeriod = 0;
    Phase    _phase = Phase::PRE_DELAY;
    uint32_t _phaseStartMs = 0;

    // ---------------- SEQUENCE BUILD ----------------

    void buildSequence() {
        // ---- ACTIVE PERIODS (0–11) ----
        for (uint8_t group = 0; group < 3; group++) {
            uint8_t fingers[NUM_FINGERS] = {0, 1, 2, 3};
            shuffle(fingers, NUM_FINGERS);

            for (uint8_t i = 0; i < NUM_FINGERS; i++) {
                float pre = randomJitter();
                float freq = randomFreq();
                uint8_t idx = group * NUM_FINGERS + i;
                
                stimPeriods[idx] = {
                    pre,
                    PULSE_WIDTH_MS,
                    MAX_JITTER_MS - pre,
                    true,
                    fingers[i],
                    freq
                };
            }
        }

        // ---- INACTIVE PERIODS (12–19) ----
        for (uint8_t i = 12; i < NUM_PERIODS; i++) {
            stimPeriods[i] = {
                TOTAL_TIME_MS,
                0.0f,
                0.0f,
                false,
                (uint8_t)random(NUM_FINGERS)
            };
        }
    }

    float randomFreq() {
        if(!FREQ_RANDOM_ENABLED) return DEFAULT_FREQ;
        return random(FREQ_RANDOM_MIN, FREQ_RANDOM_MAX + 1);
    }

    float randomJitter() {
        if(!JITTER_ENABLED) return 0;
        return random(0, (int)(MAX_PRE_JITTER_MS * 10)) / 10.0f;
    }

    void shuffle(uint8_t* arr, uint8_t size) {
        for (int i = size - 1; i > 0; i--) {
            int j = random(i + 1);
            uint8_t t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;
        }
    }

    void setPWM (int freq, int index, int dutyCycle) {
        uint8_t channel = index % 16;
        ledcSetup(channel, freq, PWM_RESOLUTION);
        ledcAttachPin(PWM_PINS[index], channel);
        ledcWrite(channel, dutyCycle);
    }

    // ---------------- BUZZER + SYNC CONTROL ----------------

    void clearBuzzers() {
        for (uint8_t i = 0; i < NUM_FINGERS; i++) {
            _buzzerStates[i] = false;
        }
    }

    void startPulse(const StimulationPeriod& p) {
        if (!p.active || p.pulseWidthMs <= 0) return;

        clearBuzzers();
        _buzzerStates[p.fingerIndex] = true;
        setPWM(p.frequency, p.fingerIndex, DUTYCYCLE_ON);
    }

    void stopPulse(const StimulationPeriod& p) {
        if (!p.active || p.pulseWidthMs <= 0) return;

        _buzzerStates[p.fingerIndex] = false;
        setPWM(p.frequency, p.fingerIndex, DUTYCYCLE_OFF);
    }
};

#endif