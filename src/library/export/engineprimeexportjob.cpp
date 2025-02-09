#include "library/export/engineprimeexportjob.h"

#include <QHash>
#include <QStringList>
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include "library/export/engineprimeexportrequest.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/crate.h"
#include "moc_engineprimeexportjob.cpp"
#include "track/track.h"
#include "util/thread_affinity.h"
#include "waveform/waveformfactory.h"

namespace e = djinterop::engine;

namespace mixxx {

namespace {

const std::string kMixxxRootCrateName = "Mixxx";

constexpr int kMaxHotCues = 8;

constexpr uint8_t kDefaultWaveformOpacity = 127;

const QStringList kSupportedFileTypes = {
        "aac",
        "m4a",
        "aiff",
        "alac",
        "flac",
        "mp3",
        "mp4",
        "ogg",
        "wav"};

std::optional<djinterop::musical_key> toDjinteropKey(
        track::io::key::ChromaticKey key) {
    static const std::array<std::optional<djinterop::musical_key>, 25> keyMap{{
            std::nullopt,                          // INVALID = 0,
            djinterop::musical_key::c_major,       // C_MAJOR = 1,
            djinterop::musical_key::d_flat_major,  // D_FLAT_MAJOR = 2,
            djinterop::musical_key::d_major,       // D_MAJOR = 3,
            djinterop::musical_key::e_flat_major,  // E_FLAT_MAJOR = 4,
            djinterop::musical_key::e_major,       // E_MAJOR = 5,
            djinterop::musical_key::f_major,       // F_MAJOR = 6,
            djinterop::musical_key::f_sharp_major, // F_SHARP_MAJOR = 7,
            djinterop::musical_key::g_major,       // G_MAJOR = 8,
            djinterop::musical_key::a_flat_major,  // A_FLAT_MAJOR = 9,
            djinterop::musical_key::a_major,       // A_MAJOR = 10,
            djinterop::musical_key::b_flat_major,  // B_FLAT_MAJOR = 11,
            djinterop::musical_key::b_major,       // B_MAJOR = 12,
            djinterop::musical_key::c_minor,       // C_MINOR = 13,
            djinterop::musical_key::d_flat_minor,  // C_SHARP_MINOR = 14,
            djinterop::musical_key::d_minor,       // D_MINOR = 15,
            djinterop::musical_key::e_flat_minor,  // E_FLAT_MINOR = 16,
            djinterop::musical_key::e_minor,       // E_MINOR = 17,
            djinterop::musical_key::f_minor,       // F_MINOR = 18,
            djinterop::musical_key::f_sharp_minor, // F_SHARP_MINOR = 19,
            djinterop::musical_key::g_minor,       // G_MINOR = 20,
            djinterop::musical_key::a_flat_minor,  // G_SHARP_MINOR = 21,
            djinterop::musical_key::a_minor,       // A_MINOR = 22,
            djinterop::musical_key::b_flat_minor,  // B_FLAT_MINOR = 23,
            djinterop::musical_key::b_minor,       // B_MINOR = 24
    }};

    return keyMap[key];
}

QString exportFile(const QSharedPointer<EnginePrimeExportRequest> pRequest,
        TrackPointer pTrack) {
    if (!pRequest->engineLibraryDbDir.exists()) {
        const auto msg = QStringLiteral(
                "Engine Library DB directory %1 has been removed from disk!")
                                 .arg(pRequest->engineLibraryDbDir.absolutePath());
        throw std::runtime_error{msg.toStdString()};
    } else if (!pRequest->musicFilesDir.exists()) {
        const auto msg = QStringLiteral(
                "Music file export directory %1 has been removed from disk!")
                                 .arg(pRequest->musicFilesDir.absolutePath());
        throw std::runtime_error{msg.toStdString()};
    }

    // Copy music files into the Mixxx export dir, if the source file has
    // been modified (or the destination doesn't exist).  To ensure no
    // chance of filename clashes, and to keep things simple, we will prefix
    // the destination files with the DB track identifier.
    mixxx::FileInfo srcFileInfo = pTrack->getFileInfo();
    QString dstFilename = pTrack->getId().toString() + " - " + srcFileInfo.fileName();
    QString dstPath = pRequest->musicFilesDir.filePath(dstFilename);
    if (!QFile::exists(dstPath) ||
            srcFileInfo.lastModified() > QFileInfo{dstPath}.lastModified()) {
        const auto srcPath = srcFileInfo.location();
        QFile::copy(srcPath, dstPath);
    }

    return pRequest->engineLibraryDbDir.relativeFilePath(dstPath);
}

std::optional<djinterop::track> getTrackByRelativePath(
        djinterop::database* pDatabase, const QString& relativePath) {
    const auto trackCandidates = pDatabase->tracks_by_relative_path(relativePath.toStdString());
    switch (trackCandidates.size()) {
    case 0:
        return std::nullopt;
    case 1:
        return std::make_optional(trackCandidates.front());
    default:
        qWarning() << "More than one external track with the same relative path.";
        return trackCandidates.front();
    }
}

bool tryGetBeatgrid(BeatsPointer pBeats,
        mixxx::audio::FramePos cuePlayPos,
        int64_t frameCount,
        std::vector<djinterop::beatgrid_marker>* pBeatgrid) {
    if (!cuePlayPos.isValid()) {
        return false;
    }

    // For now, assume a constant average BPM across the whole track.
    // Note that Mixxx does not (currently) store any information about
    // which beat of a bar a given beat represents.  As such, in order to
    // make sure we have the right phrasing, assume that the main cue point
    // starts at the beginning of a bar, then move backwards towards the
    // beginning of the track in 4-beat decrements to find the first beat
    // in the track that also aligns with the start of a bar.
    const auto firstBeatPlayPos = pBeats->firstBeat();
    const auto cueBeatPlayPos = pBeats->findClosestBeat(cuePlayPos);
    if (!firstBeatPlayPos.isValid() || !cueBeatPlayPos.isValid()) {
        return false;
    }

    int numBeatsToCue = pBeats->numBeatsInRange(firstBeatPlayPos, cueBeatPlayPos);
    const auto firstBarAlignedBeatPlayPos = pBeats->findNBeatsFromPosition(
            cueBeatPlayPos, -1 * (numBeatsToCue & ~0x3));
    if (!firstBarAlignedBeatPlayPos.isValid()) {
        return false;
    }

    // We will treat the first bar-aligned beat as beat zero.  Find the
    // number of pBeats from there until the end of the track in order to
    // correctly assign an index for the last beat.
    const auto lastBeatPlayPos = pBeats->findPrevBeat(mixxx::audio::kStartFramePos + frameCount);
    if (!lastBeatPlayPos.isValid()) {
        return false;
    }

    int numBeats = pBeats->numBeatsInRange(firstBarAlignedBeatPlayPos, lastBeatPlayPos);
    if (numBeats <= 0) {
        return false;
    }

    std::vector<djinterop::beatgrid_marker> beatgrid{
            {0, firstBarAlignedBeatPlayPos.value()},
            {numBeats, lastBeatPlayPos.value()}};
    beatgrid = e::normalize_beatgrid(std::move(beatgrid), frameCount);
    pBeatgrid->assign(std::begin(beatgrid), std::end(beatgrid));
    return true;
}

void exportMetadata(
        djinterop::database* pDatabase,
        const e::engine_schema& dbSchemaVersion,
        QHash<TrackId, int64_t>* pMixxxToEnginePrimeTrackIdMap,
        TrackPointer pTrack,
        const Waveform* pWaveform,
        const QString& relativePath) {
    // Attempt to load the track in the database, using the relative path to
    // the music file.  If it exists already, take a snapshot of the track and
    // update it.  If it does not exist, we'll create a new snapshot.
    auto externalTrack = getTrackByRelativePath(pDatabase, relativePath);
    auto snapshot = externalTrack
            ? externalTrack->snapshot()
            : djinterop::track_snapshot{};
    snapshot.relative_path = relativePath.toStdString();

    snapshot.track_number = pTrack->getTrackNumber().toInt();
    if (snapshot.track_number == 0) {
        snapshot.track_number = std::nullopt;
    }

    snapshot.duration = std::chrono::milliseconds{
            static_cast<int64_t>(1000 * pTrack->getDuration())};
    snapshot.bpm = pTrack->getBpm();
    snapshot.year = pTrack->getYear().toInt();
    snapshot.title = pTrack->getTitle().toStdString();
    snapshot.artist = pTrack->getArtist().toStdString();
    snapshot.album = pTrack->getAlbum().toStdString();
    snapshot.genre = pTrack->getGenre().toStdString();
    snapshot.comment = pTrack->getComment().toStdString();
    snapshot.composer = pTrack->getComposer().toStdString();
    snapshot.key = toDjinteropKey(pTrack->getKey());
    snapshot.bitrate = pTrack->getBitrate();
    snapshot.rating = pTrack->getRating() * 20; // note rating is in range 0-100
    snapshot.file_bytes = pTrack->getFileInfo().sizeInBytes();

    // Frames used interchangeably with "samples" here.
    const auto frameCount = static_cast<int64_t>(pTrack->getDuration() * pTrack->getSampleRate());
    snapshot.sample_count = frameCount;
    snapshot.sample_rate = pTrack->getSampleRate();

    // Track loudness controls how the waveforms are scaled on Engine players.
    // However, getting it wrong and accidentally scaling a waveform beyond a sensible maximum
    // can result in no waveform being shown at all.  In order to be safe, no loudness information
    // is exported, resulting in waveforms being displayed as-is.
    snapshot.average_loudness = 0;

    // Set main cue-point.
    mixxx::audio::FramePos cuePlayPos = pTrack->getMainCuePosition();
    const auto cuePlayPosValue = cuePlayPos.isValid() ? cuePlayPos.value() : 0;
    snapshot.main_cue = cuePlayPosValue;

    // Fill in beat grid.
    BeatsPointer beats = pTrack->getBeats();
    if (beats != nullptr) {
        std::vector<djinterop::beatgrid_marker> beatgrid;
        if (tryGetBeatgrid(beats, cuePlayPos, frameCount, &beatgrid)) {
            snapshot.beatgrid = beatgrid;
        } else {
            qWarning() << "Beats data exists but is invalid for track"
                       << pTrack->getId() << "("
                       << pTrack->getFileInfo().fileName() << ")";
        }
    } else {
        qInfo() << "No beats data found for track" << pTrack->getId()
                << "(" << pTrack->getFileInfo().fileName() << ")";
    }

    // Note that any existing hot cues on the track are kept in place, if Mixxx
    // does not have a hot cue at that location.
    const auto cues = pTrack->getCuePoints();
    snapshot.hot_cues.resize(kMaxHotCues);
    for (const CuePointer& pCue : cues) {
        // We are only interested in hot cues.
        if (pCue->getType() != CueType::HotCue) {
            continue;
        }

        int hotCueIndex = pCue->getHotCue(); // Note: Mixxx uses 0-based.
        if (hotCueIndex < 0 || hotCueIndex >= kMaxHotCues) {
            qInfo() << "Skipping hot cue" << hotCueIndex
                    << "as the Engine Prime format only supports at most"
                    << kMaxHotCues << "hot cues.";
            continue;
        }

        if (!pCue->getPosition().isValid()) {
            qWarning() << "Hot cue" << hotCueIndex << "exists but is invalid for track"
                       << pTrack->getId() << "(" << pTrack->getFileInfo().fileName() << ")";
            continue;
        }

        QString label = pCue->getLabel();
        if (label == "") {
            label = QString("Cue %1").arg(hotCueIndex + 1);
        }

        djinterop::hot_cue hotCue{};
        hotCue.label = label.toStdString();
        hotCue.sample_offset = pCue->getPosition().value();

        auto color = mixxx::RgbColor::toQColor(pCue->getColor());
        hotCue.color = djinterop::pad_color{
                static_cast<uint_least8_t>(color.red()),
                static_cast<uint_least8_t>(color.green()),
                static_cast<uint_least8_t>(color.blue()),
                255};

        snapshot.hot_cues[hotCueIndex] = hotCue;
    }

    // TODO (mr-smidge): Export saved loops.

    // Write waveform.
    if (pWaveform) {
        djinterop::waveform_extents extents = dbSchemaVersion >=
                        djinterop::engine::engine_schema::schema_2_18_0
                ? e::calculate_overview_waveform_extents(
                          frameCount, pTrack->getSampleRate())
                : e::calculate_high_resolution_waveform_extents(
                          frameCount, pTrack->getSampleRate());
        std::vector<djinterop::waveform_entry> externalWaveform;
        externalWaveform.reserve(extents.size);
        for (uint64_t i = 0; i < extents.size; ++i) {
            uint64_t j = pWaveform->getDataSize() * i / extents.size;
            externalWaveform.push_back({{pWaveform->getLow(j), kDefaultWaveformOpacity},
                    {pWaveform->getMid(j), kDefaultWaveformOpacity},
                    {pWaveform->getHigh(j), kDefaultWaveformOpacity}});
        }
        snapshot.waveform = std::move(externalWaveform);
    } else {
        qInfo() << "No waveform data found for track" << pTrack->getId()
                << "(" << pTrack->getFileInfo().fileName() << ")";
    }

    int externalTrackId;
    if (externalTrack) {
        externalTrack->update(snapshot);
        externalTrackId = externalTrack->id();
    } else {
        auto newTrack = pDatabase->create_track(snapshot);
        externalTrackId = newTrack.id();
    }

    // Record the mapping from Mixxx track id to exported track id.
    pMixxxToEnginePrimeTrackIdMap->insert(pTrack->getId(), externalTrackId);
}

void exportTrack(
        const QSharedPointer<EnginePrimeExportRequest> pRequest,
        djinterop::database* pDatabase,
        const e::engine_schema& dbSchemaVersion,
        QHash<TrackId, int64_t>* pMixxxToEnginePrimeTrackIdMap,
        const TrackPointer pTrack,
        const Waveform* pWaveform) {
    // Only export supported file types.
    if (!kSupportedFileTypes.contains(pTrack->getType())) {
        qInfo() << "Skipping file" << pTrack->getFileInfo().fileName()
                << "(id" << pTrack->getId() << ") as its file type"
                << pTrack->getType() << "is not supported";
        return;
    }

    // Copy the file, if required.
    const auto musicFileRelativePath = exportFile(pRequest, pTrack);

    // Export meta-data.
    exportMetadata(pDatabase,
            dbSchemaVersion,
            pMixxxToEnginePrimeTrackIdMap,
            pTrack,
            pWaveform,
            musicFileRelativePath);
}

void exportCrate(
        djinterop::crate* pExtRootCrate,
        const QHash<TrackId, int64_t>& mixxxToEnginePrimeTrackIdMap,
        const Crate& crate,
        const QList<TrackId>& trackIds) {
    // Create a new crate as a sub-crate of the top-level Mixxx crate, if one
    // does not already exist.
    auto crateName = crate.getName().toStdString();
    const auto optionalExtCrate = pExtRootCrate->sub_crate_by_name(crateName);
    auto extCrate = optionalExtCrate
            ? *optionalExtCrate
            : pExtRootCrate->create_sub_crate(crateName);

    // Loop through all track ids in this crate and add.
    for (const auto& trackId : trackIds) {
        const auto extTrackId = mixxxToEnginePrimeTrackIdMap[trackId];
        extCrate.add_track(extTrackId);
    }
}

} // namespace

EnginePrimeExportJob::EnginePrimeExportJob(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        QSharedPointer<EnginePrimeExportRequest> pRequest)
        : QThread{parent},
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_pRequest{pRequest} {
    // Must be collocated with the TrackCollectionManager.
    if (parent != nullptr) {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_pTrackCollectionManager);
    } else {
        DEBUG_ASSERT(m_pTrackCollectionManager);
        moveToThread(m_pTrackCollectionManager->thread());
    }
}

