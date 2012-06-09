#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include "Wavetable.h"

WaveTable* WaveTable::waveTable = nullptr;

WaveTable::WaveTable(void)
{
	this->generateSineWave();
	this->generateTriangleWave();
	this->generateSawWave();
	this->generatePulseWave();
}


WaveTable::~WaveTable(void)
{
}

WaveTable* WaveTable::getWaveTable(void)
{
	
	if (waveTable == nullptr)
	{
		waveTable = new WaveTable();
	}

	return waveTable;
}

double WaveTable::getWave(WaveForm waveForm, int phase, int harmonics)
{
	switch (waveForm)
	{
	case SINE:
		return this->getSineWave(phase, harmonics);
	case TRIANGLE:
		return this->getTriangleWave(phase, harmonics);
	case SAW:
		return this->getSawWave(phase, harmonics);
	case PULSE:
		return this->getPulseWave(phase, harmonics);
	default:
		return this->getSineWave(phase, harmonics);
	}
}

double WaveTable::getSineWave(int phase, int harmonics)
{
	if (harmonics == 0)
	{
		return 0;
	}
	return this->getInterpolatedValue(this->sineWave, phase);
}

double WaveTable::getTriangleWave(int phase, int harmonics)
{

	if (harmonics == 0)
	{
		return 0;
	}
	return this->getInterpolatedValue(this->triangleWave[(harmonics - 1) / 2], phase);
}

double WaveTable::getSawWave(int phase, int harmonics)
{
	if (harmonics == 0)
	{
		return 0;
	}
	return this->getInterpolatedValue(this->sawWave[harmonics - 1], phase);
}

double WaveTable::getPulseWave(int phase, int harmonics)
{
	if (harmonics == 0)
	{
		return 0;
	}
	return this->getInterpolatedValue(this->pulseWave[(harmonics - 1) / 2], phase);
}

double WaveTable::getInterpolatedValue(double* table, int phase)
{
	int divisor = (1<<23) / SAMPLE_SIZE;
	double ratio = (double)(phase % divisor) / (double)divisor;
	int index = ((phase >> 8) * this->SAMPLE_SIZE) >> 15; // prevent overflow
	return table[index] * (1.0 - ratio) + table[(index + 1) % this->SAMPLE_SIZE] * ratio;
}

void WaveTable::generateSineWave(void)
{
	for (int i = 0; i < this->SAMPLE_SIZE; i++)
	{
		double phase = M_PI * 2.0 * i / (double) this->SAMPLE_SIZE;
		this->sineWave[i] = sin(phase);
	}
}

void WaveTable::generateTriangleWave(void)
{
	for (int i = 0; i < this->SAMPLE_SIZE; i++)
	{
		double phase = M_PI * 2.0 * i / (double) this->SAMPLE_SIZE;
		this->triangleWave[0][i] = 8.0 * sin(phase) / (M_PI * M_PI);
	}
	for (int i = 1; i < (this->HARMONICS_MAX + 1) / 2; i++)
	{
		double n = 2 * i + 1;
		double sign = 1.0;
		if (i % 2 == 1)
		{
			sign = -1.0;
		}
		for (int j = 0; j < this->SAMPLE_SIZE; j++)
		{
			double phase = M_PI * 2.0 * j / (double) this->SAMPLE_SIZE;
			this->triangleWave[i][j] = this->triangleWave[i - 1][j] + sign * 8.0 * sin(n * phase) / (M_PI * M_PI * n * n);
		}
	}
}

void WaveTable::generateSawWave(void)
{
	for (int i = 0; i < this->SAMPLE_SIZE; i++)
	{
		double phase = M_PI * 2.0 * i / (double) this->SAMPLE_SIZE;
		this->sawWave[0][i] = 2.0 * sin(phase) / M_PI;
	}
	for (int i = 1; i < this->HARMONICS_MAX; i++)
	{
		double n = i + 1;
		for (int j = 0; j < this->SAMPLE_SIZE; j++)
		{
			double phase = M_PI * 2.0 * j / (double) this->SAMPLE_SIZE;
			this->sawWave[i][j] = this->sawWave[i - 1][j] + 2.0 * sin(n * phase) / (M_PI * n);
		}
	}
}

void WaveTable::generatePulseWave(void)
{
	for (int i = 0; i < this->SAMPLE_SIZE; i++)
	{	
		if (i < this->SAMPLE_SIZE / 2)
		{
			this->pulseWave[0][i] = 1.0;
		} else {
			this->pulseWave[0][i] = -1.0;
		}
	}
}