#include <QtDebug>

#include "waveform/waveformfactory.h"
#include "waveform/waveform.h"

// static
bool WaveformFactory::updateWaveformFromAnalysis(
        Waveform* pWaveform, const AnalysisDao::AnalysisInfo& analysis) {
    if (analysis.version == WAVEFORM_2_VERSION ||
        analysis.version == WAVEFORMSUMMARY_2_VERSION ||
        analysis.version == WAVEFORM_3_VERSION ||
        analysis.version == WAVEFORMSUMMARY_3_VERSION) {
        if (pWaveform) {
            pWaveform->reset();
            pWaveform->readByteArray(analysis.data);
            pWaveform->setId(analysis.analysisId);
            pWaveform->setVersion(analysis.version);
            pWaveform->setDescription(analysis.description);
            return true;
        }
    }
    qDebug() << "Skipping unsupported waveform version:" << analysis.version;
    return false;
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
