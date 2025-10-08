#pragma once

class Oscillator {
   public:
    virtual ~Oscillator();
    virtual void prepareToPlay(double sampleRate) = 0;
    virtual void releaseResources() = 0;

    virtual float getNextSample();
    virtual void resetPhase();
    virtual void setFrequency();

   private:
    float phase;
    float phaseIncrement;
};

class Wavetable {
   public:
    virtual float at(float phase) const = 0;
};

class PCMWavetable : public Wavetable {};

class PureSineWavetable : public Wavetable {};

class PWMWavetable : public Wavetable {};
