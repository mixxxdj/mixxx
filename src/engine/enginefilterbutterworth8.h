#define MAX_COEFS 17
#define MAX_INTERNAL_BUF 16

#include "engine/enginefilter.h"

class EngineFilterButterworth8 : public EngineObject
{
public:
	EngineFilterButterworth8(filterType type, int sampleRate, double freqCorner1, double freqCorner2 = 0);
	~EngineFilterButterworth8();

	void process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize);

private:
	filterType m_type;

	double m_coef[MAX_COEFS];

	int m_bufSize;
	//channel 1 state
	double m_buf1[MAX_INTERNAL_BUF];

	//channel 2 state
	double m_buf2[MAX_INTERNAL_BUF];
};
