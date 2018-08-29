#include "TimerFive.h" // TimerFive Library for pulsed optogenetic stimulation using interrupts
#include "digitalWriteFast.h" // digitalWrite macros: https://forum.arduino.cc/index.php?topic=46896.0 http://forum.arduino.cc/index.php?topic=498641.0 // pins need to be constants to speed up

const int optoPin = 13; // Optogenetics laser trigger
const int inputPin = 2; // Input TTL: when on, opto triggered. Mega: Interrupt Pins = 2, 3, 18, 19, 20, 21. https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

// Pulse object (e.g. for pulsed optogenetic stimulation)
typedef struct {
  volatile bool state;
  unsigned long us_on;
  unsigned long us_off;
  unsigned long freq;
  unsigned long ms_pulse;
  unsigned long ms_dur;
}
PulseObj;
PulseObj pulseOpto;

// BEGINNNING OF FUNCTIONS
void setup() {
  // SETUP Arduino: Output pins
  pinMode(optoPin, OUTPUT);
  pinMode(inputPin, INPUT);

  // Initialize Timer object in case will pulse opto laser, attach function, but stop
  Timer5.initialize(1e6); // initialize timer, and set a 1 second period (dummy for later)
  noInterrupts();
  Timer5.attachInterrupt(OptoPulse);  // attaches callback() as a timer overflow interrupt
  Timer5.stop();
  interrupts();
  digitalWriteFast(optoPin, LOW); // Reforce to go low
  pulseOpto.state = false;

  // Laser stimulation parameters
  pulseOpto.freq = 20; // Frequency of laser pulses
  pulseOpto.ms_pulse = 5; // Duration of each laser pulse (larger duration set by input TTL)
  // Convert laser stim parameters into us / microseconds
  pulseOpto.us_on = pulseOpto.ms_pulse * 1000;
  pulseOpto.us_off = (1e6 / pulseOpto.freq) - pulseOpto.us_on; // ISI onset-onset - time on

  // Set up interrupts for input triggering opto
  attachInterrupt(digitalPinToInterrupt(inputPin), OptoState, CHANGE);
}

void loop() { // Dummy loop, required for compile
}


// Turn timer on and off depending on new state
void OptoState() {
  if (digitalReadFast(inputPin) == HIGH) {
    Timer5.restart();
    Timer5.setPeriod(pulseOpto.us_on);
    pulseOpto.state = true;
  } else {
    Timer5.stop();
    pulseOpto.state = false;
  }
  digitalWriteFast(optoPin, pulseOpto.state);
}

// Turn opto/laser/TTL on/off depending on pulse timing
void OptoPulse() {
  // Takes 16-20us when checked in simple program over serial. Will block other calls
  pulseOpto.state = pulseOpto.state ^ 1; // Reverse/flip-flop state. Faster than '!' (16-20us vs. 28-32us)
  digitalWriteFast(optoPin, pulseOpto.state);
  if (pulseOpto.state) {
    Timer5.setPeriod(pulseOpto.us_on); // State now On: Set duration on in us
  } else {
    Timer5.setPeriod(pulseOpto.us_off); // State now Off: Set duration off in in us
  }
}

