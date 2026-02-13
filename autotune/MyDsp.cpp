#include "MyDsp.h"



#define MULT_16 32767

MyDsp::MyDsp() : 
AudioStream(AUDIO_INPUTS, new audio_block_t*[AUDIO_INPUTS]),



fm(AUDIO_SAMPLE_RATE_EXACT)
{
}

MyDsp::~MyDsp(){}

void MyDsp::setCFreq(float freq){
  fm.setCFreq(freq);
}

void MyDsp::setMFreq(float freq){
  fm.setMFreq(freq);
}

void MyDsp::setIndex(float freq){
  fm.setIndex(freq);
}

// set sine wave gain
void MyDsp::setGain(float gain){
  fm.setGain(gain);
}


void MyDsp::update(void) {
  audio_block_t *inBlock = receiveReadOnly(0);
  if (!inBlock) return;

  // 1. Stockage des données entrantes
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    audioBuffer[writeIdx] = inBlock->data[i]; 
    writeIdx = (writeIdx + 1) % 1024; 
  }
  release(inBlock);

  // 2. Préparer le bloc de sortie
  audio_block_t* outBlock = allocate();
  if (outBlock) {
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      // INTERPOLATION
      int base = (int)readIndex;
      int next = (base + 1) % 1024;
      float fraction = readIndex - (float)base;

      float sample = (audioBuffer[base] * (1.0f - fraction)) + (audioBuffer[next] * fraction);
      outBlock->data[i] = (int16_t)sample;

      // MISE À JOUR DE L'INDEX
      readIndex += ratio;
      if (readIndex >= 1024) readIndex -= 1024;

      // PROTECTION : Si la lecture rattrape l'écriture (trop près), 
      // on rajoute un petit saut de sécurité (Latency gap)
      float distance = writeIdx - readIndex;
      if (distance < 0) distance += 1024; 
      
      if (distance < 128) { // Si on est à moins d'un bloc de distance
          readIndex -= 256; // On recule la lecture pour laisser de la marge
          if (readIndex < 0) readIndex += 1024;
      }
    }
    transmit(outBlock, 0);
    release(outBlock);
  }
}
