#include <Audio.h>
#include <Wire.h>

// Objets Audio
AudioInputI2S            micInput;       
AudioOutputI2S           headphones;      
AudioAnalyzeNoteFrequency notefreq;
AudioControlSGTL5000     audioShield;

// Connexions globales (indispensable)
AudioConnection          patch0(micInput, 0, headphones, 0);
AudioConnection          patch1(micInput, 0, headphones, 1);
AudioConnection          patch2(micInput, 0, notefreq, 0);

float noteRef[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
const int scaleSize = 8;

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  AudioMemory(120); 

  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.micGain(45);
  audioShield.volume(0.7);

  notefreq.begin(0.30); // Seuil plus tolérant
  Serial.println("Système démarré...");
}

void loop() {
  if (notefreq.available()) {
    float freq = notefreq.read();
    float prob = notefreq.probability();
    

    if (prob > 0.6) { 
      float freqCor = findClosestNote(freq);
      float ratio = freq/freCor;
      Serial.print("Détecté: ");
      Serial.print(freq);
      Serial.print(" Hz | Corrigé: ");
      Serial.print(freqCor);
      Serial.println(" Hz");
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
