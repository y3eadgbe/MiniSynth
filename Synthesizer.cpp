#include "Synthesizer.h"
#include "Delay.h"
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

using namespace SynthConsts;

Synthesizer::Synthesizer(void)
{
	this->voiceCount = 0;
	this->voiceIndex = 0;
	this->pitchBendSensitivity = 2.0;

	for (int i = 0; i < VOICE_CONTROLLER_SIZE; i++)
	{
		this->voiceController[i] = new VoiceController;
		this->monoralBuffer[i] = new double*;
		this->monoralBuffer[i][0] = new double[BUFFER_SIZE];
	}

	this->delay = new Delay;
	this->delaySwitch = false;

	this->masterVolume = 0;
	this->masterVolumeTo = 0;

	this->parameterName[MASTER_VOLUME] = "MasterVolume";
	this->parameterName[OSCILLATOR1_WAVEFORM] = "Osc1WaveForm";
	this->parameterName[OSCILLATOR2_WAVEFORM] = "Osc2WaveForm";
	this->parameterName[AMP_ATTACK_TIME] = "AmpAttackTime";
	this->parameterName[AMP_DECAY_TIME] = "AmpDecayTime";
	this->parameterName[AMP_SUSTAIN] = "AmpSustain";
	this->parameterName[AMP_RELEASE_TIME] = "AmpReleaseTime";
	this->parameterName[OSCILLATOR_MIX] = "OscMix";
	this->parameterName[CUT_OFF_FREQUENCY] = "CutOffFreq";
	this->parameterName[RESONANCE] = "Resonance";
	this->parameterName[PORTAMENT_TIME] = "PortaTime";
	this->parameterName[VOICE_MODE] = "VoiceMode";
	this->parameterName[PORTAMENT_AUTO] = "PortaAuto";
	this->parameterName[DELAY_SWITCH] = "Delay";
	this->parameterName[DELAY_TIME] = "DelayTime";
	this->parameterName[DELAY_FEEDBACK] = "DelayFeedBack";

	this->waveFormName[SINE] = "Sine";
	this->waveFormName[TRIANGLE] = "Triangle";
	this->waveFormName[SAW] = "Saw";
	this->waveFormName[PULSE] = "Pulse";
	this->waveFormName[NOISE] = "Noise";

	this->voiceModeName[POLY] = "Poly";
	this->voiceModeName[MONO] = "Mono";
	this->voiceModeName[LEGATO] = "Legato";

	this->curProgram = 0;

	for (int i = 0; i < PROGRAM_NUMBER; i++)
	{
		this->programs[i].parameter[MASTER_VOLUME] = 0.5;
		this->programs[i].parameter[AMP_SUSTAIN] = 1.0;
		this->programs[i].parameter[CUT_OFF_FREQUENCY] = 0.5;
	}
	this->setProgram(0);
}


Synthesizer::~Synthesizer(void)
{
	for (int i = 0; i < VOICE_CONTROLLER_SIZE; i++)
	{
		delete this->voiceController[i];
		delete[] this->monoralBuffer[i];
		delete this->monoralBuffer[i];
	}
}

void Synthesizer::processMonoralToStereo(int size, double** signalIn, double** signalOut)
{
	this->processStereoToStereo(size, nullptr, signalOut);
}

