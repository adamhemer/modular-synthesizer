// Constants
#define UINT_MAX    0xffffffff
// Pin Defintions
#define LATCH   4
#define CLOCK   5
#define DATA    6
#define IN_A    3
#define IN_B    2

// Settings
#define TRIG_LENGTH 5
#define LED_LENGTH  200
#define STARTUP_BOUNCE  true
#define BOUNCE_TIME     25
#define LOOP_DELAY  500     // 0.5ms

long long clockA = 0;
long long clockB = 0;

unsigned long clockA_trig = 0;
unsigned long clockA_led = 0;
unsigned long clockB_trig = 0;
unsigned long clockB_led = 0;

int ASum = 0;
int BSum = 0;
int C_A_leds = 0;
int C_A_trig = 0;
int C_B_leds = 0;
int C_B_trig = 0;

void interruptA() {
    clockA_trig = millis() + TRIG_LENGTH;
    clockA_led = millis() + LED_LENGTH;
    clockA++;
    updateOutputA();
}

void interruptB() {
    clockB_trig = millis() + TRIG_LENGTH;
    clockB_led = millis() + LED_LENGTH;
    clockB++;
    updateOutputB();
}

void updateOutputA() {
    // Determine output for A channel
    int A2 = (clockA % 2 == 0);
    int A3 = (clockA % 3 == 0) << 1;
    int A4 = (clockA % 4 == 0) << 2;
    int A5 = (clockA % 5 == 0) << 3;
    int A6 = (clockA % 6 == 0) << 4;
    int A7 = (clockA % 7 == 0) << 5;
    int A8 = (clockA % 8 == 0) << 6;
    int A16 = (clockA % 16 == 0) << 7;

    // Combine into one byte for shifting
    ASum = A2 + A3 + A4 + A5 + A6 + A7 + A8 + A16;

    // Calculate outputs for shift register 5
    int Ainput = 1 << 5;
    int A32_led = (clockA % 32 == 0) << 3;
    int A32_trig = (clockA % 32 == 0);

    C_A_leds = Ainput + A32_led;
    C_A_trig = A32_trig;

    // Shift out data
    updateShiftRegisters();
}

void updateOutputB() {
    // Determine output for B channel
    int B2 = (clockB % 2 == 0);
    int B3 = (clockB % 3 == 0) << 1;
    int B4 = (clockB % 4 == 0) << 2;
    int B5 = (clockB % 5 == 0) << 3;
    int B6 = (clockB % 6 == 0) << 4;
    int B7 = (clockB % 7 == 0) << 5;
    int B8 = (clockB % 8 == 0) << 6;
    int B16 = (clockB % 16 == 0) << 7;

    // Combine into one byte for shifting
    BSum = B2 + B3 + B4 + B5 + B6 + B7 + B8 + B16;

    // Calculate outputs for shift register 5
    int Binput = 1 << 4;
    int B32_led = (clockB % 32 == 0) << 2;
    int B32_trig = (clockB % 32 == 0) << 1;

    C_B_leds = Binput + B32_led;
    C_B_trig = B32_trig;

    // Shift out data
    updateShiftRegisters();
}

void updateShiftRegisters() {
    int A_leds = 0, B_leds = 0, A_outputs = 0, B_outputs = 0, C = 0;

    // If Triggers for A are active
    if (clockA_trig != UINT_MAX) {
        A_outputs = ASum;
        C += C_A_trig;
    }

    // If Triggers for B are active
    if (clockB_trig != UINT_MAX) {
        B_outputs = BSum;
        C += C_B_trig;
    }

    // If LEDs for A are be on
    if (clockA_led != UINT_MAX) {
        A_leds = ASum;
        C += C_A_leds;
    }

    // If LEDs for B are be on
    if (clockB_led != UINT_MAX) {
        B_leds = BSum;
        C += C_B_leds;
    }

    // Output to shift registers
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, MSBFIRST, C);
    shiftOut(DATA, CLOCK, MSBFIRST, A_leds);
    shiftOut(DATA, CLOCK, MSBFIRST, B_outputs);
    shiftOut(DATA, CLOCK, MSBFIRST, B_leds);
    shiftOut(DATA, CLOCK, MSBFIRST, A_outputs);
    digitalWrite(LATCH, HIGH);
}

void setup() {
    // DEBUG ONLY
    // Serial.begin(115200);
    // Serial.println("Serial started.");
    
    clockA = 0;
    clockB = 0;

    // Pin Definitions
    pinMode(LATCH, OUTPUT);
    pinMode(CLOCK, OUTPUT);
    pinMode(DATA, OUTPUT);
    digitalWrite(LATCH, HIGH);
    pinMode(LED_BUILTIN, OUTPUT);

    // Startup Bounce
    if (STARTUP_BOUNCE) {
        for (int i = 0; i < 20; i++) {
            indexLed(i);
            delay(BOUNCE_TIME);
        }

        for (int i = 18; i > 0; i--) {
            indexLed(i);
            delay(BOUNCE_TIME);
        }
    }

    attachInterrupt(digitalPinToInterrupt(IN_A), interruptA, FALLING); // Inputs inverted
    attachInterrupt(digitalPinToInterrupt(IN_B), interruptB, FALLING);
}

void loop() {
    if (millis() > clockA_trig) { // Turn off the triggers after a few ms
        clockA_trig = UINT_MAX;    // Unsigned max value
        updateOutputA();
        updateShiftRegisters();
    }
    if (millis() > clockA_led) {   // Leave LEDs on for longer for visual effect
        clockA_led = UINT_MAX;     // Unsigned max value
        updateOutputA();
        updateShiftRegisters();
    }

    if (millis() > clockB_trig) { // Turn off the triggers after a few ms
        clockB_trig = UINT_MAX;    // Unsigned max value
        updateOutputB();
        updateShiftRegisters();
    }
    if (millis() > clockB_led) {   // Leave LEDs on for longer for visual effect
        clockB_led = UINT_MAX;     // Unsigned max value
        updateOutputB();
        updateShiftRegisters();
    }
    delayMicroseconds(LOOP_DELAY);
}

// TESTING ONLY - Activates LED indexed by i from 0 to 20.
void indexLed(int i) {
    // Calculate register values
    int _1 = 0, _2 = 0, _3 = 0, _4 = 0, _5 = 0;
    if (i < 2) {
        if (i == 0) {
            _1 = 1 << 5;
        } else {
            _1 = 1 << 4;
        }
    }
    else if (i < 18) {
        int x = i - 2;
        if (x % 2) { 
            _4 = 1 << ((x - 1) / 2);       
        } else {
            _2 = 1 << (x / 2);
        }
    } else {
        int val = 0;
        if (i == 18) {
            _1 = 1 << 3;
        } else if (i == 19) {
            _1 = 1 << 2;
        }     
    }

    // Shift out
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, MSBFIRST, _1);
    shiftOut(DATA, CLOCK, MSBFIRST, _2);
    shiftOut(DATA, CLOCK, MSBFIRST, _3);
    shiftOut(DATA, CLOCK, MSBFIRST, _4);
    shiftOut(DATA, CLOCK, MSBFIRST, _5);
    digitalWrite(LATCH, HIGH);     
}