#include "VoiceController.h"
#include "Wavetable.h"
#include <cmath>
#include <algorithm>
#include "Synthesizer.h"
#include "Filter.h"

VoiceController::VoiceController(void)
{
	for (int i = 0; i < OSCILLATOR_SIZE; i++)
	{
		this->oscillator[i] = new Oscillator;
		this->gain[i] = 1.0;
		this->monoralBuffer[i] = new double*;
		this->monoralBuffer[i][0] = new double[SynthConsts::BUFFER_SIZE];
	}

	this->filter = new Filter;
	this->frequency = 440;
	this->envelopeState = IDLE;
	this->filterState = IDLE;

	this->attackTime = 0;
	this->decayTime = 0;
	this->sustain = 1.0;
	this->releaseTime = 0;

	this->filterAttackTime = 0;
	this->filterDecayTime = 0.1;
	this->filterSustain = 0;
	this->filterReleaseTime = 0.5;
	this->filterAmount = 0;

	this->pitch = 0;
	this->portamentTime = 0;
	this->velocity = 0;
	this->peakVelocity = 0;
	this->baseVelocity = 0;
	this->targetVelocity = 0;
	this->isPortamentMode = true;

	this->envelopeFrame = 0;
	this->filterEnvelopeFrame = 0;
}


VoiceController::~VoiceController(void)
{
	for (int i = 0; i < OSCILLATOR_SIZE; i++)
	{
		delete[] this->monoralBuffer[i];
		delete this->monoralBuffer[i];
	}
}

