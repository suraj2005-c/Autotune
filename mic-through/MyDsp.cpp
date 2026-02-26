#include "MyDsp.h"

MyDsp::MyDsp() : 
AudioStream(AUDIO_INPUTS, new audio_block_t*[AUDIO_INPUTS]),
fm(AUDIO_SAMPLE_RATE_EXACT)
{
  memset(audioBuffer, 0, sizeof(audioBuffer));
  memset(audioBuffer2, 0, sizeof(audioBuffer2));
}

MyDsp::~MyDsp(){}

void MyDsp::setCFreq(float freq){ fm.setCFreq(freq); }
void MyDsp::setMFreq(float freq){ fm.setMFreq(freq); }
void MyDsp::setIndex(float freq){ fm.setIndex(freq); }

void MyDsp::setGain(float g){
  this->gain = g;
  fm.setGain(g);
}

void MyDsp::update(void) {
  audio_block_t *inBlock = receiveReadOnly(0);
  audio_block_t *outBlock = allocate();

  if (!inBlock || !outBlock) {
    if (inBlock) release(inBlock);
    if (outBlock) release(outBlock);
    return;
  }

  // Écriture dans le buffer actif
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    int idx = (writeIdx + i) % BUF_SIZE;
    if (ecritureA)
      audioBuffer[idx] = inBlock->data[i];
    else
      audioBuffer2[idx] = inBlock->data[i];
  }

  writeIdx += AUDIO_BLOCK_SAMPLES;

  // Attendre que le premier buffer soit plein
  if (!bufferPret) {
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
      outBlock->data[i] = 0;

    if (writeIdx >= BUF_SIZE) {
      writeIdx = 0;
      ecritureA = !ecritureA;
      bufferPret = true;
      readIndex = 0.0f;
    }

    release(inBlock);
    transmit(outBlock, 0);
    release(outBlock);
    return;
  }

  // Lecture depuis le buffer inactif avec crossfade aux transitions
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    int base = (int)readIndex % BUF_SIZE;
    int next = (base + 1) % BUF_SIZE;
    float fraction = readIndex - (int)readIndex;

    // Sample du buffer inactif (lecture normale)
    float sActif;
    if (ecritureA)
      sActif = (audioBuffer2[base] * (1.0f - fraction)) + (audioBuffer2[next] * fraction);
    else
      sActif = (audioBuffer[base] * (1.0f - fraction)) + (audioBuffer[next] * fraction);

    // Sample du buffer actif (pour le crossfade)
    float sAutre;
    if (ecritureA)
      sAutre = (audioBuffer[base] * (1.0f - fraction)) + (audioBuffer[next] * fraction);
    else
      sAutre = (audioBuffer2[base] * (1.0f - fraction)) + (audioBuffer2[next] * fraction);

    float sample = sActif;

    // Crossfade sur les 20 premiers échantillons après un swap
    if (crossfadeCounter < CROSSFADE_SAMPLES) {
      float alpha = (float)crossfadeCounter / (float)CROSSFADE_SAMPLES;
      sample = (sAutre * (1.0f - alpha)) + (sActif * alpha);
      crossfadeCounter++;
    }

    // Crossfade sur les 20 derniers échantillons avant le prochain swap
    int samplesAvantSwap = BUF_SIZE - (int)readIndex;
    if (samplesAvantSwap <= CROSSFADE_SAMPLES && samplesAvantSwap > 0) {
      float alpha = (float)samplesAvantSwap / (float)CROSSFADE_SAMPLES;
      sample = (sActif * alpha) + (sAutre * (1.0f - alpha));
    }

    outBlock->data[i] = (int16_t)(sample * gain);

    readIndex += ratio;
    if (readIndex >= BUF_SIZE) readIndex -= BUF_SIZE;
  }

  // 4. Swap sans reset du readIndex
  if (writeIdx >= BUF_SIZE) {
    writeIdx = 0;
    ecritureA = !ecritureA;
    crossfadeCounter = 0; // démarre le crossfade
  }

  release(inBlock);
  transmit(outBlock, 0);
  release(outBlock);
}
