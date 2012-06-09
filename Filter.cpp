#define _USE_MATH_DEFINES
#include "Filter.h"
#include "SignalProcessor.h"
#include "common.h"
#include <cmath>
#include <algorithm>

Filter::Filter(void)
{
	this->x1 = this->x2 = 0;
	this->y1 = this->y2 = 0;
	this->f0 = 2500;
	this->f0To = 2000;
	this->Q = 0.78f;
	updateParameter();
}


Filter::~Filter(void)
{
}

void Filter::changedSampleRate(void)
{
	updateParameter();
}

void Filter::setCutOffFrequency(double f0)
{
	if (f0 <= 0)
	{
		f0 = 1;
	}
	if (f0 > 22000)
	{
		f0 = 22000;
	}
	this->f0To = f0;
	this->updateParameter();
}

void Filter::setResonance(double Q)
{
	this->Q = Q;
	this->updateParameter();
}

void Filter::updateParameter(void)
{
	double omega, sn, cs, alpha;
	double a0, a1, a2, b0, b1, b2;

	omega = 2.f * M_PI * f0 / this->sampleRate;
	sn = sin(omega);
	cs = cos(omega);
	alpha = sn / (2.f * Q);

	b0 = (1.f - cs) / 2.f;
	b1 = 1.f - cs;
	b2 = (1.f - cs) / 2.f;
	a0 = 1.f + alpha;
	a1 = -2.f * cs;
	a2 = 1.f - alpha;

	this->b0a0 = b0 /a0;
	this->b1a0 = b1 /a0;
	this->b2a0 = b2 /a0;
	this->a1a0 = a1 /a0;
	this->a2a0 = a2 /a0;
}

void Filter::clean(void)
{
	this->x1 = this->x2 = 0;
	this->y1 = this->y2 = 0;
}

double Filter::processOnce(double signalIn)
{
	double x0 = signalIn;
	double res = b0a0 * x0 + b1a0 * x1 + b2a0 * x2 - a1a0 * y1 - a2a0 * y2;
	x2 = x1;
	x1 = x0;
	y2 = y1;
	y1 = res;
	return res;
}

void Filter::processMonoralToMonoral(int size, double** signalIn, double** signalOut)
{
	for (int i = 0; i < size; i++)
	{
		int nextSize = std::min(size, i + 24);
		int elapsed = nextSize - i;
		for (; i < size; i++)
		{
			double x0 = signalIn[0][i];
			if (abs(x0) < EPS)x0 = 0;
			signalOut[0][i] = b0a0 * x0 + b1a0 * x1 + b2a0 * x2 - a1a0 * y1 - a2a0 * y2;
			x2 = x1;
			x1 = x0;
			y2 = y1;
			y1 = signalOut[0][i];
		}
		this->updateParameters(elapsed);
	}
}

void Filter::updateParameters(int size)
{
	if (abs(x1) < EPS) x1 = 0;
	if (abs(x2) < EPS) x2 = 0;
	if (abs(y1) < EPS) y1 = 0;
	if (abs(y2) < EPS) y2 = 0;
	this->f0 += (1 - pow(0.99, size)) * (this->f0To - this->f0);
	if (abs(this->f0To - this->f0) < EPS) this->f0To = this->f0;
	this->updateParameter();
}

void Filter::setSampleRate(float freq)
{
	this->sampleRate = freq;
}