void Synthesizer::processStereoToStereo(int size, double** signalIn, double** signalOut)
{
	for (int i = 0; i < size; i++)
	{
		signalOut[0][i] = 0;
		signalOut[1][i] = 0;
	}

	double volume = this->masterVolume;
	double volumeTo = this->masterVolumeTo;

	// mix
	for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
	{
		if (this->voiceController[i]->isBusy())
		{
			this->voiceController[i]->processMonoralToMonoral(size, nullptr, monoralBuffer[i]);
			for (int j = 0; j < size; j++)
			{
				signalOut[0][j] += monoralBuffer[i][0][j];
				signalOut[1][j] += monoralBuffer[i][0][j];
			}
		} else {
			this->voiceController[i]->updateParameters(size);
		}
	}

	// delay
	if (this->delaySwitch)
	{
		this->delay->processStereoToStereo(size, signalOut, signalOut);
	}

	// master volume
	for (int j = 0; j < size; j++)
	{
		signalOut[0][j] *= volume;
		signalOut[1][j] *= volume;
		
		// change volume smoothly
		volume += (volumeTo - volume) * 0.008;
	}
	this->masterVolume = volume;
	if (abs(this->masterVolume - this->masterVolumeTo) < EPS)
	{
		this->masterVolume = this->masterVolumeTo;
	}
}

void Synthesizer::setParameter(int index, double value)
{
	WaveForm wf;
	VoiceMode vm;
	switch (index)
	{
	case MASTER_VOLUME:
		this->masterVolumeTo = this->valueToMasterVolume(value);
		break;
	case OSCILLATOR1_WAVEFORM:
		wf = this->valueToWaveForm(this->programs[this->curProgram].parameter[OSCILLATOR1_WAVEFORM]);
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setWaveForm(wf, 0);
		}
		break;
	case OSCILLATOR2_WAVEFORM:
		wf = this->valueToWaveForm(this->programs[this->curProgram].parameter[OSCILLATOR2_WAVEFORM]);
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setWaveForm(wf, 1);
		}
		break;
	case AMP_ATTACK_TIME:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setAttackTime(this->valueToAttackTime(value));
		}
		break;
	case AMP_DECAY_TIME:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setDecayTime(this->valueToTime(value));
		}
		break;
	case AMP_SUSTAIN:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setSustain(value);
		}
		break;
	case AMP_RELEASE_TIME:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setReleaseTime(this->valueToTime(value));
		}
		break;
	case OSCILLATOR_MIX:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setGain(value);
		}
		break;
	case CUT_OFF_FREQUENCY:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setCutOffFrequency(this->valueToCutOffFrequency(value));
		}
		break;
	case RESONANCE:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setResonance(this->valueToResonance(value));
		}
		break;
	case PORTAMENT_TIME:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setPortamentTime(value);
		}
		break;
	case VOICE_MODE:
		vm = this->valueToVoiceMode(value);
		if (this->voiceMode != vm)
		{
			if (this->voiceMode == POLY)
			{
				this->allNoteOff();
			}
			this->voiceMode = vm;
		}
		break;
	case PORTAMENT_AUTO:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			this->voiceController[i]->setPortamentAuto(this->valueToBool(value));
		}
		break;
	case DELAY_SWITCH:
		this->delaySwitch = this->valueToBool(value);
		if (!this->delaySwitch)
		{
			this->delay->clear();
		}
		break;
	case DELAY_TIME:
		this->delay->setDelayTime(value);
		break;
	case DELAY_FEEDBACK:
		this->delay->setFeedback(value);
		break;
	default:
		break;
	}
	this->programs[this->curProgram].parameter[index] = value;
}

double Synthesizer::getParameter(int index)
{
	//return this->programs[this->curProgram].parameter[index];
	return this->programs[this->curProgram].parameter[index];
}

void Synthesizer::getParameterName(int index, char* buffer)
{
	if (0 <= index && index < PARAMETER_NUMBER)
	{
		strcpy_s(buffer, 255,this->parameterName[index].c_str());
	}
}