// out-of-line declaration because we can't generate dtor in
// header with unique_ptr's of incomplete types.
EnginePrimeExportJob::~EnginePrimeExportJob() = default;

void EnginePrimeExportJob::loadIds(const QSet<CrateId>& crateIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_pTrackCollectionManager);

    if (crateIds.isEmpty()) {
        // No explicit crate ids specified, meaning we want to export the
        // whole library, plus all non-empty crates.  Start by building a list
        // of unique track refs from all directories in the library.
        qDebug() << "Loading all track refs and crate ids...";
        QSet<TrackRef> trackRefs;
        const auto dirInfos = m_pTrackCollectionManager->internalCollection()
                                      ->getDirectoryDAO()
                                      .loadAllDirectories();
        for (const mixxx::FileInfo& dirInfo : dirInfos) {
            const auto trackRefsFromDir = m_pTrackCollectionManager
                                                  ->internalCollection()
                                                  ->getTrackDAO()
                                                  .getAllTrackRefs(dirInfo.toQDir());
            for (const auto& trackRef : trackRefsFromDir) {
                trackRefs.insert(trackRef);
            }
        }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_trackRefs = QList<TrackRef>{trackRefs.begin(), trackRefs.end()};
#else
        m_trackRefs = trackRefs.toList();
#endif

        // Convert a list of track refs to a list of track ids, and use that
        // to identify all crates that contain those tracks.
        QList<TrackId> trackIds;
        for (const auto& trackRef : trackRefs) {
            trackIds.append(trackRef.getId());
        }
        auto crateIdsOfTracks = m_pTrackCollectionManager->internalCollection()
                                        ->crates()
                                        .collectCrateIdsOfTracks(trackIds);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_crateIds = QList<CrateId>{crateIdsOfTracks.begin(), crateIdsOfTracks.end()};
#else
        m_crateIds = crateIdsOfTracks.toList();
#endif
    } else {
        // Explicit crates have been specified to export.
        qDebug() << "Loading track refs from" << crateIds.size() << "crate(s)";
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_crateIds = QList<CrateId>{crateIds.begin(), crateIds.end()};
#else
        m_crateIds = crateIds.toList();
#endif

        // Identify track refs from the specified crates.
        m_trackRefs.clear();
        for (const auto& crateId : crateIds) {
            auto result = m_pTrackCollectionManager->internalCollection()
                                  ->crates()
                                  .selectCrateTracksSorted(crateId);
            while (result.next()) {
                const auto trackId = result.trackId();
                const auto location = m_pTrackCollectionManager->internalCollection()
                                              ->getTrackDAO()
                                              .getTrackLocation(trackId);
                m_trackRefs.append(TrackRef::fromFilePath(location, trackId));
            }
        }
    }
}

