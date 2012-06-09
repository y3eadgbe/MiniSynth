#pragma once

#include "Synthesizer.h"
#include "public.sdk/source/vst2.x/audioeffectx.h"

class VstXSynth : public AudioEffectX
{
public:
	VstXSynth (audioMasterCallback audioMaster);
	~VstXSynth ();

	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);

	virtual void setProgram (VstInt32 program);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);

	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);
	
	virtual void setSampleRate (float sampleRate);
	
	virtual VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset);
	virtual VstInt32 getChunk(void** data, bool isPreset);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstInt32 canDo (char* text);

private:

	Synthesizer* synthesizer;
	VstEvent* vstEvents;

	int vstEventMax;
	int vstEventNum;
	int currentVstEvent;
	int currentSample;
};