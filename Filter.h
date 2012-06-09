#pragma once
#include "SignalProcessor.h"
#include <cmath>

class Filter : SignalProcessor
{
public:
	Filter(void);
	virtual ~Filter(void);

	virtual void changedSampleRate(void);

	void setSampleRate(float freq);
	void setResonance(double Q);
	void setCutOffFrequency(double f0);
	void processMonoralToMonoral(int size, double** signalIn, double** signalOut);
	double processOnce(double signalIn);
	void clean(void);
	void updateParameters(int size);

private:
	double x1, x2;
	double y1, y2;
	double b0a0, b1a0, b2a0, a1a0, a2a0;
	double f0, Q;
	double f0To;

	void updateParameter(void);
};