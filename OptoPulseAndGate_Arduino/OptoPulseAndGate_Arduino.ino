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

unsigned long us_delay = 2 * 1000; // Short delay so as not to overlap with ongoing imaging, shouldn't be necessary
unsigned long us_dur = 5 * 1000;

// BEGINNNING OF FUNCTIONS
void setup() {
  // SETUP Arduino: Output pins
  pinMode(optoPin, OUTPUT);
  pinMode(pulsePin, INPUT_PULLUP);
  pinMode(gatePin, INPUT_PULLUP);
}

void loop() {
  while (digitalReadFast(gatePin) == HIGH) { // Gate active: Enter pulse montoring loop
    while (digitalReadFast(pulsePin)  == HIGH) { } // Wait until pulse inactive to restart pulse monitoring loop. Assuming can not go low and then high again during pulse delay & duration
    if (digitalReadFast(gatePin) == LOW) {
      // Make sure gatePin is not low. If it is low, then exit loop early, otherwise will cont to wait for next pulse, even after done
      break;
    }
    while (digitalReadFast(pulsePin)  == LOW) { }// Wait for a clean entry into the first pulse: otherwise may start at the end of a frame: Inherent delay unfortunatley, but necessary for clean imaging to wait for transition from low to high
    if (digitalReadFast(gatePin) == LOW) {
      // Make sure gatePin is not low. If it is low, then exit loop early, don't proceed to pulse
      break;
    }
    // Once a pulse is initiated, complete it
    unsigned long us_trigger = micros(); // Mark onset of delay
    while (micros() - us_trigger < us_delay) { } // Loop until delay has passed
    digitalWriteFast(optoPin, HIGH); // Start pulse
    unsigned long us_pulse = micros(); // Mark onset of pulse
    while (micros() - us_pulse < us_dur) { } // Loop until pulse duration has passed
    digitalWriteFast(optoPin, LOW); // End pulse
  }
}
