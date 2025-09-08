# 4ms VCV Rack Core Modules User Guide 

## Atvert

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Atvert.png" width ="48">

>Atvert is a 2-channel attenuverter.
It lets you scale or invert two input signals using two knobs. If no input is connected, it uses a default +5V offset.

### Controls

* **2 knobs (one per channel):**
Adjust from -100% (inverted) to +100% (normal). Center = 0% (no output)

## BPF

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/BPF.png" width ="48">

>BPF is a 12db per octave resonant bandpass filter with two filter types: "Standard" which is based on a Korg style sallen-key topology, and "Oberheim" which is based on the filter in the SEM.

### Controls 
* **Cutoff:** filter cutoff frequency, 261hz - 1041hz
* **Q:** filter resonance, 1-20x
* **Mode:** toggle between Korg and Oberheim mode
* **CV:** CV Input for filter cutoff, -5v/+5v.
* **Input:** Audio input 
* **Out:** Audio output 

## CLKD

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/CLKD.png" width ="48">

>CLKD is a simple clock divider that takes in incoming gates and divides their timing by a given amount. 

### Controls 

* **Divide:** from "=" (no division) to /16.
* **CV:** CV input for clock division, -5v/+5v 
* **Clk In:** Clock input
* **Clock:** Clock output 

## CLKM

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/CLKM.png" width ="48">

>CLKM is a simple clock multiplier that takes in incoming gates and multiplies their timing by a given amount. 

### Controls 
* **Multiply:** from "x1" (no multiplication) to x16.
* **CV:** CV input for clock multiplication, -5v/+5v 
* **Clk In:** Clock input
* **Clock:** Clock output 

## Complex EG

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Complex%20EG.png" width ="168">

>Complex EG is an ADSHR envelope generator with control over curve shape for attack, decay, and release time. This envelope is particularly useful with a keyboard, where long note holds can control complex changes of a sound over time.

### Controls 
* **Attack:** The attack phase (rise time) of the envelope, 1ms - 1sec. 
* **Decay:** The decay phase (amount of time from attack to release stage) of the envelope, 1ms - 1sec.
* **Release:** The release phase (amount of time after the sustain stage for signal to fall to 0) of the envelope, 1ms - 1 sec. 
* **A Curve:** Change the time scaling of the attack curve continously from linear to logarithmic. 
* **D Curve:** Change the time scaling of the decay curve continously from linear to logarithmic. 
* **R Curve:** Change the time scaling of the release curve continously from linear to logarithmic. 
* **Sustain:** The sustain phase (volume at which sound is held after decay stage) of the envelope. 0-8v. 
* **Loop:** Toggle for engaging looping behavior of the envelope. If off, envelope needs external gate.  
* **Hold:** The hold phase (amount of time after note off for release phase to begin) of the envelope. 1ms-1sec. 
* **Gate In:** Gate input for beginning the envelope generator. 
* **Attack CV In:** CV input for attack time, -5v/+5v.
* **Decay CV In:** CV input for decay time, -5v/+5v.
* **Release CV In:** CV input for release time, -5v/+5v.
* **Sustain CV In:** CV input for sustain volume, -5v/+5v.
* **Hold CV In:** CV input for hold time, -5v/+5v.
* **Attack Out:** CV output for attack stage.
* **Decay Out:** CV output for decay stage.
* **Release Out:** CV output for release stage.
* **Sustain Out:** CV output for sustain stage.
* **Hold Out:** CV output for Hold stage. 
* **Envelope Out:** Full Envelope output.

## Detune 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Detune.png" width ="96">

>Detune is a "Wow and flutter" tape simulator in which the incoming signal is slowly modulated by a pseudo random pitch shifter. The purpose of this effect is to mimick the behavior of a poorly calibrated tape machine. 

### Controls 
* **W Speed:** "Wow" speed (slower pitch modulation) 0.1hz - 5.33hz.
* **F Speed:** "Flutter" speed (faster pitch modulation) 5hz - 30hz.
* **W Depth:** "Wow" effect amount. 
* **F Depth:** "Flutter" effect amount.
* **Input:** Audio input. 
* **Detune CV In:** CV input for both W and F speeds, -5v/+5v. 
* **Depth CV In:** CV input for both W and F depths, -5v/+5v.
* **Output:** Audio output

## Djembe

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Djembe.png" width ="96">

>Djembe is a drum module based on the sonic characteristic of a real acoustic Djembe. It uses a simple exciter with resonant filters. 

### Controls 
* **Pitch:** The overall pitch of the drum 20hz - 500hz.
* **Sharpness:** The overall volume of the drum.
* **Hit:** A lowpass filter for the exciter portion of the sound. 
* **Strike Amount:** Exciter volume. 
* **Pitch CV In:** Pitch CV input -5v/+5v.
* **Sharp CV In:** Sharp CV input -5v/+5v.
* **Hit CV In:** Hit CV input -5v/+5v.
* **Strike CV In:** Strike CV input -5v/+5v.
* **Trigger In:** Trigger input.
* **Out:** Audio output. 

