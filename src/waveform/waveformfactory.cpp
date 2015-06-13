#include <QtDebug>

#include "waveform/waveformfactory.h"
#include "waveform/waveform.h"

// static
Waveform* WaveformFactory::loadWaveformFromAnalysis(
        const AnalysisDao::AnalysisInfo& analysis) {
    Waveform* pWaveform = new Waveform(analysis.data);
    pWaveform->setId(analysis.analysisId);
    pWaveform->setVersion(analysis.version);
    pWaveform->setDescription(analysis.description);
    return pWaveform;
}

// static
WaveformFactory::VersionClass WaveformFactory::waveformVersionToVersionClass(const QString& version) {
    if (version == WAVEFORM_CURRENT_VERSION) {
        // use, since is our version
        return VC_USE;
    }

    if (version == WAVEFORM_4_VERSION) {
        // Used in Mixxx 1.12 beta, suffers Bug lp:1406389
        return VC_REMOVE;
    }

    if (version == WAVEFORM_2_VERSION) {
        // keep for use with old Mixxx versions
        return VC_KEEP;
    }

    if (version == WAVEFORM_3_VERSION) {
        // Used in Mixxx 1.11 beta, suffers Bug lp:1087425
        return VC_REMOVE;
    }

    // possible a future version
    return VC_KEEP;
}

// static
WaveformFactory::VersionClass WaveformFactory::waveformSummaryVersionToVersionClass(const QString& version) {
    if (version == WAVEFORMSUMMARY_CURRENT_VERSION) {
        // use, since is our version
        return VC_USE;
    }

    if (version == WAVEFORMSUMMARY_4_VERSION) {
        // Used in Mixxx 1.12 beta, suffers Bug lp:1406389
        return VC_REMOVE;
    }

    if (version == WAVEFORMSUMMARY_2_VERSION) {
        // keep for use with old Mixxx versions
        return VC_KEEP;
    }

    if (version == WAVEFORMSUMMARY_3_VERSION) {
        // Used in Mixxx 1.11 beta, suffers Bug lp:1086965
        return VC_REMOVE;
    }

    // possible a future version
    return VC_KEEP;
}

// static
QString WaveformFactory::currentWaveformVersion() {
    return WAVEFORM_CURRENT_VERSION;
}

// static
QString WaveformFactory::currentWaveformSummaryVersion() {
    return WAVEFORMSUMMARY_CURRENT_VERSION;
}

// static
QString WaveformFactory::currentWaveformDescription() {
    return WAVEFORM_CURRENT_DESCRIPTION;
}

// static
QString WaveformFactory::currentWaveformSummaryDescription() {
    return WAVEFORMSUMMARY_CURRENT_DESCRIPTION;
}
