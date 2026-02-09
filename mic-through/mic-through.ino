#include <Audio.h>

AudioInputI2S in;
AudioOutputI2S out;
AudioControlSGTL5000 audioShield;
AudioConnection patchCord0(in,0,out,0);
AudioConnection patchCord1(in,0,out,1);
float pitch_shift = 1.3;
int buffer_size = 200;
uint16_t sample_rate=44100;

void setup() {
  Serial.begin(9600);
  AudioMemory(6);
  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.micGain(5); // in dB
  audioShield.volume(1);
}

int tab[128];
int i;

void loop() {

   for(i=0;i<128;i++){
    if tab[i]=
  }
 
  delay(100);
}