## Drum 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Drum.png" width ="156">

>Drum is a drum module based on a simple two operator FM oscillator core an da noise source. 

### Controls 
* **Pitch:** The base pitch of the carrier and modulator. 10hz - 1000hz.
* **Pitch Env:** A decay envelope that controls the pitch of the carrier. 
* **Pitch Amt:** The amount the decay envelope modulates the pitch of the carrier. 
* **Ratio:** The frequency ratio for the modulator. Ratio is based on carrier frequency. 
* **FM Env:** The decay time for an envelope that controls the depth of FM.
* **FM Amt:** The overall amount of FM present in the sound. 
* **Tone Env:** The decay time for the modulated carrier. 
* **Noise Env:** The decay time for the noise portion of the signal. 
* **Noise Blend:** A crossfader that fades between the oscillators and noise source. 
* **Trigger In:** Trigger input for all decay envelopes. 
* **V/Oct CV In:** 1v/oct pitch input for carrier and modulator. 
* **P Env CV In:** CV input for pitch envelope -5v/+5v. 
* **P Amt CV In:** CV input for pitch enveliope depth -5v/+5v. 
* **Ratio CV In:** CV input for pitch ratio of modulator -5v/+5v. 
* **FM Env CV In:** CV input for FM decay envelope -5v/+5v. 
* **FM Amt CV In:** CV input for FM amount -5v/+5v.
* **T Env CV In:** CV input for tone envelope -5v/+5v.
* **N Env CV In:** CV input for noise envelope -5v/+5v. 
* **N Blend CV In:** CV input for noise blend -5v/+5v.
* **Inv Out:** CV output for inverted version of the tone envelope. 
* **Out:** Drum audio output. 

## FM

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/FM.png" width ="96">

>FM is a two operator FM oscillator with variable waveshape. 

### Controls 
* **Pitch:** Base frequency of carrier and modulator, 20hz - 20khz.
* **Mix:** A crossfader that blends between carrier and modulated carrier. 
* **Index:** Depth of frequency FM
* **Index CV:** An attenuverter for the Index parameter + CV. 
* **Ratio C:** Coarse frequency adjustment for modulator pitch ratio. 
* **Ratio F:** Fine frequency adjustment for modulator pitch ratio. 
* **Shape:** Blend from continously from sine to square.
* **Shape CV:** An attenuverter for shape parameter + CV. 
* **V/Oct C CV In:** 1v/Oct pitch input for carrier.
* **V/Oct M CV In:** 1v/Oct pitch input for modulator.
* **Mix CV In:** CV input for mix, -5v/+5v. 
* **Index CV In:** CV input for index, -5v/+5v.
* **Shape CV In:** CV input for shape, -5v/+5v.
* **Out:** Audio output. 

## FLW 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/FLW.png" width ="48">

>FLW is a two stage follower that generates an envelope based on the followed signal. An audio or CV input is fed in and a variable ramp CV is created based on the incoming signal. 

### Controls 
* **Rise:** Rise time for the envelope, 1ms - 2sec.
* **Fall:** Fall time for the envelope, 1ms - 2sec. 
* **Thresh:** The voltage threshold at which the envelope follower is triggered, 0-8v. 
* **Input:** CV or audio input.
* **Gate Out:** A gate output that flips high and low when input signal crosses the threshold. 
* **Env Out:** Envelope output for the follower. 

## Freeverb

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Freeverb.png" width ="96">

>Freeverb is a reverb module based on the popular public domain Freeverb algorithm that has been around since the year 2000. 

### Controls 
* **Size:** The size of the room. 
* **Feedback:** The amount the reverberated signal is fed back into itself. (Interrelated with overall decay time).
* **Damp:** The cutoff filter and decay time for the fedback signal. (Interrelated with overall decay time).
* **Mix:** Dry/Wet for reverb mix.  
* **Input:** Audio input.
* **Size CV In:** CV input for size, -5v/+5v.
* **Feedback CV In:** CV input for feedback, -5v/+5v.
* **Damp CV In:** CV input for damp, -5v/+5v.
* **Mix CV In:** CV input for mix, -5v/+5v. 
* **Out:** Audio output. 

## Gate 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Gate.png" width ="48">

>Gate is a modifier for gate signals. It can extend the pulse width of a gate and also apply delay to the signal. 

### Controls 
* **Length:** Adjusts on time of gate from 1ms to 1sec.
* **Delay:** Delays on time of gate from 0ms to 1sec.
* **Length CV In:** Length CV input, -5v/+5v.
* **Delay CV In:** Delay CV input, -5v/+5v.
* **Input:** Gate input.
* **Out:** Gate output. 

