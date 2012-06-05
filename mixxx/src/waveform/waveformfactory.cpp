#include <QtDebug>

#include "waveform/waveformfactory.h"
#include "waveform/waveform.h"

// static
Waveform* WaveformFactory::loadWaveformFromAnalysis(
    TrackPointer pTrack, const AnalysisDao::AnalysisInfo& analysis) {
    if (analysis.version == WAVEFORM_2_VERSION ||
        analysis.version == WAVEFORMSUMMARY_2_VERSION ||
        analysis.version == WAVEFORM_3_VERSION ||
        analysis.version == WAVEFORMSUMMARY_3_VERSION) {
        Waveform* pWaveform = new Waveform(analysis.data);
        pWaveform->setId(analysis.analysisId);
        pWaveform->setVersion(analysis.version);
        pWaveform->setDescription(analysis.description);
        return pWaveform;
    }
    qDebug() << "Skipping unsupported waveform version:" << analysis.version;
    return NULL;
}

// static
QString WaveformFactory::getPreferredWaveformVersion() {
    return WAVEFORM_3_VERSION;
}

// static
QString WaveformFactory::getPreferredWaveformSummaryVersion() {
    return WAVEFORMSUMMARY_3_VERSION;
}

// static
QString WaveformFactory::getPreferredWaveformDescription() {
    return WAVEFORM_3_DESCRIPTION;
}

// static
QString WaveformFactory::getPreferredWaveformSummaryDescription() {
    return WAVEFORMSUMMARY_3_DESCRIPTION;
}
