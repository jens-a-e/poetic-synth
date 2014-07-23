
/*  Example playing different wave in MIDI with control parameter,
 *  using Mozzi sonification library.
 *
 *
 *  Circuit: Audio output on digital pin 9 (on a Uno or similar), or 
 *  check the README or http://sensorium.github.com/Mozzi/
 *
 *  e-licktronic forum
 *  http://www.e-licktronic.com/forum
 *
 * 
 *  This example code is in the public domain.
 */

#include <MIDI.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <mozzi_midi.h>
#include <ADSR.h>
#include <Line.h>
#include <LowPassFilter.h>
#include <tables/saw256_int8.h> // saw table for oscillator
#include <tables/sin256_int8.h> // sin table for oscillator
#include <tables/triangle_analogue512_int8.h> // tri table for oscillator
#include <tables/square_analogue512_int8.h> // square table for oscillator
#include <tables/square_no_alias512_int8.h> // square table for oscillator

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 256 // powers of 2 please


// use: Oscil <table_size, update_rate> oscilName (wavetable)
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> aSaw(SAW256_DATA);
Oscil <SIN256_NUM_CELLS, AUDIO_RATE> aSin(SIN256_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE_ANALOGUE512_DATA);
Oscil <SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_ANALOGUE512_DATA);
Oscil <SIN256_NUM_CELLS, AUDIO_RATE> aLfo_sin(SIN256_DATA);
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> aLfo_saw(SAW256_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aLfo_tri(TRIANGLE_ANALOGUE512_DATA);
Oscil <SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aLfo_squ(SQUARE_ANALOGUE512_DATA);

//Line <unsigned int> aGain_saw;
//Line <unsigned int> aGain_squ;
ADSR <CONTROL_RATE,AUDIO_RATE> env;
LowPassFilter lpf;

int f;
byte wave_form;
unsigned int attack;
unsigned int releas;
unsigned int cutoff;
unsigned int resonance;
unsigned int gain_lfo;
int speed_lfo;
float gain;
float oct;
int value_pot0;
int value_pot1;
int value_pot5;
int value_pot4;
#define LED 13 // 6 on Teensy++ 2.0, 11 on Teensy 2.0, to see if MIDI is being recieved



void setup(){
  
    pinMode(LED, OUTPUT);


  MIDI.begin(MIDI_CHANNEL_OMNI); 
  
  MIDI.setHandleNoteOn(MyHandleNoteOn);
  MIDI.setHandleNoteOff(MyHandleNoteOff);
  
  
  env.setADLevels(200,100);
    env.setTimes(50,200,10000,200); // 10000 is so the note will sustain 10 seconds unless a noteOff comes
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)


}


void updateControl(){
  MIDI.read();
  
  // the probrelamtic values for _wave shapes_ ... hard code to see, if it works
  value_pot0= 0; //mozziAnalogRead(A0); // A0 used elesewhere now!!
  // the probrelamtic values for _octaves_ ... hard code to see, if it works
  value_pot1= 0; //mozziAnalogRead(A1); // A1 used elesewhere now!!
  
  
  value_pot4=mozziAnalogRead(A4);
  value_pot5=mozziAnalogRead(A5);
  //Waveform choice with potentiometer 0
  if(value_pot0<= 256) wave_form =1;
  else if (value_pot0<=512 && value_pot0>256) wave_form=2;
  else if (value_pot0<=768 && value_pot0>512) wave_form=3;
  else if (value_pot0>768) wave_form=4;

  if(value_pot1<= 256) oct =4;
  else if (value_pot1<=512 && value_pot1>256) oct=2;
  else if (value_pot1<=768 && value_pot1>512) oct=1;
  else if (value_pot1>768) oct=0.5;
  
  //Map all pot value to match with parameter
  speed_lfo = map(value_pot4,0,1024,30,2000);
  
  attack = map(mozziAnalogRead(A0),0,1024,20,2000);
  releas = map(mozziAnalogRead(A1),0,1024,40,3000);
  
  cutoff = map(mozziAnalogRead(A2),0,1024,20,255);
  
  if (wave_form==2) resonance=map(mozziAnalogRead(A3),0,1024,0,120);
  else resonance=map(mozziAnalogRead(A3),0,1024,0,170);
  
  aLfo_squ.setFreq(speed_lfo);//update Lfo frequency
  aLfo_sin.setFreq(speed_lfo);//update Lfo frequency
  aLfo_saw.setFreq(speed_lfo);//update Lfo frequency
  aLfo_tri.setFreq(speed_lfo);//update Lfo frequency
  if(value_pot5<= 256) gain_lfo= (128u+aLfo_squ.next());//Set Lfo to 0 to 255 instead of -127 to 127
  else if (value_pot5<=512 && value_pot5>256) gain_lfo= (128u+aLfo_sin.next());
  else if (value_pot5<=768 && value_pot5>512) gain_lfo= (128u+aLfo_saw.next());
  else if (value_pot5>768) gain_lfo= (128u+aLfo_tri.next());
  
  
  
  env.setAttackTime(attack); //Set attack time
  env.setReleaseTime(releas);//Set release time
  env.update();
  lpf.setResonance(resonance);//Set resonance
  if (value_pot4 <5) lpf.setCutoffFreq(cutoff);//If LFO potentiometer is null don't send any modulation
  else lpf.setCutoffFreq((cutoff*gain_lfo)>>8);

}


int updateAudio(){
  switch (wave_form){
  case 1:
    return (int)env.next() * (lpf.next(aSaw.next()))>>8; // return an int signal centred around 0
    break;
  case 2:
    return (int)env.next() * (lpf.next(aSin.next()))>>8;
    break;
  case 4:
    return (int)env.next() * (lpf.next(aTri.next()))>>8;
    break;
  case 3:
    return (int)env.next() * (lpf.next(aSqu.next()))>>8;
    break;
  }
}


void loop(){
  audioHook(); // required here
}

void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {
  if (velocity > 0) {
    f = mtof(pitch);
    aSaw.setFreq(f);
    aSin.setFreq(f);
    aTri.setFreq(f);
    aSqu.setFreq(f);

    env.noteOn();
    digitalWrite(LED,HIGH);
  } 
}

void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
  env.noteOff();
  digitalWrite(LED,LOW);
}