## HPF

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/HPF.png" width ="48">

>HPF is a 12db/Oct resonant highpass filter with two modes, "standard" and "Korg" which mimicks the Korg sallen-key style highpass of the MS20. 

### Controls 
* **Cutoff:** Filter cutoff, 130hz - 2093hz.
* **Q:** Filter resonance, 1x-20x. 
* **Mode:** Highpass filter style, Standard/Korg. 
* **CV Input:** CV input for filter cutoff, -5v/+5v.
* **Input:** Audio input.
* **Out:** Audio output. 

## KPLS

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/KPLS.png" width ="48">

>KPLS is a drum module based on the Karplus Strong synthesis model, a simple method of pinging a filtered delay line to mimick string sounds.

### Controls 
* **Pitch:** Pitch of the sound, 40hz - 200hz. 
* **Decay:** Amplitude decay time, 200ms - 1sec.
* **Spread:** Detune amount for multiple instances of the delay line. This creates a chorusing effect. 
* **V/Oct CV In:** 1v/Oct input for the pitch. 
* **Trig In:** Trigger in.
* **Out:** Audio output. 

## LPG

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/LPG.png" width ="96">

>LPG is a lowpass gate. A lowpass gate is a combination of a 6db/Oct non resonant filter and a VCA. You can "ping" the module with a trigger for a subtle drum sound. 

### Controls 
* **Level:** Drive amount for the LPG. 
* **Color:** Filter cutoff
* **Decay:** Decay time for the amplitude envelope applied when "ping" is triggered. 
* **Input:** Audio input.
* **Ping Gate In:** A gate or trigger input to "ping" the envelope. This input doesn't respond to pulsewidth changes. 
* **Level CV In:** CV input for level -5/+5v.
* **Color CV In:** CV input for color -5v/+5v.
* **Decay CV In:** CV input for decay -5v/+5v. 
* **Out:** Audio output. 

## MNMX

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/MNMX.png" width ="48">

>MNMX is a rectifier module that splits one or two voltages into their positive and negative counterparts. 

### Controls 
* **In A:** CV or audio input A.
* **In B:** CV or audio input B.
* **Min Out:** The positive output of one or both voltages.
* **Max Out:** The negative output of one or both voltages.

## Multi LFO 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Multi%20LFO.png" width ="96">

>Multi LFO is an LFO with four waveshape outputs, variable pulse width, and variable phase. 

### Controls 
* **Rate:** The speed of the LFO (dependent on range switch). Slow: 0.0003hz - 0.67hz. Fast: 0.01hz - 20hz.
* **Phase:** A skew control for the symmetry of the waveform. 
* **PW:** The pulsewidth of the period of the pulse output (on time vs. off time). 
* **Reset Gate In:** A gate input that resets the phase of the LFO to zero when applied.
* **PW CV In:** CV input for PW, -5v/+5v.
* **Rate CV In:** CV input for rate, -5v/+5v.
* **Phase CV In:** CV input for phase, -5/+5v.
* **Inv Saw Out:** Inverted Sawtooth waveshape output.
* **Pulse Out:** Pulse waveshape output.
* **Saw Out:** Sawtooth waveshape output.
* **Sine Out:** Sine waveshape output. 

## NSE 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/NSE.png" width ="48">

>NSE is a noise generator with two outputs: white and pink. White noise is all frequencies of the audio spectrum at random amplitudes. Pink noise is all frequencies of the audio spectrum at the same amplitude. 

### Controls 
* **White:** White noise output.
* **Pink:** Pink noise output. 

## OCT 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/OCT.png" width ="48">

>OCT is an octave transpose tool for 1v/Oct pitch information. Incoming notes can be shifted to different octaves. 

### Controls 
* **Octave:** Octave tranpose from -4 (four octaves below) to +4 (four octaves above) the current pitch.
* **CV In:** CV input for octave parameter, -5v/+5v.
* **Input:** Note information input.
* **Out:** Transposed note information output. 

## Pan

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Pan.png" width ="48">

>Pan is a simple utility for panning signals around the stereo field. 

### Controls 
* **Pan:** Move the sound from left to right in the stereo field. 
* **CV In:** CV input for pan, -5v/+5v. 
* **Input:** Audio input.
* **Out 1:** Left audio output.
* **Out 2:** Right audio output. 

## Pitch Shift

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Pitch%20Shift.png" width ="84">

>Pitch shift is a buffer based pitch shifter that takes audio and changes the pitch of it. 

