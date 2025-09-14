# 4ms VCV Rack Core Modules User Guide 

## Atvert

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Atvert.png" width ="48">

Atvert is a 2-channel attenuverter that lets you scale or invert two input
signals using two knobs. If no input is connected to a channel, it uses a
+5V offset for the input (the resulting output will be -5V to +5V).

### Controls

* **2 knobs (one per channel):**
Adjust from -100% (inverted) to +100% (normal). Center = 0% (no output)

## BPF

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/BPF.png" width ="48">

BPF is a 12db per octave resonant bandpass filter with two filter types:
"Standard" which is based on a Korg style sallen-key topology, and "Oberheim"
which is based on the filter in the SEM.

### Controls 
* **Cutoff:** filter cutoff frequency, 261hz - 1041hz
* **Q:** filter resonance
* **Mode:** toggle between Korg (left) and Oberheim (right) mode
* **CV:** CV Input for filter cutoff, -5v/+5v.
* **Input:** Audio input 
* **Out:** Audio output 

## Basic Wav Player

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/BWAVP.png" width ="80">

Basic Wav Player is a simple module designed to play .wav files. It
streams the .wav file, which means it can play large files without needing to
wait to load the entire file. It also consumes less memory than a non-streaming
player since only a small portion of the file is loaded at a time.

### Controls 

* **Play:** Button to start playback. When the sample is already playing,
  pressing it can stop, restart, or pause playback (selectable by the Play
  Retrig Mode setting)
* **Loop:** Button to toggle looping mode. In looping mode, the sample will
  play back from the beginning after reaching the end.

### Jacks

* **Play Jack:**  A trigger or rising edge of a gate on this jack has the same effect as pressing the Play button.
* **Loop Jack:**  A trigger or rising edge of a gate on this jack toggles looping mode.
* **Play Gate:** Gate output which goes high when the sample is playing, and low
  when it's stopped or paused. When re-triggering, the gate will go low
  briefly. The Play Gate will not go low when a sample loops.
* **End:** Trigger output which fires when the sample ends or stops. An End
  trigger will fire when the sample loops, when re-triggered, or playback
  stops. No trigger fires when the sample is paused.
* **Out Left:** Left channel of sample
* **Out Right:** Right channel of sample (or left channel if sample is mono)

### Settings

* **Play Retrig Mode:** Behavior when a play trigger or button press happens
  while the sample is playing:
    - Retrigger: The sample starts over at the beginning. An End trigger
       fires and the Play Gate dips low briefly before going high again.

    - Stop: The sample stops playing and is reset to the beginning. An End
       trigger fires and Play Gate goes low.

    - Pause: The sample pauses (stops playback). Pressing play again will
       resume from the current position. No End trigger fires, and Play Gate
       goes low when paused.

- **Waveform Zoom:** How much of the sample's time to display on the screen.
  A Zoom setting of 0% displays the last 2ms on the screen, and a Zoom setting
  of 100% displays the last ~750ms.
- **Playback Buffer Threshold:** This is an advanced option that controls how
  much of the buffer should be filled before playback is allowed to begin.
  Setting this lower will result in less latency from the time a trigger fires
  to the time the sample starts. Setting this higher will mean that if the SD
  Card or USB drive stalls momentarily, then there's less chance of the audio
  glitching. Keep in mind that if the sample data can fit in the buffer, then
  this setting has no effect once the sample is full buffered (progress bar is
  green). Also, since this is expressed as a percentage of the buffer size,
  changing the buffer size will effect the latency and stall protection. The
  default value of 25% is a good choice if you don't know what to pick.
- **Max Buffer Size:** This sets the maximum amount of memory (in MB) the
  module is allowed use. If the sample is smaller than the buffer, then only
  the memory needed will be used. In that case, the sample can be "fully buffered",
  which means once it's been played once it can be played again without any
  additional disk access or latency. 
  On the other hand, if the sample data is larger than the max buffer size,
  then the sample cannot be fully loaded into memory. In this case it will 
  always be streamed from disk. Setting the Max Buffer Size can be helpful
  in patches with lots of sample player modules.
