#pragma once
class SignalProcessor
{
public:
	SignalProcessor(void);
	virtual ~SignalProcessor(void);

	void setSampleRate(float);
	virtual void changedSampleRate(void);

	virtual void processMonoralToMonoral(int, double**, double**);
	virtual void processMonoralToStereo(int, double**, double**);
	virtual void processStereoToMonoral(int, double**, double**);
	virtual void processStereoToStereo(int, double**, double**);

protected:
	float sampleRate;
};