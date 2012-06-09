#pragma once
#include "SignalProcessor.h"
class Delay :
	public SignalProcessor
{
public:
	Delay(void);
	virtual ~Delay(void);
	virtual void changedSampleRate(void);
	
	virtual void processStereoToStereo(int size, double** signalIn, double** signalOut);
	void clear();
	void setDelayTime(double time);
	void setFeedback(double value);

private:
	static const int BUFFER_SIZE = 44100;
	int delayTime;
	int bufferIndex;
	int clearTime;
	double* buffer[2];
	double feedback;
};