- **Buffer Strategy**: This is an advanced option. When set to Fill To
  Threshold (default), the sample data will stop being loaded from disk when
  the buffer is filled up to the threshold (See previous option). As the sample
  is played, more data will be read from disk. When set to Fill Completely, the
  sample data will keep being read until the buffer is full. When this option
  is selected, then the Playback Buffer Threshold option only sets the minimum
  amount of sample data required in the buffer before audio can be played.
- **Startup Delay:** This sets an amount of time in seconds that the module
  will not access disk after the patch is initially loaded. This is useful if
  you have a lot of sample playback modules in a patch and want certain ones to
  load from disk first after the patch is loaded.
- **Load Sample:** This brings up a file choose dialog box to let you pick a
  .wav file. Stereo or mono files are acceptable. The audio will be resampled
  to the current sample rate.

### Lights

* **Disk:** Red light is one when the disk is being accessed
* **Progress bar:** The bar below the sample waveform tells you about the status of playback and buffering:
   - Grey: Sample is being read from disk, not ready to play (buffer has not been filled to the Playback Threshold yet)
   - Blue: Ready to play (buffer is filled past the Playback Threshold)
   - Green: Sample is fully loaded into the buffer
* **Waveform color:**
   - Teal: Normal playback
   - Blue: Startup delay is in effect
   - Red: File or disk error
- **Play Button light:**
   - Off: not playing
   - Green: playing
   - Red: File or disk error
   - Yellow: Buffer underrun

## CLKD

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/CLKD.png" width ="48">

CLKD is a simple clock divider that takes in incoming gates and divides their
timing by a given amount. 

### Controls 

* **Divide:** from "=" (no division) to /16, stepped
* **CV:** CV input for clock division, -5v/+5v 
* **Clk In:** Clock input
* **Clock:** Clock output 

## CLKM

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/CLKM.png" width ="48">

CLKM is a simple clock multiplier that takes in incoming gates and multiplies their timing by a given amount. 

### Controls 
* **Multiply:** from "x1" (no multiplication) to "x16", stepped
* **CV:** CV input for clock multiplication, -5v/+5v 
* **Clk In:** Clock input
* **Clock:** Clock output 

## Complex EG

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Complex%20EG.png" width ="168">

Complex EG is an ADSHR envelope generator with control over curve shape for
attack, decay, and release time. This envelope is particularly useful with a
keyboard, where long note holds can control complex changes of a sound over
time.

### Controls 
* **Attack:** The attack phase of the envelope (rise time), 1ms - 1sec
* **Decay:** The decay phase of the envelope (amount of time from attack to release stage), 1ms - 1sec
* **Release:** The release phase of the envelope (amount of time after the sustain stage for signal to fall to 0v), 1ms - 1 sec 
* **A Curve:** Change the time scaling of the attack curve continously from exponential (0%) to linear (50%) to logarithmic (100%)
* **D Curve:** Change the time scaling of the decay curve continously from exponential (0%) to linear (50%) to logarithmic (100%)
* **R Curve:** Change the time scaling of the release curve continously from exponential (0%) to linear (50%) to logarithmic (100%)
* **Sustain:** The sustain phase (volume at which sound is held after decay stage), 0-8v 
* **Loop:** Toggle for engaging looping behavior of the envelope. If off, envelope needs external gate  
* **Hold:** The hold phase of the envelope (amount of time after the incoming gate goes off before the release phase begins), 1ms-1sec 
* **Gate In:** Gate input for beginning the envelope generator 
* **Attack CV In:** CV input for attack time, -5v/+5v
* **Decay CV In:** CV input for decay time, -5v/+5v
* **Release CV In:** CV input for release time, -5v/+5v
* **Sustain CV In:** CV input for sustain volume, -5v/+5v
* **Hold CV In:** CV input for hold time, -5v/+5v
* **Attack Out:** an 8v logic output for when Attack stage is active
* **Decay Out:** an 8v logic output for when Decay stage is active
* **Release Out:** an 8v logic output for when Release stage is active
* **Sustain Out:** an 8v logic output for when Sustain stage is active
* **Hold Out:** an 8v logic output for when Hold stage is active
* **Envelope Out:** Full Envelope output 

