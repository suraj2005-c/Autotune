#include <Audio.h>
#include <Wire.h>
#include "MyDsp.h"

// --- OBJETS AUDIO ---
AudioInputI2S            micInput;
AudioAnalyzeNoteFrequency notefreq;
AudioOutputI2S           headphones;
AudioControlSGTL5000     audioShield;
MyDsp monAutoTune;

// --- CONNEXIONS ---
AudioConnection patch1(micInput, 0, notefreq, 0);
AudioConnection patch2(micInput, 0, monAutoTune, 0); 
AudioConnection patch3(monAutoTune, 0, headphones, 0);
AudioConnection patch4(monAutoTune, 0, headphones, 1);

// --- GAMME ---
float noteRef[] = {
  49.00, 55.00, 61.74,
  65.41, 73.42, 82.41, 87.31, 98.00, 110.00, 123.47,
  130.81, 146.83, 164.81, 174.61, 196.00, 220.00, 246.94,
  261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25
};
const int scaleSize = 25;

float ratio = 1.0;
float val_pot, valPot2;

float findClosestNote(float inputFreq);

void setup() {
  Serial.begin(115200);
  AudioMemory(160); 
  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.volume(0.8);
  audioShield.unmuteHeadphone();
  audioShield.micGain(25);
  notefreq.begin(0.30);
  Serial.println("Système Auto-Tune prêt brrrrrr ");
}

void loop() {
  val_pot = analogRead(19) / 1024.0;
  valPot2 = analogRead(22) / 1024.0;

  // Gain minimum à 0.3 pour éviter silence total si pot à 0
  float gainValue = map(val_pot, 0.0, 1.0, 0.3, 1.0);
  monAutoTune.setGain(gainValue);

  if (notefreq.available()) {
    float freq = notefreq.read();
    if (notefreq.probability() > 0.6) {
      float freqCor = findClosestNote(freq);
      ratio = constrain(freqCor / freq, 0.5f, 2.0f);
      monAutoTune.setRatio(ratio);
      Serial.print("Entrée: "); Serial.print(freq);
      Serial.print(" Hz | Cible: "); Serial.print(freqCor);
      Serial.print(" Hz | Ratio: "); Serial.println(ratio);
      Serial.print("Gain: "); Serial.println(gainValue);
    }
  }
}

float findClosestNote(float inputFreq) {
  float closest = noteRef[0];
  float minDiff = fabsf(inputFreq - noteRef[0]);
  for (int i = 1; i < scaleSize; i++) {
    float diff = fabsf(inputFreq - noteRef[i]);
    if (diff < minDiff) {
      minDiff = diff;
      closest = noteRef[i];
    }
  }
  return closest;
}
