# OptoPulseArduino
Arduino sketches for triggering pulses, to drive optogenetic light sources (lasers, LEDs)


There are several sketches:
1. OptoPulse_Arduino: Relatively simple conversion of 1 input to 1 output train of pulses
2. OptoPulse_Interrupt_Arduino: Interrupt driven conversion of 1 input to 1 output train of pulses
3. OptoPulse_MultiInput_Arduino: digitalRead driven conversion of multiple inputs to 1 output train of pulses or bursts


The emphasis in general above is code interpretation and flexibility, not necessarily speed.


This code makes use of several librariers:
digitalWriteFast.h : https://github.com/watterott/Arduino-Libs
TimerFive : http://letsmakerobots.com/files/Arduino101-Timers.zip
TimerFour : http://letsmakerobots.com/files/Arduino101-Timers.zip
