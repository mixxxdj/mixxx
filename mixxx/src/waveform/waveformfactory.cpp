#include <QtDebug>

#include "waveform/waveformfactory.h"
#include "waveform/waveform.h"

// static
Waveform* WaveformFactory::loadWaveformFromAnalysis(
    TrackPointer pTrack, const AnalysisDao::AnalysisInfo& analysis) {
    if (analysis.version == "Waveform-2.0" ||
        analysis.version == "WaveformSummary-2.0") {
        Waveform* pWaveform = new Waveform(analysis.data);
        pWaveform->setId(analysis.analysisId);
        pWaveform->setVersion(analysis.version);
        pWaveform->setDescription(analysis.description);
        return pWaveform;
    }
    qDebug() << "Skipping unsupported waveform version:" << analysis.version;
    return NULL;
}