void VoiceController::processMonoralToMonoral(int size, double** signalIn, double** signalOut)
{
	int i = 0;

	// Oscillator
	for (int i = 0; i < size; i++)
	{
		signalOut[0][i] = 0;
	}
	for (int i = 0; i < OSCILLATOR_SIZE; i++)
	{
		this->oscillator[i]->processMonoralToMonoral(size, nullptr, monoralBuffer[i]);
	}

	while (i < size)
	{
		int processSize = std::min(size, i + 24);
		int processFrame = processSize;
		int frameLeft;
		double velocityDelta = 0;
		
		
		// Amplifier Envelope
		switch (this->envelopeState)
		{
		case ATTACK:
			if ((frameLeft = (int)(this->attackTime * this->sampleRate) - this->envelopeFrame) != 0)
			{
				velocityDelta = (this->peakVelocity - this->velocity) / (double)frameLeft;
			}
			if (frameLeft > processSize - i)
			{
				processFrame = processSize;
				this->envelopeFrame += processSize - i;
			} else {
				processFrame = i + (int)(this->attackTime * this->sampleRate);
				this->envelopeFrame = 0;
				this->envelopeState = DECAY;
			}

			if (i == processFrame)
			{
				this->velocity = this->peakVelocity;
			}
			break;
		case DECAY:
			if ((frameLeft = (int)(this->decayTime * this->sampleRate) - this->envelopeFrame) != 0)
			{
				velocityDelta = (this->sustain * this->peakVelocity - this->velocity) / (double)frameLeft;
			}
			if (frameLeft > processSize - i)
			{
				processFrame = processSize;
				this->envelopeFrame += processSize - i;
			} else {
				processFrame = i + (int)(this->decayTime * this->sampleRate);
				this->envelopeFrame = 0;
				this->envelopeState = SUSTAIN;
			}
			break;
		case SUSTAIN:
			this->velocity = this->peakVelocity * this->sustain;
			processFrame = processSize;
			velocityDelta = 0;
			break;
		case RELEASE:
			if ((frameLeft = (int)(this->releaseTime * this->sampleRate) - this->envelopeFrame) != 0)
			{
				velocityDelta = - this->velocity / (double)frameLeft;
			}
			if (frameLeft > processSize - i)
			{
				processFrame = processSize;
				this->envelopeFrame += processSize - i;
			} else {
				processFrame = i + (int)(this->releaseTime * this->sampleRate);
				this->envelopeFrame = 0;
				this->envelopeState = IDLE;
			}
			break;
		case IDLE:
		default:
			this->velocity = 0;
			processFrame = processSize;
			velocityDelta = 0;
			break;
		}

		processFrame = std::min(processFrame, processSize);

		// Filter Envelope
		switch (this->filterState)
		{
		case ATTACK:
			if ((frameLeft = (int)(this->filterAttackTime * this->sampleRate) - this->filterEnvelopeFrame) != 0)
			{
				this->filterRatio += (1 - this->filterRatio) / (double)frameLeft * std::min(frameLeft, processFrame - i);
			} else {
				this->filterRatio = 1;
			}
			if (frameLeft > processFrame - i)
			{
				this->filterEnvelopeFrame += processFrame - i;
			} else {
				this->filterEnvelopeFrame = 0;
				this->filterState = DECAY;
			}
			break;
		case DECAY:
			if ((frameLeft = (int)(this->filterDecayTime * this->sampleRate) - this->filterEnvelopeFrame) != 0)
			{
				this->filterRatio += (this->filterSustain - this->filterRatio) / (double)frameLeft * std::min(frameLeft, processFrame - i);
			} else {
				this->filterRatio = this->filterSustain;
			}
			if (frameLeft > processFrame - i)
			{
				this->filterEnvelopeFrame += processFrame - i;
			} else {
				this->filterEnvelopeFrame = 0;
				this->filterState = SUSTAIN;
			}
			break;
		case SUSTAIN:
			filterRatio = this->filterSustain;
			break;
		case RELEASE:
			if ((frameLeft = (int)(this->filterReleaseTime * this->sampleRate) - this->filterEnvelopeFrame) != 0)
			{
				this->filterRatio += -this->filterRatio / (double)frameLeft * std::min(frameLeft, processFrame - i);
			} else {
				this->filterRatio = 0;
			}
			if (frameLeft > processFrame - i)
			{
				this->filterEnvelopeFrame += processFrame - i;
			} else {
				this->filterEnvelopeFrame = 0;
				this->filterState = IDLE;
			}
			break;
		case IDLE:
		default:
			this->filterEnvelopeFrame = 0;
			break;
		}

		this->filter->setCutOffFrequency(this->cutOffFrequency + this->filterRatio * this->filterAmount);
		
		for ( ; i < processFrame; i++)
		{
			this->velocity += velocityDelta;
			signalOut[0][i] = 0;
			for (int j = 0; j < OSCILLATOR_SIZE; j++)
			{
				signalOut[0][i] += monoralBuffer[j][0][i] * this->gain[j];				
			}
			signalOut[0][i] = this->filter->processOnce(signalOut[0][i]);
			signalOut[0][i] *= this->velocity;
		}
		this->updateParameters(processFrame);
	}

	//this->filter->processMonoralToMonoral(size, signalOut, signalOut);

	if (this->envelopeState == IDLE)
	{
		this->filter->clean();
	}
}

void VoiceController::changedSampleRate(void)
{
	for (int i = 0; i < OSCILLATOR_SIZE; i++)
	{
		this->oscillator[i]->setSampleRate(this->sampleRate);
	}
}

void VoiceController::setWaveForm(WaveForm wf, int index)
{
	if (0 <= index && index < OSCILLATOR_SIZE)
	{
		this->oscillator[index]->setWaveForm(wf);
	}
}

void VoiceController::setFrequency(double value)
{
	if (value < 0)
	{
		this->frequency = 0;
	} else {
		this->frequency = value;
	}
	for (int i = 0; i < OSCILLATOR_SIZE; i++)
	{
		if (this->isPortamentMode)
		{
			if (this->portamentAuto == true && this->envelopeState == IDLE) {
				this->oscillator[i]->setFrequency(this->frequency);
			} else {
				this->oscillator[i]->setPortamentFrequency(this->frequency, this->portamentTime);
			}
		} else {
			this->oscillator[i]->setFrequency(this->frequency);
		}
	}
}

double VoiceController::getFrequency(void)
{
	return this->frequency;
}

void VoiceController::setVelocity(double velocity)
{
	if (velocity < 0)
	{
		this->velocity = 0;
	} else if (velocity > 1) {
		this->velocity = 1;
	} else {
		this->velocity = velocity;
	}
}