void EnginePrimeExportJob::loadTrack(const TrackRef& trackRef) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_pTrackCollectionManager);

    // Load the track.
    m_pLastLoadedTrack = m_pTrackCollectionManager->getOrAddTrack(trackRef);

    // Load high-resolution waveform from analysis info.
    auto& analysisDao = m_pTrackCollectionManager->internalCollection()->getAnalysisDAO();
    const auto waveformAnalyses = analysisDao.getAnalysesForTrackByType(
            m_pLastLoadedTrack->getId(), AnalysisDao::TYPE_WAVEFORM);
    if (!waveformAnalyses.isEmpty()) {
        const auto& waveformAnalysis = waveformAnalyses.first();
        m_pLastLoadedWaveform.reset(
                WaveformFactory::loadWaveformFromAnalysis(waveformAnalysis));
    } else {
        m_pLastLoadedWaveform.reset();
    }
}

void EnginePrimeExportJob::loadCrate(const CrateId& crateId) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(m_pTrackCollectionManager);

    // Load crate details.
    m_pTrackCollectionManager->internalCollection()->crates().readCrateById(
            crateId, &m_lastLoadedCrate);

    // Loop through all track ids in this crate and add to a list.
    auto result = m_pTrackCollectionManager->internalCollection()
                          ->crates()
                          .selectCrateTracksSorted(crateId);
    m_lastLoadedCrateTrackIds.clear();
    while (result.next()) {
        m_lastLoadedCrateTrackIds.append(result.trackId());
    }
}

