/*
    OptoPulseAndGate_Interrupt_Arduino

    Emits a pulse when triggered by the pulse input,
    as long as Gate is High as well

    Frequency is determined by the pulse inputs
    But the pulse width is determined by this protocol
*/

#include "TimerFive.h" // TimerFive Library for pulsed optogenetic stimulation using interrupts
#include "digitalWriteFast.h" // digitalWrite macros: https://forum.arduino.cc/index.php?topic=46896.0 http://forum.arduino.cc/index.php?topic=498641.0 // pins need to be constants to speed up

const int optoPin = 13; // Optogenetics laser trigger
const int pulsePin = 2; // Input TTL: when on, opto triggered if gatePin is also on. Mega: Interrupt Pins = 2, 3, 18, 19, 20, 21. https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
const int gatePin = 3; // Input TTL: when on, opto triggered for each pulse on pulsePin. Mega: Interrupt Pins = 2, 3, 18, 19, 20, 21. https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

volatile bool gateState;

// BEGINNNING OF FUNCTIONS
void setup() {
  // SETUP Arduino: Output pins
  pinMode(optoPin, OUTPUT);
  pinMode(pulsePin, INPUT);
  pinMode(gatePin, INPUT);

  // Initialize Timer object in case will pulse opto laser, attach function, but stop
  Timer5.initialize(1e6); // initialize timer, and set a 1 second period (dummy for later)
  noInterrupts();
  Timer5.attachInterrupt(OptoPulse);  // attaches callback() as a timer overflow interrupt
  Timer5.stop();
  interrupts();
  digitalWriteFast(optoPin, LOW); // Reforce to go low

  // Laser stimulation parameters in us / microseconds
  long us_pulse = 5 * 1000;
  Timer5.setPeriod(us_pulse);

  // Set up interrupts for input triggering opto
  attachInterrupt(digitalPinToInterrupt(gatePin), GateState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pulsePin), PulseOn, RISING);
}

void loop() { // Dummy loop, required for compile
}


// Register/save state of gate depending on new state, minimize repeated reading of gatePin
void GateState() {
  gateState = digitalReadFast(gatePin);
}

// Turn pulse on depending on gateState
void PulseOn() {
  if (gateState) {
    Timer5.restart();
    digitalWriteFast(optoPin, HIGH);
    Timer5.start();
  }
}

// Turn opto/laser/TTL on/off depending on pulse timing: Should only ever occur when pulse state is true
void OptoPulse() {
  digitalWriteFast(optoPin, LOW);
  Timer5.stop();
}

