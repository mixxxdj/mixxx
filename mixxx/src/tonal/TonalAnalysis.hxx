
#ifndef TonalAnalysis_hxx
#define TonalAnalysis_hxx

#include "DataTypes.hxx"
#include "DiscontinuousSegmentation.hxx"
#include "OutPort.hxx"
#include "AudioInPort.hxx"
#include "Processing.hxx"
#include "Audio.hxx"
#include "ProcessingConfig.hxx"
#include <string>

namespace Simac { class ChordExtractor; }

namespace CLAM
{

class TonalAnalysisConfig : public ProcessingConfig
{
public:
	DYNAMIC_TYPE_USING_INTERFACE (TonalAnalysisConfig, 5, ProcessingConfig);
	DYN_ATTRIBUTE (0, public, double, FilterInertia);
	DYN_ATTRIBUTE (1, public, bool, TunningEnabled);
	DYN_ATTRIBUTE (2, public, bool, PeakWindowingEnabled);
	DYN_ATTRIBUTE (3, public, double, HopRatio);
	DYN_ATTRIBUTE (4, public, unsigned, SegmentationMethod);
protected:
	void DefaultInit(void);
};

class TonalAnalysis : public Processing
{
private:
	
	TonalAnalysisConfig _config;
	AudioInPort _input;
	OutPort<std::vector<CLAM::TData> > _pcp;
	OutPort<std::vector<CLAM::TData> > _chordCorrelation;
	OutPort<DiscontinuousSegmentation> _segmentation;
	OutPort<std::vector<std::pair<CLAM::TData, CLAM::TData> > > _chromaPeaks;
	OutPort<std::pair<CLAM::TData, CLAM::TData> > _tunning;

public:
	TonalAnalysis( const TonalAnalysisConfig & config = TonalAnalysisConfig() );

	bool Do();
	virtual ~TonalAnalysis();
	const char * GetClassName() const {return "TonalAnalysis";}
	
	inline const ProcessingConfig &GetConfig() const { return _config;}
	bool ConcreteConfigure(const ProcessingConfig& c);
	bool ConcreteStart();
	bool ConcreteStop();
private:
	Simac::ChordExtractor * _implementation;
	std::vector<float> _floatBuffer;
	CLAM::TData _currentTime;

};

} //namespace CLAM

#endif



