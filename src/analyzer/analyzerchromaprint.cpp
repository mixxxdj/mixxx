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

bool AnalyzerChromaprint::processSamples(const CSAMPLE* buffer, SINT count) {
    VERIFY_OR_DEBUG_ASSERT(m_pChromaprintCtx) {
        return false;
    }

    // Chromaprint expects interleaved signed 16-bit samples.
    // Resize the reusable buffer to fit the incoming chunk — no allocation
    // if the buffer is already large enough.
    if (static_cast<SINT>(m_int16Buffer.size()) < count) {
        m_int16Buffer.resize(count);
    }

    SampleUtil::convertFloat32ToS16(m_int16Buffer.data(), buffer, count);

    const int ret = chromaprint_feed(
            m_pChromaprintCtx,
            m_int16Buffer.data(),
            static_cast<int>(count));

    if (!ret) {
        kLogger.warning() << "AnalyzerChromaprint -> [processSamples] -> "
                             "chromaprint_feed() failed"
                          << "trackId:" << m_trackId
                          << "count:" << count;
        return false;
    }
    return true;
}

void AnalyzerChromaprint::storeResults(TrackPointer /*pTrack*/) {
    // initialize() returns false when skipping a track, which means
    // m_pChromaprintCtx is null — nothing to store.
    if (!m_pChromaprintCtx) {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [storeResults] -> "
                        "no context, skipping (track had valid fingerprint)";
        }
        return;
    }

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [storeResults] -> entry"
                 << "trackId:" << m_trackId;
    }

    // Finalise the fingerprint computation
    if (!chromaprint_finish(m_pChromaprintCtx)) {
        kLogger.warning() << "AnalyzerChromaprint -> [storeResults] -> "
                             "chromaprint_finish() failed"
                          << "trackId:" << m_trackId;
        return;
    }

    // Retrieve the raw uint32[] fingerprint array
    uint32_p fprint = nullptr;
    int size = 0;
    const int ret = chromaprint_get_raw_fingerprint(
            m_pChromaprintCtx, &fprint, &size);

    if (ret != 1 || !fprint || size <= 0) {
        kLogger.warning() << "AnalyzerChromaprint -> [storeResults] -> "
                             "chromaprint_get_raw_fingerprint() failed"
                          << "trackId:" << m_trackId
                          << "ret:" << ret
                          << "size:" << size;
        if (fprint) {
            chromaprint_dealloc(fprint);
        }
        return;
    }

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [storeResults] -> "
                    "raw fingerprint obtained"
                 << "trackId:" << m_trackId
                 << "uint32 values:" << size;
    }

    // Raw uint32[] array → .chroma binary file (section 2, Final_Database.md).
    //
    // This file is used for:
    //   1. SimHash pre-filter (Q1 phase 1) — computed from the array below
    //   2. Full XOR/popcount comparison (Q1 phase 2) — read from file on demand
    //   3. AcoustID Base64 submission — encoded in memory at submit time, discarded after
    //
    // The .chroma file is NEVER loaded at startup — only on demand for matching.
    const QByteArray rawData(
            reinterpret_cast<const char*>(fprint),
            size * static_cast<int>(sizeof(uint32_t)));
    chromaprint_dealloc(fprint);
    fprint = nullptr;

    // Derive both hashes from the raw fingerprint data (section 2).
    //
    // SimHash — 32-bit locality-sensitive hash, used as a fast non-unique
    // pre-filter. NOT a unique key: collisions are expected and handled by
    // full XOR/popcount comparison of the .chroma files.
    const quint32 simHash = computeSimHash(
            reinterpret_cast<const uint32_t*>(rawData.constData()), size);

    // SHA-256 — collision-resistant identity key.
    // Unique in cmrt_groups (one group = one canonical audio identity).
    // Non-unique in fingerprint_metadata (exact duplicate tracks share it).
    const QString sha256 = QCryptographicHash::hash(
            rawData, QCryptographicHash::Sha256)
                                   .toHex();

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [storeResults] -> hashes computed"
                 << "trackId:" << m_trackId
                 << "simHash:" << simHash
                 << "sha256:" << sha256;
    }

    // 1. Persist the raw fingerprint to disk as track_{id}.chroma
    if (!m_fingerprintDao.saveChromaFile(m_trackId, rawData)) {
        kLogger.warning() << "AnalyzerChromaprint -> [storeResults] -> "
                             "failed to save .chroma file"
                          << "trackId:" << m_trackId;
        return;
    }

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [storeResults] -> .chroma file saved"
                 << "trackId:" << m_trackId
                 << "bytes:" << rawData.size();
    }

    // 2. Write all fingerprint_metadata fields
    FingerprintMetadata metadata;
    metadata.trackId = m_trackId;
    metadata.fingerprintHash = simHash;
    metadata.chromaSha256 = sha256;
    // Full track duration in seconds (frameLength / sampleRate)
    metadata.fingerprintDuration =
            static_cast<double>(m_frameLength) / m_sampleRate;
    // Store the Chromaprint algorithm ID — identifies the version of the
    // fingerprinting algorithm, not just the library major version.
    metadata.fingerprintVersion = CHROMAPRINT_ALGORITHM_DEFAULT;
    // CMRT group fields are left at defaults until Q1 group assignment (future PR)
    metadata.cmrtGroupId = -1;
    metadata.cmrtOffsetSeconds = 0.0;
    metadata.isCanonical = false;
    metadata.fingerprintValid = true;
    metadata.fingerprintNeedsRegen = false;
    metadata.computedAt = QDateTime::currentDateTimeUtc();

    if (!m_fingerprintDao.saveFingerprintMetadata(metadata)) {
        kLogger.warning() << "AnalyzerChromaprint -> [storeResults] -> "
                             "failed to save fingerprint_metadata"
                          << "trackId:" << m_trackId;
        return;
    }

    if (sDebugAnalyzerChromaprint) {
        qDebug() << "AnalyzerChromaprint -> [storeResults] -> metadata saved"
                 << "trackId:" << m_trackId
                 << "duration:" << metadata.fingerprintDuration << "s"
                 << "version:" << metadata.fingerprintVersion;
    }

    // 3. Enqueue for AcoustID background lookup.
    // The UNIQUE constraint on acoustid_queue.track_id silently no-ops if
    // the track is already queued, so this is safe to call unconditionally.
    if (!m_fingerprintDao.enqueueAcoustId(m_trackId)) {
        kLogger.warning() << "AnalyzerChromaprint -> [storeResults] -> "
                             "failed to enqueue AcoustID job"
                          << "trackId:" << m_trackId;
    } else {
        if (sDebugAnalyzerChromaprint) {
            qDebug() << "AnalyzerChromaprint -> [storeResults] -> "
                        "enqueued for AcoustID lookup"
                     << "trackId:" << m_trackId;
        }
    }

    // TODO(CMRT): Q1 two-phase group assignment (SimHash pre-filter +
    // full XOR/popcount comparison of .chroma files) goes here in a
    // subsequent PR. Until then tracks are saved with cmrtGroupId = -1.

    kLogger.debug() << "Chromaprint fingerprint stored for track"
                    << m_trackId
                    << "simHash:" << simHash
                    << "duration:" << metadata.fingerprintDuration << "s"
                    << "rawBytes:" << rawData.size();
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
