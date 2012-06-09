#pragma once
#include "common.h"

class WaveTable
{
public:
	~WaveTable(void);

	static const int HARMONICS_MAX = 735; // 735 = 22050 / 30

	static WaveTable* getWaveTable(void);
	double getWave(WaveForm waveForm, int phase, int harmonics);

private:
	static const int SAMPLE_SIZE = 1024;

	static WaveTable* waveTable;
	double sineWave[SAMPLE_SIZE];
	double triangleWave[(HARMONICS_MAX + 1) / 2][SAMPLE_SIZE];
	double sawWave[HARMONICS_MAX][SAMPLE_SIZE];
	double pulseWave[(HARMONICS_MAX + 1) / 2][SAMPLE_SIZE];

	WaveTable(void);
	WaveTable(const WaveTable& op);

	void generateSineWave(void);
	void generateTriangleWave(void);
	void generateSawWave(void);
	void generatePulseWave(void);
	double getSineWave(int phase, int harmonics);
	double getTriangleWave(int phase, int harmonics);
	double getSawWave(int phase, int harmonics);
	double getPulseWave(int phase, int harmonics);
	double getInterpolatedValue(double* table, int phase);
};
