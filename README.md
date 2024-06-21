# DDS Chord Piano for the ATtiny85 Microcontroller

	* Features Direct Digital Synthesis (DDS) for generating multiple waveforms overtop each other

	* DDS analog output is created with Pulse-Width Modulation (PWM) and low-pass filtering

    - Amplified after filtering using NE5532N OpAmp

	- PWM uses the Phase-Locked Loop (PLL) as a clock source for faster PWM frequency. More info about PLL and PWM here(http://www.technoblogy.com/show?20W6)

	- Will feature multiple voices that can form chords

	- Shift register to extend the digital input pins of the microcontroller
