#ifndef faust_teensy_h_
#define faust_teensy_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"
#include "Fm.h"

#define AUDIO_OUTPUTS 1
#define AUDIO_INPUTS 1
#define BUF_SIZE 2048
#define CROSSFADE_SAMPLES 100

class MyDsp : public AudioStream
{
  public:
    MyDsp();
    ~MyDsp();
    virtual void update(void);
    void setCFreq(float freq);
    void setMFreq(float freq);
    void setIndex(float i);
    void setGain(float gain);
    void setRatio(float r) { ratio = r; }

  private:
    Fm fm;
    float ratio = 1.0;
    float readIndex = 0.0;
    int16_t audioBuffer[BUF_SIZE];
    int16_t audioBuffer2[BUF_SIZE];
    bool ecritureA = true;
    bool bufferPret = false;
    int writeIdx = 0;
    float gain = 1.0;
    audio_block_t *inputQueueArray[1];
    int crossfadeCounter = 0;
};

#endif