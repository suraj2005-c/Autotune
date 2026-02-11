#include <Audio.h>
#include <Wire.h>

// 1. Matériel
AudioInputI2S            micInput;       
AudioOutputI2S           headphones;      
AudioRecordQueue         queue; 
AudioControlSGTL5000     audioShield;
AudioAnalyzeNoteFrequency notefreq;
// 2. Connexions (Micro -> Casque ET Micro -> Analyseur)
AudioConnection          patch0(micInput, 0, headphones, 0);
AudioConnection          patch1(micInput, 0, headphones, 1);
AudioConnection          patch2(micInput, 0, queue, 0);

// 3. Buffer de stockage pour le calcul de corrélation
int16_t storage[512]; 
int blocksRead = 0;
float noteRef[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
int scaleSize = 8;
float freqDetectee, freqCor;
void setup() {
  Serial.begin(115200);
  AudioMemory(60); 

  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.micGain(35);
  audioShield.volume(0.7);

  notefreq.begin(.15);
  queue.begin(); // On commence à remplir la file
  Serial.println("Stockage et calcul activés.");
}

void loop() {
  // --- ETAPE 1 : STOCKAGE ---
  if (queue.available() >= 1) {
    // On récupère le bloc de 128 samples du micro
    int16_t* buffer = queue.readBuffer();
    
    // On le copie dans notre tableau de stockage (storage)
    // On remplit 128 samples à chaque fois jusqu'à 512
    memcpy(&storage[blocksRead * 128], buffer, 256); 
    
    queue.freeBuffer(); // Très important pour libérer la mémoire audio
    blocksRead++;

    // --- ETAPE 2 : CALCUL DE CORRELATION ---
    // On attend d'avoir 512 samples (environ 11ms de son)
    if (blocksRead >= 4) {
      float maxCorr = 0;
      int bestLag = -1;

      // On compare le signal avec lui-même décalé (lag)
      // Lag 80 = 551Hz, Lag 450 = 98Hz
      for (int lag = 80; lag < 450; lag++) {
        float correlation = 0;
        
        for (int i = 0; i < (512 - lag); i++) {
          // Calcul de la corrélation : multiplication et somme
          correlation += (float)storage[i] * (float)storage[i + lag];
        }

        if (correlation > maxCorr) {
          maxCorr = correlation;
          bestLag = lag;
        }
      }
      
      if (notefreq.available()){
        freqDetectee = notefreq.read();
        freqCor= findClosestNote(freqDetectee);
      }
      

      // --- ETAPE 3 : AFFICHAGE ---
      if (maxCorr > 1000000) { // On vérifie qu'il y a assez de signal
        float freq = 44100.0 / bestLag;
        //Serial.print("Frequence calculee :");
        //Serial.print(freq);
        Serial.print("Frequence detect :");
        Serial.print(freqDetectee);
        Serial.print("\nFrequence rapportée=");
        Serial.print(freqCor);
        Serial.println(" Hz");
      }

      blocksRead = 0; // On vide le stockage pour recommencer
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