void EnginePrimeExportJob::run() {
    // Crate music directory if it doesn't already exist.
    QDir().mkpath(m_pRequest->musicFilesDir.path());

    // Load ids of tracks and crates to export.
    // Note that loading must happen on the same thread as the track collection
    // manager, which is not the same as this method's worker thread.
    QMetaObject::invokeMethod(
            this,
            "loadIds",
            Qt::BlockingQueuedConnection,
            Q_ARG(QSet<CrateId>, m_pRequest->crateIdsToExport));

    // Measure progress as one 'count' for each track, each crate, plus some
    // additional counts for various other operations.
    int maxProgress = m_trackRefs.size() + m_crateIds.size() + 2;
    int currProgress = 0;
    emit jobMaximum(maxProgress);
    emit jobProgress(currProgress);

    // Ensure that the database exists, creating an empty one if not.
    std::unique_ptr<djinterop::database> pDb;
    e::engine_schema dbSchemaVersion;
    try {
        bool created;
        pDb = std::make_unique<djinterop::database>(e::create_or_load_database(
                m_pRequest->engineLibraryDbDir.path().toStdString(),
                m_pRequest->exportSchemaVersion,
                created,
                dbSchemaVersion));

        if (!created) {
            dbSchemaVersion = m_pRequest->exportSchemaVersion;
        }
    } catch (std::exception& e) {
        qWarning() << "Failed to create/load database:" << e.what();
        m_lastErrorMessage = e.what();
        emit failed(m_lastErrorMessage);
        return;
    }

    ++currProgress;
    emit jobProgress(currProgress);

    // We will build up a map from Mixxx track id to EL track id during export.
    QHash<TrackId, int64_t> mixxxToEnginePrimeTrackIdMap;

    for (const auto& trackRef : std::as_const(m_trackRefs)) {
        // Load each track.
        // Note that loading must happen on the same thread as the track collection
        // manager, which is not the same as this method's worker thread.
        QMetaObject::invokeMethod(
                this,
                "loadTrack",
                Qt::BlockingQueuedConnection,
                Q_ARG(TrackRef, trackRef));

        if (m_cancellationRequested.loadAcquire() != 0) {
            qInfo() << "Cancelling export";
            return;
        }

        DEBUG_ASSERT(m_pLastLoadedTrack != nullptr);

        qInfo() << "Exporting track" << m_pLastLoadedTrack->getId().toString()
                << "at" << m_pLastLoadedTrack->getFileInfo().location() << "...";
        try {
            exportTrack(m_pRequest,
                    pDb.get(),
                    dbSchemaVersion,
                    &mixxxToEnginePrimeTrackIdMap,
                    m_pLastLoadedTrack,
                    m_pLastLoadedWaveform.get());
        } catch (std::exception& e) {
            qWarning() << "Failed to export track"
                       << m_pLastLoadedTrack->getId().toString() << ":"
                       << e.what();
            //: %1 is the artist %2 is the title and %3 is the original error message
            m_lastErrorMessage = tr("Failed to export track %1 - %2:\n%3")
                                         .arg(m_pLastLoadedTrack->getArtist(),
                                                 m_pLastLoadedTrack->getTitle(),
                                                 e.what());
            emit failed(m_lastErrorMessage);
            return;
        }

        m_pLastLoadedTrack.reset();

        ++currProgress;
        emit jobProgress(currProgress);
    }

    // We will ensure that there is a special top-level crate representing the
    // root of all Mixxx-exported items.  Mixxx tracks and crates will exist
    // underneath this crate.
    std::unique_ptr<djinterop::crate> pExtRootCrate;
    try {
        const auto optionalExtRootCrate = pDb->root_crate_by_name(kMixxxRootCrateName);
        pExtRootCrate = std::make_unique<djinterop::crate>(optionalExtRootCrate
                        ? *optionalExtRootCrate
                        : pDb->create_root_crate(kMixxxRootCrateName));
    } catch (std::exception& e) {
        qWarning() << "Failed to create/identify root crate:" << e.what();
        m_lastErrorMessage = e.what();
        emit failed(m_lastErrorMessage);
        return;
    }

    // Add each track to the root crate, even if it also belongs to others.
    for (const TrackRef& trackRef : std::as_const(m_trackRefs)) {
        if (!mixxxToEnginePrimeTrackIdMap.contains(trackRef.getId())) {
            qInfo() << "Not adding track" << trackRef.getId()
                    << "to any crates, as it was not exported";
            continue;
        }

        const auto extTrackId = mixxxToEnginePrimeTrackIdMap.value(
                trackRef.getId());
        try {
            pExtRootCrate->add_track(extTrackId);
        } catch (std::exception& e) {
            qWarning() << "Failed to add track" << trackRef.getId()
                       << "to root crate:" << e.what();
            m_lastErrorMessage = e.what();
            emit failed(m_lastErrorMessage);
            return;
        }
    }

    ++currProgress;
    emit jobProgress(currProgress);

    // Export all Mixxx crates
    for (const CrateId& crateId : std::as_const(m_crateIds)) {
        // Load the current crate.
        // Note that loading must happen on the same thread as the track collection
        // manager, which is not the same as this method's worker thread.
        QMetaObject::invokeMethod(
                this,
                "loadCrate",
                Qt::BlockingQueuedConnection,
                Q_ARG(CrateId, crateId));

        if (m_cancellationRequested.loadAcquire() != 0) {
            qInfo() << "Cancelling export";
            return;
        }

        qInfo() << "Exporting crate" << m_lastLoadedCrate.getId().toString() << "...";
        try {
            exportCrate(
                    pExtRootCrate.get(),
                    mixxxToEnginePrimeTrackIdMap,
                    m_lastLoadedCrate,
                    m_lastLoadedCrateTrackIds);
        } catch (std::exception& e) {
            qWarning() << "Failed to add crate" << m_lastLoadedCrate.getId().toString()
                       << ":" << e.what();
            m_lastErrorMessage = e.what();
            emit failed(m_lastErrorMessage);
            return;
        }

        ++currProgress;
        emit jobProgress(currProgress);
    }

    qInfo() << "Engine Prime Export Job completed successfully";
    emit completed(m_trackRefs.size(), m_crateIds.size());
}

void EnginePrimeExportJob::slotCancel() {
    m_cancellationRequested = 1;
}

} // namespace mixxx
