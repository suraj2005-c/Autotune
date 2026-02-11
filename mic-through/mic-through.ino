#include <Audio.h>
#include <Wire.h>

// --- OBJETS AUDIO ---
AudioInputI2S            micInput;
AudioAnalyzeNoteFrequency notefreq;
AudioRecordQueue         queue;      // Capture le son pour le modifier
AudioPlayQueue           playQueue;  // Renvoie le son modifié
AudioOutputI2S           headphones;
AudioControlSGTL5000     audioShield;

// --- CONNEXIONS (MODIFIÉES) ---
// On ne branche plus micInput sur headphones en direct !
AudioConnection          patch2(micInput, 0, notefreq, 0); 
AudioConnection          patch3(micInput, 0, queue, 0);    
AudioConnection          patch4(playQueue, 0, headphones, 0);
AudioConnection          patch5(playQueue, 0, headphones, 1);

// --- TA GAMME ET TES VARIABLES ---
float noteRef[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
const int scaleSize = 8;
float ratio = 1.0;
float readIndex = 0.0;
int16_t audioBuffer[1024]; 
//float val_pot, valPot2;
void setup() {
  Serial.begin(115200);
  AudioMemory(120); 

  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.micGain(25);
  audioShield.volume(0.5);
  notefreq.begin(0.30);
  queue.begin();
  Serial.println("Système Auto-Tune prêt...");
}

void loop() {
  //val_pot=analogRead(19);
  //valPot2 = analogRead(22)/1024;

  // 1. ANALYSE (Ta logique)
  if (notefreq.available()) {
    float freq = notefreq.read();
    if (notefreq.probability() > 0.6) {
      float freqCor = findClosestNote(freq);
      // Correction de la formule : Ratio = Cible / Entrée
      ratio = freqCor / freq; 
      Serial.print("Entrée: ");
      Serial.print(freq);
      Serial.print(" Hz | Cible: ");
      Serial.print(freqCor);
      Serial.print(" Hz | Ratio: ");
      Serial.println(ratio);
      //Serial.println(val_pot);
    }
  }

  // 2. TRAITEMENT (L'effet élastique)
  if (queue.available() >= 1) {
    int16_t* bufferIn = queue.readBuffer();
    
    // On remplit le tampon circulaire
    static int writeIdx = 0;
    for(int j=0; j<128; j++) {
      audioBuffer[writeIdx] = bufferIn[j];
      writeIdx = (writeIdx + 1) % 1024;
    }
    queue.freeBuffer();

    int16_t* bufferOut = playQueue.getBuffer();
    if (bufferOut != NULL) {
      for (int i = 0; i < 128; i++) {
        int base = (int)readIndex;
        int next = (base + 1) % 1024;
        float fraction = readIndex - base;

        // Interpolation pour éviter les grésillements
        bufferOut[i] = (audioBuffer[base] * (1.0 - fraction)) + (audioBuffer[next] * fraction);

        readIndex += ratio; //*valPot2; // Application du RATIO ici
        if (readIndex >= 1024) readIndex -= 1024;
      }
      playQueue.playBuffer();
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