## Detune 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Detune.png" width ="96">

Detune is a "Wow and flutter" tape simulator in which the incoming signal is
slowly modulated by a pseudo random pitch shifter. The purpose of this effect
is to mimic the behavior of a poorly calibrated tape machine. 

### Controls 
* **W Speed:** "Wow" speed (slower pitch modulation), 0.1hz - 5.33hz
* **F Speed:** "Flutter" speed (faster pitch modulation), 5hz - 30hz
* **W Depth:** "Wow" effect amount 
* **F Depth:** "Flutter" effect amount
* **Input:** Audio input 
* **Detune CV In:** CV input for both W and F speeds, -5v/+5v 
* **Depth CV In:** CV input for both W and F depths, -5v/+5v
* **Output:** Audio output

## Djembe

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Djembe.png" width ="96">

Djembe is a drum module based on the sonic characteristic of a real acoustic Djembe. It uses a simple exciter with resonant filters. 

### Controls 
* **Pitch:** The overall pitch of the drum 20hz - 500hz
* **Sharpness:** The overall volume of the drum
* **Hit:** A lowpass filter for the exciter portion of the sound
* **Strike Amount:** Exciter volume 
* **Pitch CV In:** Pitch CV input -5v/+5v
* **Sharp CV In:** Sharp CV input -5v/+5v
* **Hit CV In:** Hit CV input -5v/+5v
* **Strike CV In:** Strike CV input -5v/+5v
* **Trigger In:** Trigger input
* **Out:** Audio output

## Drum 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Drum.png" width ="156">

Drum is a drum module based on a simple two operator FM oscillator core and a noise source. 

### Controls 
* **Pitch:** The base pitch of the carrier and modulator. 10hz - 1000hz
* **Pitch Env:** A decay envelope that controls the pitch of the carrier 
* **Pitch Amt:** The amount the decay envelope modulates the pitch of the carrier
* **Ratio:** The frequency of the modulating oscillator, with respect to the frequency of the carrier oscillator
* **FM Env:** The duration of envelope that controls the amount of frequency modulation
* **FM Amt:** The overall amount of FM present in the sound (amplitude of the FM envelope)
* **Tone Env:** The decay time for the FM portion of the drum
* **Noise Env:** The decay time for the noise portion of the drum
* **Noise Blend:** A crossfader that fades between the FM and noise portions
* **Trigger In:** Trigger input for all decay envelopes
* **V/Oct CV In:** 1v/oct pitch input for carrier and modulator
* **P Env CV In:** CV input for pitch envelope -5v/+5v
* **P Amt CV In:** CV input for pitch enveliope depth -5v/+5v
* **Ratio CV In:** CV input for pitch ratio of modulator -5v/+5v 
* **FM Env CV In:** CV input for FM decay envelope -5v/+5v 
* **FM Amt CV In:** CV input for FM amount -5v/+5v
* **T Env CV In:** CV input for tone envelope -5v/+5v
* **N Env CV In:** CV input for noise envelope -5v/+5v 
* **N Blend CV In:** CV input for noise blend -5v/+5v
* **Inv Out:** CV output for inverted version of the tone envelope
* **Out:** Drum audio output

## FM

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/FM.png" width ="96">

FM is a two operator FM oscillator with variable waveshape. 

