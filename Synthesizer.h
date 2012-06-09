#pragma once
#include "common.h"
#include "SignalProcessor.h"
#include "VoiceController.h"
#include "Filter.h"
#include "Delay.h"
#include <string>
#include <list>

class Synthesizer :
	public SignalProcessor
{
public:
	typedef struct _PatchInfo {
	char name[25];
	double parameter[SynthConsts::PARAMETER_NUMBER];
	} PatchInfo;

	PatchInfo programs[SynthConsts::PROGRAM_NUMBER];

	Synthesizer(void);
	~Synthesizer(void);

	virtual void processMonoralToStereo(int size, double** signalIn, double** signalOut);
	virtual void processStereoToStereo(int size, double** signalIn, double** signalOut);
	void setParameter(int index, double value);
	double getParameter(int index);
	void getParameterName(int index, char* buffer);
	void getParameterDisplay(int index, char* buffer);

	void setProgram(int index);
	virtual void changedSampleRate(void);
	void setTempo(double tempo);

	void processMidiEvent(char* ev);
	void receiveNoteOn(int noteNo, int velocity);
	void receiveNoteOff(int noteNo);
	void receivePitchBend(int value);
	void allNoteOff(void);

private:
	std::string parameterName[SynthConsts::PARAMETER_NUMBER];
	std::string waveFormName[SynthConsts::WAVEFORM_SIZE];
	std::string voiceModeName[SynthConsts::VOICE_MODE_SIZE];
	double parameterValue[SynthConsts::PARAMETER_NUMBER];
	int voiceCount;
	int voiceIndex;
	int curProgram;
	std::list<int> noteList;

	VoiceMode voiceMode;

	VoiceController* voiceController[SynthConsts::VOICE_CONTROLLER_SIZE];
	Delay* delay;
	double** monoralBuffer[SynthConsts::VOICE_CONTROLLER_SIZE];
	double tempo;
	double samplingRate;
	double masterVolume;
	double masterVolumeTo;
	double pitchBendSensitivity;

	bool delaySwitch;

	double calculateFrequency(int noteNo, double cent);

	double valueToMasterVolume(double value);
	WaveForm valueToWaveForm(double value);
	VoiceMode valueToVoiceMode(double value);
	double valueToTime(double value);
	double valueToAttackTime(double value);
	double valueToCutOffFrequency(double value);
	double valueToResonance(double value);
	bool valueToBool(double value);
};