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
WaveformFactory::VersionClass WaveformFactory::waveformVersionToVersionClass(
        const QString& version,
        mixxx::WaveformAnalysisProfile profile) {
    if (version == currentWaveformVersion(profile)) {
        return VC_USE;
    }

    if (version == WAVEFORM_LEGACY_CURRENT_VERSION ||
            version == WAVEFORM_PERCEPTUAL_3BAND_VERSION ||
            version == WAVEFORM_5_VERSION) {
        return VC_REMOVE;
    }

    if (version == WAVEFORM_4_VERSION) {
        // Used in Mixxx 1.12 beta, suffers Bug #7776
        return VC_REMOVE;
    }

    if (version == WAVEFORM_2_VERSION) {
        // keep for use with old Mixxx versions
        return VC_KEEP;
    }

    if (version == WAVEFORM_3_VERSION) {
        // Used in Mixxx 1.11 beta, suffers Bug #6748
        return VC_REMOVE;
    }

#ifdef __STEM__
    if (version == WAVEFORM_6_VERSION || version == WAVEFORM_6_0_VERSION) {
        return VC_REMOVE;
    }
#endif

    // possible a future version
    return VC_KEEP;
}

// static
WaveformFactory::VersionClass WaveformFactory::waveformSummaryVersionToVersionClass(
        const QString& version,
        mixxx::WaveformAnalysisProfile profile) {
    if (version == currentWaveformSummaryVersion(profile)) {
        return VC_USE;
    }

    if (version == WAVEFORMSUMMARY_LEGACY_CURRENT_VERSION ||
            version == WAVEFORMSUMMARY_PERCEPTUAL_3BAND_VERSION) {
        return VC_REMOVE;
    }

    if (version == WAVEFORMSUMMARY_4_VERSION) {
        // Used in Mixxx 1.12 beta, suffers Bug #7776
        return VC_REMOVE;
    }

    if (version == WAVEFORMSUMMARY_2_VERSION) {
        // keep for use with old Mixxx versions
        return VC_KEEP;
    }

    if (version == WAVEFORMSUMMARY_3_VERSION) {
        // Used in Mixxx 1.11 beta, suffers Bug #6744
        return VC_REMOVE;
    }

    // possible a future version
    return VC_KEEP;
}

// static
QString WaveformFactory::currentWaveformVersion(
        mixxx::WaveformAnalysisProfile profile) {
    return profile == mixxx::WaveformAnalysisProfile::Perceptual3Band
            ? QStringLiteral(WAVEFORM_PERCEPTUAL_3BAND_VERSION)
            : QStringLiteral(WAVEFORM_LEGACY_CURRENT_VERSION);
}

// static
QString WaveformFactory::currentWaveformDescription(
        mixxx::WaveformAnalysisProfile profile) {
    return profile == mixxx::WaveformAnalysisProfile::Perceptual3Band
            ? QStringLiteral(WAVEFORM_PERCEPTUAL_3BAND_DESCRIPTION)
            : QStringLiteral(WAVEFORM_LEGACY_CURRENT_DESCRIPTION);
}

// static
QString WaveformFactory::currentWaveformSummaryVersion(
        mixxx::WaveformAnalysisProfile profile) {
    return profile == mixxx::WaveformAnalysisProfile::Perceptual3Band
            ? QStringLiteral(WAVEFORMSUMMARY_PERCEPTUAL_3BAND_VERSION)
            : QStringLiteral(WAVEFORMSUMMARY_LEGACY_CURRENT_VERSION);
}

// static
QString WaveformFactory::currentWaveformSummaryDescription(
        mixxx::WaveformAnalysisProfile profile) {
    return profile == mixxx::WaveformAnalysisProfile::Perceptual3Band
            ? QStringLiteral(WAVEFORMSUMMARY_PERCEPTUAL_3BAND_DESCRIPTION)
            : QStringLiteral(WAVEFORMSUMMARY_LEGACY_CURRENT_DESCRIPTION);
}