### Controls 
* **Pitch:** Base frequency of carrier and modulator, 20hz - 20khz
* **Mix:** A crossfader that blends between carrier and modulated carrier 
* **Index:** Depth of frequency FM
* **Index CV:** An attenuverter for the Index parameter + CV 
* **Ratio C:** Coarse frequency adjustment for modulator pitch ratio 
* **Ratio F:** Fine frequency adjustment for modulator pitch ratio 
* **Shape:** Blend continously from sine to square
* **Shape CV:** An attenuverter for shape parameter + CV 
* **V/Oct C CV In:** 1v/Oct pitch input for carrier
* **V/Oct M CV In:** 1v/Oct pitch input for modulator
* **Mix CV In:** CV input for mix, -5v/+5v 
* **Index CV In:** CV input for index, -5v/+5v
* **Shape CV In:** CV input for shape, -5v/+5v
* **Out:** Audio output  

## FLW 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/FLW.png" width ="48">

FLW is an envelope follower that generates an exponential/logarithmic envelope based on the amplitude of an audio or CV signal. Additionally, there is comparator output.

### Controls 
* **Rise:** Rise time for the envelope, 1ms - 2sec. The rise is non-linear: it rises quickly at first, then slows.
* **Fall:** Fall time for the envelope, 1ms - 2sec. The fall is non-linear: it falls quickly at first, then slows.
* **Thresh:** The voltage threshold at which a gate is output. When the envelope signal is higher than this voltage, the gate will be high; when it's lower, the gate will be low.
* **Input:** CV or audio input
* **Gate Out:** A gate output that's high when the envelope is above the threshold.
* **Env Out:** Envelope output for the follower.

## Freeverb

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Freeverb.png" width ="96">

Freeverb is a light-weight reverb effect based on the popular public domain Freeverb algorithm. It uses a collection of all-pass and comb filters.

### Controls 
* **Size:** The size of the room (length of delays)
* **Feedback:** The amount the reverberated signal is fed back into itself
* **Damp:** Low-pass filtering for the reverberated signal
* **Mix:** Dry/Wet for reverb mix  
* **Input:** Audio input
* **Size CV In:** CV input for size, -5v/+5v
* **Feedback CV In:** CV input for feedback, -5v/+5v
* **Damp CV In:** CV input for damp, -5v/+5v
* **Mix CV In:** CV input for mix, -5v/+5v 
* **Out:** Audio output

## Gate 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Gate.png" width ="48">

Gate is a modifier for gate signals. It has one-shot pulse generator with a variable pulse width an delay. 

### Controls 
* **Length:** Adjusts on time of gate, 1ms - 1sec
* **Delay:** Delay time between input gate and output gate, 0ms - 1sec
* **Length CV In:** Length CV input, -5v/+5v
* **Delay CV In:** Delay CV input, -5v/+5v
* **Input:** Gate input
* **Out:** Gate output 

## HPF

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/HPF.png" width ="48">

HPF is a 12db/Oct resonant highpass filter with two modes, "standard" and "Korg" which mimics the Korg sallen-key style highpass of the MS20. 

### Controls 
* **Cutoff:** Filter cutoff, 130hz - 2093hz
* **Q:** Filter resonance, 1x-20x
* **Mode:** Highpass filter style, Standard (left) or Korg (right)
* **CV Input:** CV input for filter cutoff, -5v/+5v
* **Input:** Audio input
* **Out:** Audio output 

## KPLS

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/KPLS.png" width ="48">

KPLS is a percussive module based on the Karplus Strong synthesis model, a simple method of pinging a filtered delay line to mimic string sounds. There are 6 delay lines, each at a different frequency.

### Controls 
* **Pitch:** Pitch of the sound, 40hz - 200hz
* **Decay:** Amplitude decay time, 200ms - 1sec
* **Spread:** Detune amount for multiple instances of the delay line. This creates a chorusing effect
* **V/Oct CV In:** 1v/Oct input for the pitch 
* **Trig In:** Trigger in
* **Out:** Audio output

## LPG

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/LPG.png" width ="96">

LPG is a lowpass gate. A lowpass gate is a combination of a resonant low-pass filter and a VCA. You can "ping" the module with a trigger for a subtle drum sound. 