void Synthesizer::getParameterDisplay(int index, char* buffer)
{
	std::stringstream ss;
	switch (index)
	{
	case OSCILLATOR1_WAVEFORM:
		strcpy_s(buffer, 255, this->waveFormName[this->valueToWaveForm(this->programs[this->curProgram].parameter[OSCILLATOR1_WAVEFORM])].c_str());
		break;
	case OSCILLATOR2_WAVEFORM:
		strcpy_s(buffer, 255, this->waveFormName[this->valueToWaveForm(this->programs[this->curProgram].parameter[OSCILLATOR2_WAVEFORM])].c_str());
		break;
	case VOICE_MODE:
		strcpy_s(buffer, 255, this->voiceModeName[this->valueToVoiceMode(this->programs[this->curProgram].parameter[VOICE_MODE])].c_str());
		break;
	case CUT_OFF_FREQUENCY:
		ss << this->valueToCutOffFrequency(this->programs[this->curProgram].parameter[index]);
		strcpy_s(buffer, 255, ss.str().c_str());
		break;
	case RESONANCE:
		ss << this->valueToResonance(this->programs[this->curProgram].parameter[index]);
		strcpy_s(buffer, 255, ss.str().c_str());
		break;
	case AMP_ATTACK_TIME:
		ss << this->valueToAttackTime(this->programs[this->curProgram].parameter[index]);
		strcpy_s(buffer, 255, ss.str().c_str());
		break;
	case AMP_DECAY_TIME:
	case AMP_RELEASE_TIME:
		ss << this->valueToTime(this->programs[this->curProgram].parameter[index]);
		strcpy_s(buffer, 255, ss.str().c_str());
		break;
	default:
		ss << this->programs[this->curProgram].parameter[index];
		strcpy_s(buffer, 255, ss.str().c_str());
		break;
	}
}

void Synthesizer::setProgram(int index)
{
	this->curProgram = index;
	for (int i = 0; i < PARAMETER_NUMBER; i++)
	{
		this->setParameter(i, this->programs[index].parameter[i]);
	}
}

void Synthesizer::changedSampleRate(void)
{
	for (int i = 0; i < VOICE_CONTROLLER_SIZE; i++)
	{
		this->voiceController[i]->setSampleRate(this->sampleRate);
	}
}

void Synthesizer::setTempo(double tempo)
{
	this->tempo = tempo;
}

void Synthesizer::receiveNoteOn(int noteNo, int velocity)
{
	if (velocity == 0)
	{
		this->receiveNoteOff(noteNo);
	} else {
		this->noteList.push_back(noteNo);
		switch (this->voiceMode)
		{
		case POLY:
			if (this->voiceCount == SynthConsts::VOICE_CONTROLLER_SIZE) {
				this->voiceController[this->voiceIndex]->triggerNoteOn(noteNo, (double)velocity / 127.0);
				this->voiceIndex = (this->voiceIndex + 1) % SynthConsts::VOICE_CONTROLLER_SIZE;
			} else {
				int releasingIndex = -1;
				int nonBusyIndex = -1;
				for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
				{
					int index = (this->voiceIndex + i) % SynthConsts::VOICE_CONTROLLER_SIZE;
					if (this->voiceController[index]->isReleasing() && releasingIndex == -1) releasingIndex = index;
					if (!this->voiceController[index]->isBusy())
					{
						nonBusyIndex = index;
						break;
					}
				}
				if (nonBusyIndex == -1 && releasingIndex == -1)
				{
					std::cerr << "Assert Failure: Synthesizer.cpp" << std::endl;
					return;
				}
					
				if (nonBusyIndex == -1) nonBusyIndex = releasingIndex;
				
				this->voiceController[nonBusyIndex]->triggerNoteOn(noteNo, (double)velocity / 127.0);
				this->voiceIndex = (nonBusyIndex + 1) % SynthConsts::VOICE_CONTROLLER_SIZE;
				this->voiceCount++;
			}
			break;
		case MONO:
			if (this->voiceController[0]->isBusy())
			{
				this->voiceController[0]->triggerNoteChange(noteNo, (double)velocity / 127.0);
			} else {
				this->voiceController[0]->triggerNoteOn(noteNo, (double)velocity / 127.0);
			}
			this->voiceIndex = 1;
			this->voiceCount++;
			break;
		case LEGATO:
			if (this->voiceController[0]->isBusy())
			{
				this->voiceController[0]->triggerNoteChangeWithoutAttack(noteNo, (double)velocity / 127.0);
			} else {
				this->voiceController[0]->triggerNoteOn(noteNo, (double)velocity / 127.0);
			}
			this->voiceIndex = 1;
			this->voiceCount++;
			break;
		default:
			break;
		}
	}
}