void VoiceController::setPitch(double pitch)
{
	this->pitch = pitch;
	this->setFrequency(this->calculateFrequency(this->noteNo, this->pitch));
}

double VoiceController::getVelocity(void)
{
	return this->velocity;
}

double VoiceController::getPeakVelocity(void)
{
	return this->peakVelocity;
}

bool VoiceController::isBusy()
{
	return this->envelopeState != IDLE;
}

bool VoiceController::isReleasing()
{
	return this->envelopeState == RELEASE;
}

int VoiceController::getNoteNo()
{
	return this->noteNo;
}

void VoiceController::triggerNoteOn(int noteNo, double velocity)
{
	if (this->envelopeState == IDLE)
	{
		for (int i = 0; i < OSCILLATOR_SIZE; i++)
		{	
			this->oscillator[i]->resetPhase();
		}
	}

	this->noteNo = noteNo;
	this->envelopeFrame = 0;
	this->filterEnvelopeFrame = 0;
	this->velocity = 0;
	this->filterRatio = 0;
	this->peakVelocity = velocity;
	this->setFrequency(this->calculateFrequency(noteNo, this->pitch));
	this->envelopeState = ATTACK;
	this->filterState = ATTACK;
}

void VoiceController::triggerNoteChange(int noteNo, double velocity)
{
	this->noteNo = noteNo;
	this->envelopeFrame = 0;
	this->filterEnvelopeFrame = 0;
	this->peakVelocity = velocity;
	this->setFrequency(this->calculateFrequency(noteNo, this->pitch));
	this->envelopeState = ATTACK;
	this->filterState = ATTACK;
}

void VoiceController::triggerNoteChangeWithoutAttack(int noteNo, double velocity)
{
	this->noteNo = noteNo;
	this->peakVelocity = velocity;
	this->setFrequency(this->calculateFrequency(noteNo, this->pitch));
	if (this->envelopeState == RELEASE || this->envelopeState == IDLE)
	{
		this->envelopeFrame = 0;
		this->filterEnvelopeFrame = 0;
		this->envelopeState = ATTACK;
		this->filterState = ATTACK;
	}
}

void VoiceController::triggerNoteOff(void)
{
	this->envelopeFrame = 0;
	this->envelopeState = RELEASE;
	this->filterState = RELEASE;
}

void VoiceController::stopSound(void)
{
	this->envelopeFrame = 0;
	this->envelopeState = IDLE;
	this->filterState = IDLE;
	this->velocity = 0;
	this->filterRatio = 0;
}

double VoiceController::calculateFrequency(int noteNo, double cent = 0)
{
	return pow(2, ((double)noteNo - 69.0 + cent / 100.0) / 12.0) * 440.0;
}

void VoiceController::setAttackTime(double value)
{
	if (value < 0.0004) value = 0.0004;
	this->attackTime = value;
}

void VoiceController::setDecayTime(double value)
{
	this->decayTime = value;
}

void VoiceController::setSustain(double value)
{
	this->sustain = value;
}

void VoiceController::setReleaseTime(double value)
{
	this->releaseTime = value;
}

void VoiceController::setGain(double value)
{
	this->gain[0] = value;
	this->gain[1] = 1 - value;
	if (gain[0] < EPS)
	{
		gain[0] = 0;
	}
	if (gain[1] < EPS)
	{
		gain[1] = 0;
	}
}

void VoiceController::setCutOffFrequency(double value)
{
	this->cutOffFrequency = value;
	//this->filter->setCutOffFrequency(value);
}

void VoiceController::setResonance(double value)
{
	this->filter->setResonance(value);
}

void VoiceController::setPortamentTime(double value)
{
	this->portamentTime = value;
	for (int i = 0; i < this->OSCILLATOR_SIZE; i++)
	{
		this->oscillator[i]->setPortamentTime(value);
	}
}

void VoiceController::setPortamentAuto(bool value)
{
	this->portamentAuto = value;
}

void VoiceController::updateParameters(int size)
{
	this->filter->updateParameters(size);
}