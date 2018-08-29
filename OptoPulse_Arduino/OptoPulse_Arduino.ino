#include "TimerFive.h" // TimerFive Library for pulsed optogenetic stimulation using interrupts
#include "digitalWriteFast.h" // digitalWrite macros: https://forum.arduino.cc/index.php?topic=46896.0 http://forum.arduino.cc/index.php?topic=498641.0 // pins need to be constants to speed up

const int optoPin = 13; // Optogenetics laser trigger
const int inputPin = 53; // Input TTL: when on, opto triggered
bool last_input, curr_input; // States of current/last input, so that opto parameters are only changed on changes
// Rec use bool instead of boolean: https://www.arduino.cc/reference/en/language/variables/data-types/bool/

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

  // Initialize Serial communication
  Serial.begin(115200);
}


void loop() {
  curr_input = digitalReadFast(inputPin);
  if (curr_input != last_input) {
    // Only change the laser if there has been a change in the input
    last_input = curr_input;
    // Serial.println(curr_input);
    if (curr_input) {
      pulseOpto.state = true;
      Timer5.restart(); // Restart timer counter (does not get restarted in setPeriod apparently. Doesn't work well if restarted after (delayed first on pulse--misfires)
      //      TCNT5 = 0; // from TimerFive::restart()
      //      TCCR5B |= Timer5.clockSelectBits; // from Timer5.start();
    } else {
      Timer5.stop();       //      TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52)); // from Timer5.stop(); clears all clock selects bits
      // TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52)); // from Timer5.stop(); clears all clock selects bits
      pulseOpto.state = false;
    }
  }
}


void OptoPulse() {
  // Takes 16-20us when checked in simple program over serial. Will block other calls
  pulseOpto.state = pulseOpto.state ^ 1; // Reverse/flip-flop state. Faster than '!' (16-20us vs. 28-32us)
  digitalWriteFast(optoPin, pulseOpto.state);
  if (pulseOpto.state) {
    Timer5.setPeriod(pulseOpto.us_on); // State On: Set time on in us
  } else {
    Timer5.setPeriod(pulseOpto.us_off); // State Off: Set time off in in us
  }
}

