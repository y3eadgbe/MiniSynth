#include "Delay.h"


Delay::Delay(void)
{
	this->buffer[0] = new double[BUFFER_SIZE];
	this->buffer[1] = new double[BUFFER_SIZE];
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < BUFFER_SIZE; j++)
		{
			this->buffer[i][j] = 0;
		}
	}
	this->delayTime = 15000;
	this->bufferIndex = 0;
	this->clearTime = 0;
	this->feedback = 0;
}


Delay::~Delay(void)
{
	delete[] this->buffer[0];
	delete[] this->buffer[1];
}

void Delay::changedSampleRate(void)
{
}

void Delay::processStereoToStereo(int size, double** signalIn, double** signalOut)
{
	int bi = this->bufferIndex;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			int index = (bi - delayTime + BUFFER_SIZE) % BUFFER_SIZE;
			double delaySound = this->buffer[j][index];
			if (this->clearTime) {
				delaySound = 0;
			}
			buffer[j][bi] = signalIn[j][i] * this->feedback + delaySound * this->feedback;
			signalOut[j][i] = signalIn[j][i] + delaySound;
		}
		bi = (bi + 1) % BUFFER_SIZE;
		if (this->clearTime)
		{
			this->clearTime--;
		}
	}
	this->bufferIndex = bi;
}

void Delay::clear()
{
	this->clearTime = this->delayTime;
}

void Delay::setDelayTime(double time)
{
	int t = (int)(this->sampleRate * time);
	if (t < 41)
	{
		t = 41;
	}
	if (t > BUFFER_SIZE)
	{
		t = BUFFER_SIZE;
	}
	int diff = t - this->delayTime;
	if (diff > 0 && this->clearTime > 0)
	{
		this->clearTime += diff;
	}
	this->delayTime = t;
}

void Delay::setFeedback(double value)
{
	if (value < 0)
	{
		value = 0;
	}
	if (value > 1)
	{
		value = 1;
	}
	this->feedback = value;
}