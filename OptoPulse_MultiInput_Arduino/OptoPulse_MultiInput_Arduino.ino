#include "TimerFive.h" // TimerFive Library for pulsed optogenetic stimulation using interrupts
#include "TimerFour.h" // TimerFive Library for pulsed optogenetic stimulation using interrupts
#include "digitalWriteFast.h" // digitalWrite macros: https://forum.arduino.cc/index.php?topic=46896.0 http://forum.arduino.cc/index.php?topic=498641.0 // pins need to be constants to speed up

// Timers: https://arduino-info.wikispaces.com/Timers-Arduino , https://oscarliang.com/arduino-timer-and-interrupt-tutorial/
//Pins 4 and 13: controlled by timer0
//Pins 11 and 12: controlled by timer1
//Pins 9 and10: controlled by timer2
//Pin 2, 3 and 5: controlled by timer 3
//Pin 6, 7 and 8: controlled by timer 4
//Pin 46, 45 and 44:: controlled by timer 5

// Optogenetics parameters
const int optoPin = 13; // Optogenetics laser trigger
const int MAX_INPUTS = 3; // All arrays below must be <= MAX_INPUTS long
int inputPins[] = {2, 3, 4}; // Avoid 6, 7, 8, 44, 45, 46 as above due to timer conflicts, at least for PWM modes
int freq_opto[] = {5, 20, 20};
int burst_on_sec[] = {0, 0, 1};
int burst_off_sec[] = {0, 0, 3};
int ms_opto_pulse = 5; // Gets copied below to pulseOpto.us_pulse

// State parameters
bool last_input[MAX_INPUTS];
bool curr_input[MAX_INPUTS]; // States of current/last input, so that opto only changed on changes
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
PulseObj burstOpto;


// BEGINNNING OF FUNCTIONS
void setup() {
  // SETUP Arduino: Output * input pins
  pinMode(optoPin, OUTPUT);
  for (int i_input = 0; i_input < MAX_INPUTS; i_input++) {
    pinMode(inputPins[i_input], INPUT);
  }

  // Initialize Timer object in case will pulse opto laser, attach function, but stop
  Timer5.initialize(1e6); // initialize timer, and set a 1 second period (dummy for later)
  noInterrupts();
  Timer5.attachInterrupt(OptoPulse);  // attaches callback() as a timer overflow interrupt
  Timer5.stop();
  interrupts();
  digitalWriteFast(optoPin, LOW); // Reforce to go low
  pulseOpto.state = false;

  // Initialize Timer object in case will burst opto laser, attach function, but stop
  Timer4.initialize(1e6); // initialize timer, and set a 1 second period (dummy for later)
  noInterrupts();
  Timer4.attachInterrupt(OptoBurst);  // attaches callback() as a timer overflow interrupt
  Timer4.stop();
  interrupts();
  burstOpto.state = false;

  // Set laser stimulation parameters for pulse duration. Bursts are configured in real-time
  pulseOpto.us_on = ms_opto_pulse * 1E3; // Duration of each laser pulse (larger duration set by input TTL)

  // Initialize Serial communication
  Serial.begin(115200);
}


