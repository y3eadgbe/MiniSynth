#include "Oscillator.h"
#include "Wavetable.h"
#include <cmath>
#include <algorithm>

Oscillator::Oscillator(void)
{
	this->waveForm = SINE;
	this->waveTable = WaveTable::getWaveTable();
	this->phase = 0;

	setFrequency(440);
}

Oscillator::~Oscillator(void)
{
}

void Oscillator::processMonoralToMonoral(int size, double** signalIn, double** signalOut)
{
	int phaseTmp = this->phase;
	int phaseDelta = (int)(this->frequency / this->sampleRate * 8388608.0);
	for (int i = 0; i < size; i++)
	{
		signalOut[0][i] = this->waveTable->getWave(this->waveForm, phaseTmp, this->harmonics);
		phaseTmp += phaseDelta;
		phaseTmp &= (1 << 23) - 1;
	}
	this->phase = phaseTmp;
	if (this->portamentTime != 0)
	{
		this->portamentRatio += (double)size / (this->portamentTime * this->sampleRate);
		if (this->portamentRatio >= 1.f)
		{
			this->portamentRatio = 1;
			this->frequency = this->targetFrequency;
			this->baseFrequency = this->targetFrequency;
			this->portamentTime = 0;
		} else {
			this->frequency = this->calculatePortamentRatio(this->portamentRatio) * (this->targetFrequency - this->baseFrequency) + this->baseFrequency;
		}
	}
}

void Oscillator::setWaveForm(WaveForm wf)
{
	this->waveForm = wf;
}

void Oscillator::setFrequency(double frequency)
{
	if (frequency < 0) frequency = 0;

	this->baseFrequency = frequency;
	this->frequency = frequency;
	this->targetFrequency = frequency;
	this->portamentTime = 0;
	this->portamentRatio = 1;
	if (frequency != 0)
	{
		this->harmonics = std::min((int)(this->sampleRate / (2.0 * frequency)), this->waveTable->HARMONICS_MAX);
	}
}

void Oscillator::setPortamentFrequency(double frequency, double time)
{
	if (frequency < 0) frequency = 0;
	if (time < 0) time = 0;

	if (time == 0) {
		this->setFrequency(frequency);
	} else {
		this->baseFrequency = this->frequency;
		this->targetFrequency = frequency;
		this->portamentTime = time;
		this->portamentRatio = 0;
	}
}

void Oscillator::setPortamentTime(double time)
{
	if (time < 0) time = 0;
	this->portamentTime = time;
}

void Oscillator::resetPhase(void)
{
	this->phase = 0;
}

double Oscillator::calculatePortamentRatio(double x)
{
	return (1.f-exp(-x*4.f)*(1.f-x));
}