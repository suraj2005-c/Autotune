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

  // On copie les 128 nouveaux échantillons du micro dans notre grand buffer
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    audioBuffer[writeIdx] = inBlock->data[i]; 
    writeIdx = (writeIdx + 1) % 1024; 
  }
  release(inBlock);

  // 3. Préparer le bloc de sortie
  audio_block_t* outBlock = allocate();
  if (outBlock) {
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      int base = (int)readIndex;
      int next = (base + 1) % 1024;
      float fraction = readIndex - base;

      // Calcul de l'échantillon 
      float sample = (audioBuffer[base] * (1.0 - fraction)) + (audioBuffer[next] * fraction);
      
      outBlock->data[i] = (int16_t)sample;

      readIndex += ratio;
      if (readIndex >= 1024) readIndex -= 1024;
    }
    transmit(outBlock, 0);
    release(outBlock);
  }
}