void Synthesizer::receiveNoteOff(int noteNo)
{
	double velocity;
	this->noteList.remove(noteNo);
	switch (this->voiceMode)
	{
	case POLY:
		for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
		{
			if (this->voiceController[i]->isBusy() && !this->voiceController[i]->isReleasing() && this->voiceController[i]->getNoteNo() == noteNo)
			{
				this->voiceController[i]->triggerNoteOff();
				this->voiceCount--;
			}
		}
		break;
	case MONO:
	case LEGATO:
		if (this->voiceController[0]->isBusy() && !this->voiceController[0]->isReleasing() && this->voiceController[0]->getNoteNo() == noteNo)
		{
			velocity = this->voiceController[0]->getPeakVelocity();
			//this->voiceController[0]->triggerNoteOff();
			this->voiceCount--;
			if (this->noteList.size() != 0)
			{
				this->receiveNoteOn(this->noteList.back(), (int)std::floor(velocity * 127 + 0.5));
			} else {
				this->voiceController[0]->triggerNoteOff();
			}
		}
	default:
		break;
	}
}

void Synthesizer::allNoteOff(void)
{
	this->noteList.clear();
	this->voiceCount = 0;
	for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
	{
		this->voiceController[i]->stopSound();
	}
}

void Synthesizer::receivePitchBend(int value)
{
	for (int i = 0; i < SynthConsts::VOICE_CONTROLLER_SIZE; i++)
	{
		this->voiceController[i]->setPitch(100.0 * this->pitchBendSensitivity * (double)value / 8192.0);
	}
}

void Synthesizer::processMidiEvent(char* ev)
{
	switch (ev[0] & 0xf0) // ignore channnel
	{
	case 0x80:
		// note off
		this->receiveNoteOff((int) ev[1]);
		break;
	case 0x90:
		// note on
		this->receiveNoteOn((int) ev[1], (int) ev[2]);
		break;
	case 0xE0:
		this->receivePitchBend((int)ev[2] * 128 + (int)ev[1] - 8192);
	default:
		break;
	}
}

double Synthesizer::valueToMasterVolume(double value)
{
	double res;
	if (value == 0)
	{
		res = 0;
	} else {
		res = (exp(value * 2) - 1) / (exp(2.f) - 1);
	}
	return res;
}

WaveForm Synthesizer::valueToWaveForm(double value)
{
	return (WaveForm)(std::min((int)(value * SynthConsts::WAVEFORM_SIZE), SynthConsts::WAVEFORM_SIZE - 1));
}

VoiceMode Synthesizer::valueToVoiceMode(double value)
{
	return (VoiceMode)(std::min((int)(value * VOICE_MODE_SIZE), VOICE_MODE_SIZE - 1));
}

double Synthesizer::valueToTime(double value)
{
	return 0.02 + 3.97 * ((exp(value * 3) - 1) / (exp(3.f) - 1));
}

double Synthesizer::valueToAttackTime(double value)
{
	return 4.f * ((exp(value * 3) - 1) / (exp(3.f) - 1));
}

double Synthesizer::valueToCutOffFrequency(double value)
{
	return 20.f + 21980.f * ((exp(value * 6) - 1) / (exp(6.f) - 1));
}

double Synthesizer::valueToResonance(double value)
{
	double res = 0.78 + 19.2 * ((exp(value * 4) - 1) / (exp(4.f) - 1));
	if (value == 1.0)
	{
		res = 40.0;
	}
	return res;
}

bool Synthesizer::valueToBool(double value)
{
	return value >= 0.5;
}