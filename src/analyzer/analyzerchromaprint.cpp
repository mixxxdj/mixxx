#include "analyzer/analyzerchromaprint.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QtDebug>

#include "track/track.h"
#include "util/logger.h"
#include "util/sample.h"

namespace {

mixxx::Logger kLogger("AnalyzerChromaprint");

const bool sDebugAnalyzerChromaprint = true;

// Chromaprint API version guard — uint32_p changed from void* to uint32_t*
// in v1.4.0. Mirrors the same guard in src/musicbrainz/chromaprinter.cpp.
#if (CHROMAPRINT_VERSION_MINOR > 3) || (CHROMAPRINT_VERSION_MAJOR > 1)
typedef uint32_t* uint32_p;
#else
typedef void* uint32_p;
#endif

} // namespace

AnalyzerChromaprint::AnalyzerChromaprint(
        UserSettingsPointer pConfig,
        const QSqlDatabase& dbConnection)
        : m_fingerprintDao(pConfig),
          m_pChromaprintCtx(nullptr),
          m_sampleRate(0),
          m_channelCount(0),
          m_frameLength(0) {
    m_fingerprintDao.initialize(dbConnection);

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [constructor] -> initialized";
    }
}

AnalyzerChromaprint::~AnalyzerChromaprint() {
    // cleanup() is always called by the analyzer pipeline before destruction,
    // but guard here in case of early teardown.
    if (m_pChromaprintCtx) {
        chromaprint_free(m_pChromaprintCtx);
        m_pChromaprintCtx = nullptr;
    }
}

bool AnalyzerChromaprint::initialize(
        const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    const TrackPointer pTrack = track.getTrack();
    const TrackId trackId = pTrack->getId();

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [initialize] -> entry"
                 << "trackId:" << trackId
                 << "sampleRate:" << sampleRate
                 << "channelCount:" << channelCount
                 << "frameLength:" << frameLength;
    }

    if (!trackId.isValid()) {
        qDebug() << "AnalyzerChromaprint -> [initialize] -> "
                    "aborting: invalid trackId";
        return false;
    }

    // Check whether a valid, non-stale fingerprint already exists.
    // hasValidFingerprint() also detects file modifications and marks
    // the fingerprint stale if the source file has changed.
    if (hasValidFingerprint(pTrack)) {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [initialize] -> "
                        "skipping: valid fingerprint already stored"
                     << "trackId:" << trackId;
        }
        return false;
    }

    // Store signal properties for use in processSamples() and storeResults()
    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    m_frameLength = frameLength;
    m_trackId = trackId;

    // Create a fresh Chromaprint context for full-track analysis
    m_pChromaprintCtx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
    if (!m_pChromaprintCtx) {
        kLogger.warning() << "Failed to create Chromaprint context for track"
                          << trackId;
        return false;
    }

    // Unlike ChromaPrinter (which caps at 120s for AcoustID), we feed the
    // entire track so that the full fingerprint is available for CMRT matching.
    const int ret = chromaprint_start(
            m_pChromaprintCtx,
            sampleRate,
            channelCount);
    if (!ret) {
        kLogger.warning() << "chromaprint_start() failed for track" << trackId;
        chromaprint_free(m_pChromaprintCtx);
        m_pChromaprintCtx = nullptr;
        return false;
    }

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [initialize] -> "
                    "Chromaprint context ready"
                 << "trackId:" << trackId;
    }
    return true;
}

// private
bool AnalyzerChromaprint::hasValidFingerprint(TrackPointer pTrack) const {
    const TrackId trackId = pTrack->getId();

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [hasValidFingerprint] -> entry"
                 << "trackId:" << trackId;
    }

    auto pMetadata = m_fingerprintDao.getFingerprintMetadata(trackId);
    if (!pMetadata) {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [hasValidFingerprint] -> "
                        "no metadata row found"
                     << "trackId:" << trackId;
        }
        return false;
    }

    if (!pMetadata->fingerprintValid) {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [hasValidFingerprint] -> "
                        "fingerprint marked invalid"
                     << "trackId:" << trackId;
        }
        return false;
    }

    if (pMetadata->fingerprintNeedsRegen) {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [hasValidFingerprint] -> "
                        "fingerprint explicitly marked for regeneration"
                     << "trackId:" << trackId;
        }
        return false;
    }

    // Check whether the source file has been modified since the fingerprint
    // was computed. A newer mtime means the audio content may have changed
    // (re-encoded, re-tagged with padding shifts, replaced by another version).
    const QFileInfo fileInfo(pTrack->getLocation());
    if (fileInfo.exists() && pMetadata->computedAt.isValid() &&
            fileInfo.lastModified() > pMetadata->computedAt) {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [hasValidFingerprint] -> "
                        "source file modified after fingerprint computed — marking stale"
                     << "trackId:" << trackId
                     << "fileModified:" << fileInfo.lastModified().toString(Qt::ISODate)
                     << "computedAt:" << pMetadata->computedAt.toString(Qt::ISODate);
        }
        // Mark stale in the DB so the scanner also knows about it
        m_fingerprintDao.markFingerprintNeedsRegen(trackId);
        return false;
    }

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [hasValidFingerprint] -> "
                    "valid fingerprint found, skipping analysis"
                 << "trackId:" << trackId
                 << "computedAt:" << pMetadata->computedAt.toString(Qt::ISODate);
    }
    return true;
}