### Controls 
* **Level:** Combination VCA level and LPF cutoff. No effect when Ping is patched, and otherwise effects the overall volume.
* **Color:** Tonal characteristic of LPG
* **Decay:** Decay time for the amplitude envelope applied when Ping is triggered or when Level is changed.
* **Input:** Audio input
* **Ping Gate In:** A gate or trigger input to "ping" the envelope. The rising edge will trigger a decay envelope that effectively controls the Level parameter.
* **Level CV In:** CV input for level, -5v/+5v
* **Color CV In:** CV input for color, -5v/+5v
* **Decay CV In:** CV input for decay, -5v/+5v 
* **Out:** Audio output

## MNMX

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/MNMX.png" width ="48">

MNMX (Min/Max) outputs the minimum and maximum values of two signals.

### Controls 
* **In A:** CV or audio input A
* **In B:** CV or audio input B
* **Min Out:** The voltage level of A or B, whichever is greater
* **Max Out:** The voltage level of A or B, whichever is less

## Multi LFO 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Multi%20LFO.png" width ="96">

Multi LFO is an LFO with four waveshape outputs, variable pulse width, and variable phase. 

### Controls 
* **Rate:** The speed of the LFO (dependent on range switch). Slow: 0.0003Hz - 0.67Hz. Fast: 0.01Hz - 20Hz
* **Phase:** The phase offset for the LFO
* **PW:** The pulsewidth of the pulse output (on time vs. off time)
* **Reset Gate In:** A gate input that resets the LFO to the phase set by the Phase parameter
* **PW CV In:** CV input for PW, -5v/+5v
* **Rate CV In:** CV input for rate, -5v/+5v
* **Phase CV In:** CV input for phase, -5/+5v
* **Inv Saw Out:** Inverted Sawtooth waveshape output
* **Pulse Out:** Pulse waveshape output
* **Saw Out:** Sawtooth waveshape output
* **Sine Out:** Sine waveshape output 

## NSE 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/NSE.png" width ="48">

NSE is a noise generator with two outputs: white and pink.

### Controls 
* **White:** White noise output
* **Pink:** Pink noise output

## OCT 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/OCT.png" width ="48">

OCT is an octave transpose tool for 1V/oct pitch information. Incoming notes can be shifted up or down by an exact number of octaves.

### Controls 
* **Octave:** Octave tranpose from -4 (four octaves below) to +4 (four octaves above) the current pitch
* **CV In:** CV input for octave parameter, -5v/+5v
* **Input:** Note CV signal
* **Out:** Transposed Note CV signal

## Pan

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Pan.png" width ="48">

Pan is a simple utility for panning signals around the stereo field. 

### Controls 
* **Pan:** Move the sound from left to right in the stereo field
* **CV In:** CV input for pan, -5v/+5v
* **Input:** Audio input
* **Out 1:** Left audio output
* **Out 2:** Right audio output 

## Pitch Shift

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Pitch%20Shift.png" width ="84">

Pitch shift is a simple pitch shifter.

### Controls 
* **Coarse:** semitone shift, -12 to +12 semitones
* **Fine:** fine tune adjustment for pitch shift, -1 to 1 semitone
* **Window:** buffer size, 0.42ms - 200ms
* **Mix:** dry/wet crossfader for effect
* **Input:** audio input
* **Pitch CV In:** CV input for pitch, -5v/+5v
* **Window CV In:** CV input for window, -5v/+5v
* **Mix CV In:** CV input for mix, -5v/+5v
* **Out:** audio output

## Prob 8

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Prob%208.png" width ="96">

Prob 8 is an 8 step gate sequencer with probability per step. 

### Controls 
* **Knobs 1-8:** probability (the likelihood the step will occur), 0-100%
* **Clock Gate In:** the main clock input for the module. Can accept gates or triggers.
* **Reset Gate In:** reset input, sets the sequence to step 0 when high
* **Inv Out:** inverted version of the gate sequence output
* **Out:** gate sequence output for the module. Gates are 0-8v with a fixed 50% pulse width.

