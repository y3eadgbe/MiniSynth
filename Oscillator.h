#pragma once
#include "SignalProcessor.h"
#include "wavetable.h"

class Oscillator :
	public SignalProcessor
{
public:
	Oscillator(void);
	virtual ~Oscillator(void);

	virtual void processMonoralToMonoral(int size, double** signalIn, double** signalOut);
	void setFrequency(double frequency);
	void setPortamentFrequency(double frequency, double time);
	void setPortamentTime(double time);
	void setWaveForm(WaveForm wf);
	void resetPhase(void);

private:
	double calculatePortamentRatio(double x);

	WaveForm waveForm;
	WaveTable* waveTable;
	double frequency;
	double baseFrequency;
	double targetFrequency;
	double portamentTime;
	double portamentRatio;
	int phase;
	int harmonics;
};