void loop() {
  // Reset check flags
  bool flag_change = false;
  int num_on = 0;
  int idx_on = -1; // Start at -1 since other pins 0 indexed

  // Go through each input pin: Note current state, if any are on, then compare if any have changed state, and which is on
  // Only process laser configuration below if any have changed state
  for (int i_input = 0; i_input < MAX_INPUTS; i_input++) {
    curr_input[i_input] = digitalRead(inputPins[i_input]);
    if (curr_input[i_input]) { // If input is on:
      num_on += curr_input[i_input]; // Note how many are on total: no changes below if >1 is on
      idx_on = i_input; // Note which pin that is on. This will be overwritten if more than one is on, hence tracking num_on as well
    }
    if (curr_input[i_input] != last_input[i_input]) { // If input has changed
      flag_change = true; // Process below if the pin has changed state
      last_input[i_input] = curr_input[i_input]; // Update last state of this pin now that a change has been flagged
    }
  }

  // If any pins have changed state: process new laser/stim settings
  if (flag_change) {
    if (0 == num_on) {    // If all off, then turn off opto: pulses and bursts
      if (burstOpto.us_on > 0) { // Burst mode: Turn off
        Timer4.stop();
      }
      if (pulseOpto.freq > 0) { // Pulsed light: Turn Off
        Timer5.stop();       //      TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52)); // from Timer5.stop(); clears all clock selects bits
      }
      pulseOpto.state = false;
      digitalWriteFast(optoPin, pulseOpto.state);
    } else if ((1 == num_on) && (idx_on > -1)) { // If one should be turned on
      // New state: Only one is on (may not be newest though from prior check depending on how flip-flops are communicated, e.g. flipped on before last flipped off
      // Set pulse freq, dur, train vs. burst based on idx_on
      if (burst_on_sec[idx_on] > 0) {
        burstOpto.us_on = burst_on_sec[idx_on] * 1E6;
        burstOpto.us_off = burst_off_sec[idx_on] * 1E6;
        Timer4.restart(); // Restart timer counter (does not get restarted in setPeriod apparently. Doesn't work well if restarted after (delayed first on pulse--misfires)
        Timer4.setPeriod(burstOpto.us_on); // State On: Set time on in us. Last 2 lines of set period are stop/start timer
      } else { // No bursts
        Timer4.stop();
        burstOpto.us_on = 0;
        burstOpto.us_off = 0;
      }
      pulseOpto.freq = freq_opto[idx_on];
      if (pulseOpto.freq > 0) { // Pulsed light: set appropriate period
        pulseOpto.us_off = (1e6 / pulseOpto.freq) - pulseOpto.us_on; // ISI onset-onset - time on
        Timer5.restart(); // Restart timer counter (does not get restarted in setPeriod apparently. Doesn't work well if restarted after (delayed first on pulse--misfires)
        Timer5.setPeriod(pulseOpto.us_on); // State On: Set time on in us. Last 2 lines of set period are stop/start timer
      } else { // Constant light, do not pulse
        Timer5.stop();       //      TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52)); // from Timer5.stop(); clears all clock selects bits
        pulseOpto.us_on = 0;
        pulseOpto.us_off = 0;
      }
      // Turn opto/laser/stim on
      pulseOpto.state = true;
      digitalWriteFast(optoPin, pulseOpto.state);
      // Other Error conditions:
      //    } else if (num_on > 1) {  // If more than one on, then there is an error, ignore until only 1 is on?
      //      //Serial.println("ERROR: More than 1 opto input requested, not changing"); // ERROR!: Check which one is on during next loop in case new trigger flipped on before last flipped off
      //    } else if (idx_on < 0) {
      //      //Serial.println("ERROR: Inputs indicated as on, but no clear input");
    }
  }
}

// Shorter interrupt for pulsing
void OptoPulse() {
  // Takes 16-20us when checked in simple program over serial. Will block other calls
  // Timer not reset in setPeriod, however, restarted by hitting prior setpoint
  pulseOpto.state = pulseOpto.state ^ 1; // Reverse/flip-flop state. Faster than '!' (16-20us vs. 28-32us)
  digitalWriteFast(optoPin, pulseOpto.state);
  if (pulseOpto.state) {
    Timer5.setPeriod(pulseOpto.us_on); // State On: Set time on in us
  } else {
    Timer5.setPeriod(pulseOpto.us_off); // State Off: Set time off in in us
  }
}

// Longer interrupt for bursting
void OptoBurst() {
  // Takes 16-20us when checked in simple program over serial. Will block other calls
  // Timer not reset in setPeriod, however, restarted by hitting prior setpoint
  burstOpto.state = burstOpto.state ^ 1; // Reverse/flip-flop state. Faster than '!' (16-20us vs. 28-32us)
  if (burstOpto.state) {
    // Turn on Opto
    if (pulseOpto.freq > 0) { // Pulsed light: set appropriate period
      Timer5.restart(); // Restart timer counter (does not get restarted in setPeriod apparently. Doesn't work well if restarted after (delayed first on pulse--misfires)
      Timer5.setPeriod(pulseOpto.us_on); // State On: Set time on in us. Last 2 lines of set period are stop/start timer
    }
    pulseOpto.state = true;
    digitalWriteFast(optoPin, pulseOpto.state);
    Timer4.setPeriod(burstOpto.us_on); // State On: Set time on in us
  } else {
    // Turn off opto
    if (pulseOpto.freq > 0) { // Pulsed light: Turn Off
      Timer5.stop();       //      TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52)); // from Timer5.stop(); clears all clock selects bits
    }
    pulseOpto.state = false;
    digitalWriteFast(optoPin, pulseOpto.state);
    Timer4.setPeriod(burstOpto.us_off); // State Off: Set time off in in us
  }
}