## S&H

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/S%26H.png" width ="48">

S&H is a simple dual sample and hold module.

### Controls 
* **Sam:** channel 1 source input to be sampled and held
* **In 1:** channel 1 clock gate input to initiate the sample and holding 
* **Samp:** channel 2 source input to be sampled and held
* **In 2:** channel 2 clock gate input to initiate the sample and holding
* **Out 1:** channel 1 output
* **Out 2:** channel 2 output 

## SEQ 8

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/SEQ8.png" width ="96">

SEQ8 is a basic 8 step CV sequencer. 

### Controls 
* **Knobs 1-8:** 0-10v offset
* **Clock Gate In:** the main clock input for the module. Can accept gates or triggers
* **Reset Gate In:** reset input, sets the sequence to step 0 when high
* **Gate Out:** fires a step on step 1 of the sequence
* **Out:** CV output for the active step.

## SLW

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Slew.png" width ="48">

SLW is a simple attack/release slew limiter for processing incoming CV or audio.

### Controls 
* **Rise:** the rise time of the slew limiter, 1ms - 2sec
* **Fall:** the fall time of the slew limiter, 1ms - 2sec
* **Input:** CV or audio input
* **Out:** slewed voltage output

## SRC

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Source.png" width ="48">

SRC is a dual bipolar DC offset generator.

### Controls 
* **1:** offset 1, -10v/10v
* **2:** offset 2, -10v/10v
* **1 Out:** offset 1 output
* **2 Out:** offset 2 output 

## Stereo Mixer 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Stereo%20Mixer.png" width ="216">

Stereo Mixer is a four channel stereo audio mixer with pan per channel.

### Controls 
* **Level 1-4:** volume for each associated channel
* **Pan 1-4:** pan for each associated channel, shifting the signal left or right in the stereo field
* **In L 1-4:** audio left input per channel
* **In R 1-4:** audio right input per channel
* **L Out:** mixed output sum left channel
* **R Out:** mixed output sum right channel 

## Switch 1:4

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Switch%201-4.png" width ="96">

Switch 1:4 is a sequential switch module that takes in a signal and routes it to 1 of 4 destinations.

### Controls 
* **Input:** the signal to be routed
* **Reset Gate In:** gate input that resets the sequential switch to step 1
* **Clock Gate In:** the main clock input that advances the switch to the next step
* **Cv In:** CV input to crossfade between the 4 outputs instead of stepping, 0-5v
* **Out 1-4:** the four outputs of the switch

## Switch 4:1

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Switch%204-1.png" width ="96">

Switch 4:1 is a sequential switch module that takes 4 signals and routes them to a single destination.

### Controls 
* **In 1-4**: the signals to be routed
* **Clock In:** the main clock input that advances the switch to the next step
* **CV In:** CV input to crossfade between the 4 inputs instead of stepping, 0-5v
* **Reset In:** gate input that resets the sequential switch to step 1
* **Out:** the output of the switch

## Verb 

<img src ="https://github.com/4ms/metamodule-core-modules/blob/quickstart-guide/doc/res/Verb.png" width ="120">

An experimental reverb module based on the Freeverb algorithm but with extra controls. Often noisy, harsh, and unpredictable.

### Controls 
* **Size:** the size of the "room"
* **Time:** decay time of the reverb
* **Damp:** the amount of high frequencies that decay in the reverb
* **AP Ratio:** Ratio of all-pass filter sizes
* **Comb:** Ratio of comb filter sizes
* **Mix:** dry/wet control for reverb amount
* **Input:** audio input
* **Size CV In:** CV input for size, -5v/+5v
* **Time CV In:** CV input for time, -5v/+5v
* **Damp CV In:** CV input for damp, -5v/+5v
* **Ratio CV In:** CV input for ratio, -5v/+5v
* **Comb CV In:** CV input for comb, -5v/+5v
* **Mix CV In:** CV input for mix, -5v/+5v
* **Out:**  Audio output
