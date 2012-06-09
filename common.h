#pragma once
#pragma warning(disable:4996)
#include <string>

enum ParameterKind {
	MASTER_VOLUME,
	OSCILLATOR1_WAVEFORM,
	OSCILLATOR2_WAVEFORM,
	AMP_ATTACK_TIME,
	AMP_DECAY_TIME,
	AMP_SUSTAIN,
	AMP_RELEASE_TIME,
	OSCILLATOR_MIX,
	CUT_OFF_FREQUENCY,
	RESONANCE,
	PORTAMENT_TIME,
	VOICE_MODE,
	PORTAMENT_AUTO,
	DELAY_SWITCH,
	DELAY_TIME,
	DELAY_FEEDBACK,

	PARAMETER_KIND_NUMBER
};

enum VoiceMode {
	POLY,
	MONO,
	LEGATO,

	VOICE_MODE_NUMBER
};

enum EnvelopeState {
	IDLE,
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE
};

enum WaveForm {
	SINE,
	TRIANGLE,
	SAW,
	PULSE,
	NOISE,

	WAVE_FORM_NUMBER
};

static const double EPS = 1e-30;

namespace SynthConsts {

static const int PROGRAM_NUMBER = 128;
static const int PARAMETER_NUMBER = (int)PARAMETER_KIND_NUMBER;
static const int VOICE_CONTROLLER_SIZE = 32;
static const int BUFFER_SIZE = 288;
static const int VOICE_MODE_SIZE = (int)VOICE_MODE_NUMBER;
static const int WAVEFORM_SIZE = (int)WAVE_FORM_NUMBER;

}