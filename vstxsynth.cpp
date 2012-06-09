#include "vstxsynth.h"
#include "Synthesizer.h"
#include "common.h"
#include <algorithm>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new VstXSynth (audioMaster);
}

VstXSynth::VstXSynth(audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, SynthConsts::PROGRAM_NUMBER, SynthConsts::PARAMETER_NUMBER)
{	
	this->programsAreChunks();
	this->setNumInputs(0);				// no inputs
	this->setNumOutputs(2);				// 2 outputs
	this->canProcessReplacing();
	this->isSynth();
	this->setUniqueID('VxS2');			// <<<! *must* change this!!!!

	this->vstEvents = nullptr;
	this->synthesizer = new Synthesizer();
	this->vstEventMax = 0;
	this->vstEventNum = 0;
	this->currentVstEvent = 0;
	this->currentSample = 0;
	this->curProgram = 0;
	this->numParams = SynthConsts::PARAMETER_NUMBER;
	this->numPrograms = SynthConsts::PROGRAM_NUMBER;

	this->setProgram(0);

	this->suspend();
}

VstXSynth::~VstXSynth()
{
}

void VstXSynth::setProgram(VstInt32 program)
{
	if (program < 0 || program >= SynthConsts::PROGRAM_NUMBER)
		return;
	
	this->curProgram = program;
	this->synthesizer->setProgram(program);
}

void VstXSynth::setProgramName(char* name)
{
	vst_strncpy(this->synthesizer->programs[this->curProgram].name, name, kVstMaxProgNameLen);
}

void VstXSynth::getProgramName(char* name)
{
	vst_strncpy(name, this->synthesizer->programs[this->curProgram].name, kVstMaxProgNameLen);
}


void VstXSynth::getParameterDisplay(VstInt32 index, char* text)
{
	char buf[256];
	this->synthesizer->getParameterDisplay(index, buf);
	buf[kVstMaxParamStrLen] = '\0';
	vst_strncpy(text, buf, kVstMaxParamStrLen);
}

void VstXSynth::getParameterName(VstInt32 index, char* text)
{
	char buf[256];
	this->synthesizer->getParameterName(index, buf);
	buf[kVstMaxParamStrLen] = '\0';
	vst_strncpy(text, buf, kVstMaxParamStrLen);
}

void VstXSynth::setParameter(VstInt32 index, float value)
{
	this->synthesizer->setParameter(index, value);
}

float VstXSynth::getParameter(VstInt32 index)
{
	return (float)this->synthesizer->getParameter(index);
}

bool VstXSynth::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
	if (index < SynthConsts::PROGRAM_NUMBER)
	{
		vst_strncpy (text, this->synthesizer->programs[index].name, kVstMaxProgNameLen);
		return true;
	}
	return false;
}

bool VstXSynth::getEffectName(char* name)
{
	vst_strncpy (name, "MiniSynth", kVstMaxEffectNameLen);
	return true;
}

bool VstXSynth::getVendorString(char* text)
{
	vst_strncpy (text, "y3eadgbe", kVstMaxVendorStrLen);
	return true;
}

bool VstXSynth::getProductString(char* text)
{
	vst_strncpy (text, "MiniSynth", kVstMaxProductStrLen);
	return true;
}

VstInt32 VstXSynth::getVendorVersion()
{ 
	return 1000; 
}

VstInt32 VstXSynth::canDo(char* text)
{
	if (!strcmp (text, "receiveVstEvents")) return 1;
	if (!strcmp (text, "receiveVstMidiEvent")) return 1;
	return -1;
}

VstInt32 VstXSynth::setChunk(void* data, VstInt32 byteSize, bool isPreset)
{
	if (isPreset) {
		if (byteSize != sizeof(Synthesizer::PatchInfo))return 0;
		this->synthesizer->programs[this->curProgram] = *(Synthesizer::PatchInfo*)data;
	} else {
		if (byteSize != sizeof(Synthesizer::PatchInfo) * SynthConsts::PROGRAM_NUMBER)return 0;
		for (int i = 0; i < SynthConsts::PROGRAM_NUMBER; i++)
		{
			this->synthesizer->programs[this->curProgram] = *(Synthesizer::PatchInfo*)data;
		}
	}
	return 1;
}

VstInt32 VstXSynth::getChunk(void** data, bool isPreset)
{
	if (isPreset) {
		*data = (void*)&this->synthesizer->programs[this->curProgram];
		return sizeof(Synthesizer::PatchInfo);
	} else {
		*data = (void*)&this->synthesizer->programs[0];
		return sizeof(Synthesizer::PatchInfo) * SynthConsts::PROGRAM_NUMBER;
	}
}

void VstXSynth::setSampleRate(float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);
	this->synthesizer->setSampleRate(sampleRate);
}

void VstXSynth::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
	VstMidiEvent* nextEvent;
	int restSample = sampleFrames;
	int processSample;
	int currentSample = 0;

	// set tempo
	VstTimeInfo *timeInfo = getTimeInfo(kVstTempoValid);
	if (timeInfo)
	{
		this->synthesizer->setTempo(timeInfo->tempo);
	}

	while (restSample > 0)
	{
		// process until next midi event comes
		if (this->currentVstEvent < this->vstEventNum)
		{
			nextEvent = (VstMidiEvent*)&this->vstEvents[this->currentVstEvent];
			processSample = std::max(0, std::min(restSample, nextEvent->deltaFrames - this->currentSample));
		} else {
			nextEvent = nullptr;
			processSample = restSample;

		}

		int rest = processSample;
		int size = std::min(SynthConsts::BUFFER_SIZE, rest);
		double* data[2];
		double databuf[2][SynthConsts::BUFFER_SIZE];
		data[0] = databuf[0];
		data[1] = databuf[1];

		while (rest > 0)
		{
			int size = std::min(SynthConsts::BUFFER_SIZE, rest);
			this->synthesizer->processStereoToStereo(size, nullptr, data);
			for	(int i = 0; i < size; i++)
			{
				outputs[0][currentSample] = (float)data[0][i];
				outputs[1][currentSample] = (float)data[1][i];
				if (outputs[0][currentSample] > 0.999) outputs[0][currentSample] = 0.999;
				if (outputs[1][currentSample] > 0.999) outputs[1][currentSample] = 0.999;
				currentSample++;
			}
			rest -= size;
		}
		restSample -= processSample;
		this->currentSample += processSample;

		// process a midi event
		if (nextEvent != nullptr)
		{
			this->synthesizer->processMidiEvent(nextEvent->midiData);
			this->currentVstEvent++;
		}
	}
}

VstInt32 VstXSynth::processEvents(VstEvents* ev)
{
	this->currentSample = 0;
	this->vstEventNum = 0;
	this->currentVstEvent = 0;

	if (ev->numEvents > 0)
	{
		if (this->vstEventMax < ev->numEvents) {
			// reallocate buffer
			if (this->vstEvents != nullptr) delete[] this->vstEvents;
			this->vstEventMax = ev->numEvents;
			this->vstEvents = new VstEvent[ev->numEvents];
		}

		for (int i = 0; i < ev->numEvents; i++)
		{
			// copy midi events except system messages
			if ((ev->events[i]->type == kVstMidiType) && (((VstMidiEvent*)ev->events[i])->midiData[0] & 0xf0) != 0xf0)
			{
				this->vstEvents[this->vstEventNum] = *(ev->events[i]);
				this->vstEventNum++;
			}
		}
	}
	return 1;
}