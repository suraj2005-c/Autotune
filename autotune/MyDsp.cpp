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
void MyDsp::setGain(float g){
  this->gain = g;   // On met à jour la variable utilisée dans update()
  fm.setGain(g);    // On garde ça si 'fm' en a aussi besoin
}

void MyDsp::update(void) {
  audio_block_t *inBlock = receiveReadOnly(0);
  audio_block_t* outBlock = allocate();
  if (!inBlock) return;

  // 1. Stockage des données entrantes
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    if (ecritureA){
      audioBuffer[writeIdx] = inBlock->data[i]; 
    }
    else
      audioBuffer2[writeIdx] = inBlock->data[i]; 

    int base = (int)readIndex;
    int next = (base + 1) % 1024;
    float fraction = readIndex - (float)base;

 // INTERPOLATION

     if (ecritureA) 
      sample = (audioBuffer2[base] * (1.0f - fraction)) + (audioBuffer2[next] * fraction);
    else
      sample = (audioBuffer[base] * (1.0f - fraction)) + (audioBuffer[next] * fraction);


    sample=sample*gain;

    outBlock->data[i] = (int16_t)sample;

    writeIdx++; 

      // MISE À JOUR DE L'INDEX
      readIndex += ratio;
     // On boucle l'index
      
      if ( writeIdx >=1024 ) {
        readIndex=0;
        writeIdx=0;
        if (ecritureA==true)
          ecritureA=false;
        else
          ecritureA=true;
      }
        
  }
  release(inBlock);
  transmit(outBlock, 0);
  release(outBlock);
}