### Controls 
* **Coarse:** semitone shift, -12 to +12 semitones.
* **Fine:** fine tune adjustment for pitch shift, -1 to 1 semitone.
* **Window:** buffersize, 0.42ms - 200ms.
* **Mix:** dry/wet crossfader for effect.
* **Input:** audio input.
* **Pitch CV In:** CV input for pitch, -5/+5v.
* **Window CV In:** CV input for window, -5/+5v.
* **Mix CV In:** CV input for mix, -5/+5.
* **Out:** audio output.

## Prob 8

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Prob%208.png" width ="96">

>Prob 8 is an 8 step CV sequencer with probability per step. 

### Controls 
* **Knobs 1-8:** probability (the likelihood the step will occur), 0-100%.
* **Clock Gate In:** the main clock input for the module. Can accept gates or triggers. 
* **Reset Gate In:** reset input, sets the sequence to step 0 when high. 
* **Inv Out:** inverted version of the sequence output. 
* **Out:** CV output for the sequencer; a sum of all 8 steps. 

## S&H

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/S%26H.png" width ="48">

>S&H is a simple dual sample and hold module, where control voltages are sampled and held when processed.

### Controls 
* **Sam:** channel 1 source input to be sampled and held.
* **In 1:** channel 1 clock gate input to initiate the sample and holding effect. 
* **Samp:** channel 2 source input to be sampled and held.
* **In 2:** channel 2 clock gate input to initiate the sample and holding effect. 
* **Out 1:** channel 1 output.
* **Out 2:** channel 2 output. 

## SEQ 8

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/SEQ8.png" width ="96">

>SEQ8 is a basic 8 step sequencer. 

### Controls 
* **Knobs 1-8:** 0-10v offset.
* **Clock Gate In:** the main clock input for the module. Can accept gates or triggers.
* **Reset Gate In:** reset input, sets the sequence to step 0 when high.
* **Gate Out:** fires a step on step 1 of the sequence.
* **Out:** CV output for the sequencer; a sum of all 8 steps. 

## SLW

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Slew.png" width ="48">

>SLW is a simple attack/release slew generator for processing incoming CV or audio.

### Controls 
* **Rise:** the rise time of the slew limiter, 1ms - 2sec.
* **Fall:** the fall time of the slew limiter, 1ms - 2sec.
* **Input:** CV or audio input.
* **Out:** slewed output. 

## SRC

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Source.png" width ="48">

>SRC is a dual bipolar offset generator.

### Controls 
* **1:** offset 1, -10v to 10v.
* **2:** offset 2, -10v to 10v.
* **1 Out:** offset 1 output.
* **2 Out:** offset 2 output. 

## Stereo Mixer 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Stereo%20Mixer.png" width ="216">

>Stereo Mixer is a four channel stereo audio mixer with pan per channel.

### Controls 
* **Level 1-4:** volume for each associated channel.
* **Pan 1-4:** pan for each associated channel, shifting the signal left or right in the stereo field.
* **In L 1-4:** audio left input per channel.
* **In R 1-4:** audio right input per channel.
* **L Out:** mixed output sum left channel.
* **R Out:** mixed output sum right channel. 

## Switch 1:4

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Switch%201-4.png" width ="96">

>Switch 1:4 is a sequential switch module that takes in a signal and routes it to 1 of 4 destinations.

### Controls 
* **Input:** the signal to be routed.
* **Reset Gate In:** gate input that resets the sequential switch to step 1.
* **Clock Gate In:** the main clock input that advances the switch to the next step.
* **Cv In:** CV input to crossfade between the 4 outputs instead of stepping, 0-5v.
* **Out 1-4:** the four outputs of the switch. 

## Switch 4:1

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Switch%204-1.png" width ="96">

>Switch 4:1 is a sequential switch module that takes 4 signals and routes them to a single destination.

### Controls 
* **In 1-4**: the signals to be routed.
* **Clock In:** the main clock input that advances the switch to the next step.
* **CV In:** CV input to crossfade between the 4 inputs instead of stepping, 0-5v.
* **Reset In:** gate input that resets the sequential switch to step 1.
* **Out:** the output of the switch. 

## Verb 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Verb.png" width ="120">

>A simple reverb module. 

### Controls 
* **Size:** the size of the "room."
* **Time:** decay time of the reverb.
* **Damp:** the amount of high frequencies that decay in the reverb.
* **AP Ratio:** diffusion of the reverb in the space, making the tail feel more natural. 
* **Comb:** changes the pre-delay of the reverb algorithm, making the distance of the "walls" greater.
* **Mix:** dry/wet control for reverb amount.
* **Input:** audio input.
* **Size CV In:** CV input for size, -5v/+5v.
* **Time CV In:** CV input for time, -5v/+5v.
* **Damp CV In:** CV input for damp, -5v/+5v.
* **Ratio CV In:** CV input for ratio, -5v/+5v.
* **Comb CV In:** CV input for comb, -5v/+5v.
* **Mix CV In:** CV input for mix, -5v/+5v.
* **Out:**  Audio output. 