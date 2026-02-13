#ifndef faust_teensy_h_
#define faust_teensy_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

#include "Fm.h"

#define AUDIO_OUTPUTS 1
#define AUDIO_INPUTS 1

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
    int16_t audioBuffer[1024]; // tampon circulaire 
    int writeIdx = 0;
    float _gain = 1.0;
};

#endif
