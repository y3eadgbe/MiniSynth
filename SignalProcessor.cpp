#include "SignalProcessor.h"


SignalProcessor::SignalProcessor(void)
{
	this->sampleRate = 44100;
}


SignalProcessor::~SignalProcessor(void)
{
}

void SignalProcessor::setSampleRate(float rate)
{
	if (rate >= 22050)
	{
		this->sampleRate = rate;
		this->changedSampleRate();
	}
}

void SignalProcessor::changedSampleRate(void)
{
}

void SignalProcessor::processMonoralToMonoral(int size, double** signalIn, double** signalOut)
{
}

void SignalProcessor::processMonoralToStereo(int size, double** signalIn, double** signalOut)
{
}

void SignalProcessor::processStereoToMonoral(int size, double** signalIn, double** signalOut)
{
}

void SignalProcessor::processStereoToStereo(int size, double** signalIn, double** signalOut)
{
}