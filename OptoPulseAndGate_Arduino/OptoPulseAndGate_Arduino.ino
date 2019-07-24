/*
    OptoPulseAndGate_Arduino

    Emits a pulse of a predetermined duration
    when triggered by the pulse input,
    as long as Gate is High as well

    Frequency is determined by the pulse inputs
    But the pulse width is determined by this protocol
*/

#include "digitalWriteFast.h" // digitalWrite macros: https://forum.arduino.cc/index.php?topic=46896.0 http://forum.arduino.cc/index.php?topic=498641.0 // pins need to be constants to speed up

const int optoPin = LED_BUILTIN; // Optogenetics laser trigger: LED_BUILTIN = 13
const int pulsePin = 2; // Input TTL: when on, opto triggered if gatePin is also on. Mega: Interrupt Pins = 2, 3, 18, 19, 20, 21. https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
const int gatePin = 3; // Input TTL: when on, opto triggered for each pulse on pulsePin. Mega: Interrupt Pins = 2, 3, 18, 19, 20, 21. https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

volatile bool gateState, pulseState, pulseLast, optoOn;
unsigned long us_on;
unsigned long us_dur = 5 * 1000;

// BEGINNNING OF FUNCTIONS
void setup() {
  // SETUP Arduino: Output pins
  pinMode(optoPin, OUTPUT);
  pinMode(pulsePin, INPUT_PULLUP);
  pinMode(gatePin, INPUT_PULLUP);

  noInterrupts();
  GateState(); // Initialize gate state, afterwards will only check with changes
  PulseState(); // Initialize gate state, afterwards will only check with changes
  attachInterrupt(digitalPinToInterrupt(gatePin), GateState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pulsePin), PulseState, CHANGE);
  interrupts(); // Restart interrupts: ready to go!

  Serial.begin(115200);
}

void loop() { // If gate active, pulse was not on, but now is on, then give a pulse!
//  unsigned long us = micros();
  if (gateState && !pulseLast && pulseState && !optoOn) {
    digitalWriteFast(optoPin, HIGH);
    us_on = micros();
    optoOn = true;
    pulseLast = true;
//    Serial.print("Pulse On ");
//    Serial.print(" ");
//    Serial.print(us);
//    Serial.print(" ");
//    Serial.print(us_on);
//    Serial.print(" ");
//    Serial.print(us_on - us);
//    Serial.println();
  }
  
  if (optoOn && (micros() - us_on >= us_dur)) {
    digitalWriteFast(optoPin, LOW);
    optoOn = false;
//    Serial.print("Pulse Off ");
//    Serial.print(micros());
//    Serial.print(" ");
//    Serial.print(us_on);
//    Serial.print(" ");
//    Serial.print(us_dur);
//    Serial.print(" ");
//    Serial.print(micros() - us_on);
//    Serial.println();
  }

  if (pulseLast && !pulseState && !optoOn) {
    pulseLast = false;
  }
}


// Register/save state of gate depending on new state, minimize repeated reading of gatePin
void GateState() {
  gateState = digitalReadFast(gatePin);
}

// Register/save state of gate depending on new state, minimize repeated reading of gatePin
void PulseState() {
  pulseState = digitalReadFast(pulsePin);
}
