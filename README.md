# OptoPulseArduino
Arduino sketches for triggering pulses, to drive optogenetic light sources (lasers, LEDs)


There are several sketches:
1. OptoPulse_Arduino: Relatively simple conversion of 1 input to 1 output train of pulses
2. OptoPulse_Interrupt_Arduino: Interrupt driven conversion of 1 input to 1 output train of pulses
3. OptoPulse_MultiInput_Arduino: digitalRead driven conversion of multiple inputs to 1 output train of pulses or bursts
4. OptoPulseAndGate_Interrupt_Arduino: Interrupt driven conversion of 2 inputs to 1 output train of pulses (Inputs represent a gate and a series of pulse initiations)

The emphasis in general above is code interpretation and flexibility, not necessarily speed. When checked using an oscilloscope, accuracy is probably <1ms (between 10-500 us).


This code makes use of several librariers:
1. digitalWriteFast.h : https://github.com/watterott/Arduino-Libs
2. TimerFive : http://letsmakerobots.com/files/Arduino101-Timers.zip
3. TimerFour : http://letsmakerobots.com/files/Arduino101-Timers.zip
