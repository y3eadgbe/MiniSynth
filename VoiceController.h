#pragma once
#include "Oscillator.h"
#include "SignalProcessor.h"
#include "Filter.h"
#include "Wavetable.h"
#include "common.h"

class VoiceController :
	public SignalProcessor
{
public:
	static const int OSCILLATOR_SIZE = 2;

	VoiceController(void);
	virtual ~VoiceController(void);

	virtual void processMonoralToMonoral(int size, double** signalIn, double** signalOut);
	virtual void changedSampleRate(void);
	void setFrequency(double value);
	double getFrequency(void);
	void setVelocity(double velocity);
	double getVelocity(void);
	double getPeakVelocity(void);
	void setWaveForm(WaveForm waveform, int index);
	void setCutOffFrequency(double value);
	void setResonance(double value);

	bool isBusy(void);
	bool isReleasing(void);
	int getNoteNo(void);

	void triggerNoteOn(int noteNo, double velocity);
	void triggerNoteChange(int noteNo, double velocity);
	void triggerNoteChangeWithoutAttack(int noteNo, double velocity);
	void triggerNoteOff(void);

	void stopSound(void);

	void setAttackTime(double value);
	void setDecayTime(double value);
	void setSustain(double value);
	void setReleaseTime(double value);
	void setGain(double value);
	void setPitch(double value);
	void setPortamentTime(double value);
	void setPortamentAuto(bool value);
	void setOscillatorDetune(int n, double value);
	void setOscillatorPitch(int n, double value);

	void updateParameters(int size);
protected:
	double calculateFrequency(int noteNo, double cent);
	double** monoralBuffer[OSCILLATOR_SIZE];

	bool isPortamentMode;

	Oscillator* oscillator[OSCILLATOR_SIZE];
	Filter* filter;
	int noteNo;
	EnvelopeState envelopeState;
	EnvelopeState filterState;
	double frequency;
	double velocity;
	double baseVelocity;
	double targetVelocity;
	double gain[OSCILLATOR_SIZE];
	double peakVelocity;
	double attackTime;
	double decayTime;
	double sustain;
	double releaseTime;

	double oscillatorDetune[2];
	double oscillatorPitch[2];

	double filterAttackTime;
	double filterDecayTime;
	double filterSustain;
	double filterReleaseTime;
	double filterAmount;
	double filterRatio;

	double cutOffFrequency;
	double resonance;

	int envelopeFrame;
	int filterEnvelopeFrame;

	double portamentTime;
	bool portamentAuto;

	double pitch;
};