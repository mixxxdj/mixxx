#include "waveform/waveformfactory.h"
#include "waveform/waveform.h"

namespace {
QString versionWithFrequencies(const QString& base, double lowMid, double midHigh) {
    return base + QStringLiteral(":") + QString::number(lowMid, 'g', 17) +
            QStringLiteral(":") + QString::number(midHigh, 'g', 17);
}
} // namespace

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
        const QString& version, double lowMidFrequency, double midHighFrequency) {
    if (version == currentWaveformVersion(lowMidFrequency, midHighFrequency)) {
        // use, since is our version
        return VC_USE;
    }

    if (version == WAVEFORM_4_VERSION || version == WAVEFORM_5_VERSION ||
            version == WAVEFORM_CURRENT_VERSION ||
            version.startsWith(QStringLiteral(WAVEFORM_CURRENT_VERSION ":"))
#ifdef __STEM__
            || version == WAVEFORM_6_VERSION
#endif
    ) {
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
    if (version == WAVEFORM_6_0_VERSION) {
        // Used in Mixxx 2.6 beta, introducing stem data but later replaced with
        // the signal scale removal
        return VC_REMOVE;
    }
#endif

    // possible a future version
    return VC_KEEP;
}

// static
WaveformFactory::VersionClass WaveformFactory::waveformSummaryVersionToVersionClass(
        const QString& version, double lowMidFrequency, double midHighFrequency) {
    if (version == currentWaveformSummaryVersion(lowMidFrequency, midHighFrequency)) {
        // use, since is our version
        return VC_USE;
    }

    if (version == WAVEFORMSUMMARY_4_VERSION || version == WAVEFORMSUMMARY_5_VERSION ||
            version == WAVEFORMSUMMARY_CURRENT_VERSION ||
            version.startsWith(QStringLiteral(WAVEFORMSUMMARY_CURRENT_VERSION ":"))) {
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
QString WaveformFactory::currentWaveformVersion(double lowMidFrequency, double midHighFrequency) {
    return versionWithFrequencies(QStringLiteral(WAVEFORM_CURRENT_VERSION),
            lowMidFrequency,
            midHighFrequency);
}

// static
QString WaveformFactory::currentWaveformSummaryVersion(
        double lowMidFrequency, double midHighFrequency) {
    return versionWithFrequencies(QStringLiteral(WAVEFORMSUMMARY_CURRENT_VERSION),
            lowMidFrequency,
            midHighFrequency);
}

// static
QString WaveformFactory::currentWaveformDescription() {
    return WAVEFORM_CURRENT_DESCRIPTION;
}

// static
QString WaveformFactory::currentWaveformSummaryDescription() {
    return WAVEFORMSUMMARY_CURRENT_DESCRIPTION;
}
