// sounds.c
// various little "make a sound in firmware" methods
#include "alles.h"

// amy_add_event() adds amy_global.latency_ms to e.time, so take it off again for sounds that should play now
static uint32_t now_ms() {
    return amy_sysclock() - amy_global.latency_ms;
}

void note_on(int8_t osc, uint32_t time) {
    amy_event e = amy_default_event();
    e.osc = osc;
    e.time = time;
    e.velocity = 1;
    amy_add_event(&e);
}

// Set up osc 0 and 1 as two sines with an envelope, then trigger them
static void two_sines(float f0, float f1, char *bp) {
    amy_event e = amy_default_event();
    amy_parse_message(bp, &e);
    uint32_t time = now_ms();
    e.osc = 0;
    e.time = time;
    e.wave = SINE;
    e.freq_coefs[COEF_CONST] = f0;
    amy_add_event(&e);
    e.osc = 1;
    e.freq_coefs[COEF_CONST] = f1;
    amy_add_event(&e);

    note_on(0, time+1);
    note_on(1, time+1);
}

// Play a pew -- upgdrading
void upgrade_tone() {
    two_sines(220, 420, "A0,0,10,1,500,0,0,0");
}

// Play a sonar ping -- searching for wifi
void wifi_tone() {
    two_sines(440, 840, "A0,1,10,1,500,0,0,0");
}


// Schedule a bleep at sysclock time start
void bleep(uint32_t start) {
    amy_event e = amy_default_event();
    uint32_t sysclock = start - amy_global.latency_ms;
    e.osc = 0;
    e.time = sysclock;
    e.wave = SINE;
    e.freq_coefs[COEF_CONST] = 220;
    amy_add_event(&e);
    e.velocity = 1;
    e.pan_coefs[COEF_CONST] = 0.9;
    amy_add_event(&e);
    e.time = sysclock + 150;
    e.freq_coefs[COEF_CONST] = 440;
    e.pan_coefs[COEF_CONST] = 0.1;
    amy_add_event(&e);
    e.time = sysclock + 300;
    e.velocity = 0;
    e.pan_coefs[COEF_CONST] = 0.5;  // Restore default pan to osc 0.
    amy_add_event(&e);
}

void debleep() {
    amy_event e = amy_default_event();
    uint32_t sysclock = now_ms();
    e.osc = 0;
    e.time = sysclock;
    e.wave = SINE;
    e.freq_coefs[COEF_CONST] = 440;
    e.velocity = 1;
    amy_add_event(&e);
    e.time = sysclock + 150;
    e.freq_coefs[COEF_CONST] = 220;
    amy_add_event(&e);
    e.time = sysclock + 300;
    e.velocity = 0;
    e.freq_coefs[COEF_CONST] = 0;
    amy_add_event(&e);
}
