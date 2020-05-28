#include "library/export/engineprimeexportjob.h"

#include <QHash>
#include <QMetaMethod>
#include <QStringList>
#include <array>
#include <cstdint>
#include <djinterop/djinterop.hpp>
#include <memory>
#include <optional>
#include <stdexcept>

#include "library/trackcollection.h"
#include "library/trackset/crate/crate.h"
#include "track/track.h"
#include "waveform/waveformfactory.h"

namespace el = djinterop::enginelibrary;

namespace mixxx {

namespace {

const std::string kMixxxRootCrateName = "Mixxx";

const QStringList kSupportedFileTypes = {"mp3", "flac", "ogg"};

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

std::string exportFile(const EnginePrimeExportRequest& request,
        TrackPointer pTrack) {
    if (!request.engineLibraryDbDir.exists()) {
        auto msg = QString(
                "Engine Library DB directory %1 has been removed from disk!")
                           .arg(request.engineLibraryDbDir.absolutePath());
        throw std::runtime_error{msg.toStdString()};
    } else if (!request.musicFilesDir.exists()) {
        auto msg = QString(
                "Music file export directory %1 has been removed from disk!")
                           .arg(request.musicFilesDir.absolutePath());
        throw std::runtime_error{msg.toStdString()};
    }

    // Copy music files into the Mixxx export dir, if the source file has
    // been modified (or the destination doesn't exist).  To ensure no
    // chance of filename clashes, and to keep things simple, we will prefix
    // the destination files with the DB track identifier.
    TrackFile srcFileInfo = pTrack->getFileInfo();
    auto trackId = pTrack->getId().value();
    QString dstFilename = QString::number(trackId) + " - " + srcFileInfo.fileName();
    QString dstPath = request.musicFilesDir.filePath(dstFilename);
    if (!QFile::exists(dstPath) ||
            srcFileInfo.fileLastModified() > QFileInfo{dstPath}.lastModified()) {
        auto srcPath = srcFileInfo.location();
        QFile::copy(srcPath, dstPath);
    }

    return request.engineLibraryDbDir.relativeFilePath(dstPath).toStdString();
}

djinterop::track getTrackByRelativePath(
        djinterop::database database, const std::string& relativePath) {
    auto trackCandidates = database.tracks_by_relative_path(relativePath);
    switch (trackCandidates.size()) {
    case 0:
        return database.create_track(relativePath);
    case 1:
        return trackCandidates.front();
    default:
        qInfo() << "Warning: More than one external track with the same relative path";
        return trackCandidates.front();
    }
}

void exportMetadata(djinterop::database& db,
        QHash<TrackId, int64_t>& mixxxToEnginePrimeTrackIdMap,
        TrackPointer pTrack,
        std::unique_ptr<Waveform> pWaveform,
        const std::string& relativePath) {
    // Create or load the track in the database, using the relative path to
    // the music file.  We will record the mapping from Mixxx track id to
    // exported track id as well.
    auto externalTrack = getTrackByRelativePath(db, relativePath);
    mixxxToEnginePrimeTrackIdMap.insert(pTrack->getId(), externalTrack.id());

    // Note that the Engine Prime format has the scope for recording meta-data
    // about whether track was imported from an external database.  However,
    // that meta-data only extends as far as other Engine Prime databases,
    // which Mixxx is not.  So we do not set any import information on the
    // exported track.
    externalTrack.set_track_number(pTrack->getTrackNumber().toInt());
    externalTrack.set_bpm(pTrack->getBpm());
    externalTrack.set_year(pTrack->getYear().toInt());
    externalTrack.set_title(pTrack->getTitle().toStdString());
    externalTrack.set_artist(pTrack->getArtist().toStdString());
    externalTrack.set_album(pTrack->getAlbum().toStdString());
    externalTrack.set_genre(pTrack->getGenre().toStdString());
    externalTrack.set_comment(pTrack->getComment().toStdString());
    externalTrack.set_composer(pTrack->getComposer().toStdString());
    externalTrack.set_key(toDjinteropKey(pTrack->getKey()));
    int64_t lastModifiedMillisSinceEpoch =
            pTrack->getFileInfo().fileLastModified().toMSecsSinceEpoch();
    std::chrono::system_clock::time_point lastModifiedAt{
            std::chrono::milliseconds{lastModifiedMillisSinceEpoch}};
    externalTrack.set_last_modified_at(lastModifiedAt);
    externalTrack.set_bitrate(pTrack->getBitrate());

    // Frames used interchangeably with "samples" here.
    auto sampleCount = static_cast<int64_t>(pTrack->getDuration() * pTrack->getSampleRate());
    externalTrack.set_sampling({static_cast<double>(pTrack->getSampleRate()), sampleCount});

    // Set track loudness.
    // Note that the djinterop API method for setting loudness may be revised
    // in future, as more is discovered about the exact meaning of the loudness
    // field in the Engine Library format.  Make the assumption for now that
    // ReplayGain ratio is an appropriate value to set, which has been validated
    // by basic experimental testing.
    externalTrack.set_average_loudness(pTrack->getReplayGain().getRatio());

    // Set main cue-point.
    double cuePlayPos = pTrack->getCuePoint().getPosition();
    externalTrack.set_default_main_cue(cuePlayPos / 2);
    externalTrack.set_adjusted_main_cue(cuePlayPos / 2);

    // Fill in beat grid.  For now, assume a constant average BPM across
    // the whole track.  Note that points in the track are specified as
    // "play positions", which are twice the sample offset.
    BeatsPointer beats = pTrack->getBeats();
    if (beats != nullptr) {
        // Note that Mixxx does not (currently) store any information about
        // which beat of a bar a given beat represents.  As such, in order to
        // make sure we have the right phrasing, assume that the main cue point
        // starts at the beginning of a bar, then move backwards towards the
        // beginning of the track in 4-beat decrements to find the first beat
        // in the track that also aligns with the start of a bar.
        double firstBeatPlayPos = beats->findNextBeat(0);
        double cueBeatPlayPos = beats->findClosestBeat(cuePlayPos);
        int numBeatsToCue = beats->numBeatsInRange(firstBeatPlayPos, cueBeatPlayPos);
        double firstBarAlignedBeatPlayPos = beats->findNBeatsFromSample(
                cueBeatPlayPos, numBeatsToCue & ~0x3);

        // We will treat the first bar-aligned beat as beat zero.  Find the
        // number of beats from there until the end of the track in order to
        // correctly assign an index for the last beat.
        double lastBeatPlayPos = beats->findPrevBeat(sampleCount * 2);
        int numBeats = beats->numBeatsInRange(firstBarAlignedBeatPlayPos, lastBeatPlayPos);
        std::vector<djinterop::beatgrid_marker> beatgrid{
                {0, firstBarAlignedBeatPlayPos / 2}, {numBeats, lastBeatPlayPos / 2}};
        beatgrid = el::normalize_beatgrid(std::move(beatgrid), sampleCount);
        externalTrack.set_default_beatgrid(beatgrid);
        externalTrack.set_adjusted_beatgrid(beatgrid);
    } else {
        qInfo() << "No beats data found for track" << pTrack->getId()
                << "(" << pTrack->getFileInfo().fileName() << ")";
    }

    auto cues = pTrack->getCuePoints();
    for (const CuePointer& pCue : cues) {
        // We are only interested in hot cues.
        if (pCue->getType() != CueType::HotCue) {
            continue;
        }

        int hot_cue_index = pCue->getHotCue(); // Note: Mixxx uses 0-based.
        if (hot_cue_index < 0 || hot_cue_index >= 8) {
            // Only supports a maximum of 8 hot cues.
            qInfo() << "Skipping hot cue" << hot_cue_index
                    << "as the Engine Prime format only supports at most 8"
                    << "hot cues.";
            continue;
        }

        QString label = pCue->getLabel();
        if (label == "") {
            label = QString("Cue %1").arg(hot_cue_index + 1);
        }

        djinterop::hot_cue hc{};
        hc.label = label.toStdString();
        hc.sample_offset = pCue->getPosition() / 2; // Convert "play pos".
        hc.color = el::standard_pad_colors::pads[hot_cue_index];
        externalTrack.set_hot_cue_at(hot_cue_index, hc);
    }

    // Note that Mixxx does not support pre-calculated stored loops, but it will
    // remember the position of a single ad-hoc loop between track loads.
    // However, since this single ad-hoc loop is likely to be different in use
    // from a set of stored loops (and is easily overwritten), we do not export
    // it to the external database here.
    externalTrack.set_loops({});

    // Write waveform.
    // Note that writing a single waveform will automatically calculate an
    // overview waveform too.
    if (pWaveform) {
        int64_t samplesPerEntry = externalTrack.required_waveform_samples_per_entry();
        int64_t externalWaveformSize = (sampleCount + samplesPerEntry - 1) / samplesPerEntry;
        std::vector<djinterop::waveform_entry> externalWaveform;
        externalWaveform.reserve(externalWaveformSize);
        for (int64_t i = 0; i < externalWaveformSize; ++i) {
            auto j = pWaveform->getDataSize() * i / externalWaveformSize;
            externalWaveform.push_back({{pWaveform->getLow(j), 127},
                    {pWaveform->getMid(j), 127},
                    {pWaveform->getHigh(j), 127}});
        }
        externalTrack.set_waveform(std::move(externalWaveform));
    } else {
        qInfo() << "No waveform data found for track" << pTrack->getId()
                << "(" << pTrack->getFileInfo().fileName() << ")";
    }
}

void exportTrack(
        const EnginePrimeExportRequest& request,
        djinterop::database& db,
        QHash<TrackId, int64_t>& mixxxToEnginePrimeTrackIdMap,
        const TrackPointer pTrack,
        std::unique_ptr<Waveform> pWaveform) {
    // Only export supported file types.
    if (!kSupportedFileTypes.contains(pTrack->getType())) {
        qInfo() << "Skipping file" << pTrack->getFileInfo().fileName()
                << "(id" << pTrack->getId() << ") as its file type"
                << pTrack->getType() << "is not supported";
        return;
    }

    // Copy the file, if required.
    auto musicFileRelativePath = exportFile(request, pTrack);

    // Export meta-data.
    exportMetadata(db,
            mixxxToEnginePrimeTrackIdMap,
            pTrack,
            std::move(pWaveform),
            musicFileRelativePath);
}

void exportCrate(
        djinterop::crate& extRootCrate,
        QHash<TrackId, int64_t>& mixxxToEnginePrimeTrackIdMap,
        const Crate& crate,
        const QList<TrackId>& trackIds) {
    // Create a new crate as a sub-crate of the top-level Mixxx crate.
    auto extCrate = extRootCrate.create_sub_crate(crate.getName().toStdString());

    // Loop through all track ids in this crate and add.
    for (auto iter = trackIds.cbegin(); iter != trackIds.cend(); ++iter) {
        auto trackId = *iter;
        auto extTrackId = mixxxToEnginePrimeTrackIdMap[trackId];
        extCrate.add_track(extTrackId);
    }
}

} // namespace

EnginePrimeExportJob::EnginePrimeExportJob(
        QObject* parent,
        TrackCollectionManager* pTrackCollectionManager,
        EnginePrimeExportRequest request)
        : QThread(parent),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_request{std::move(request)} {
    // Must be collocated with the TrackCollectionManager.
    DEBUG_ASSERT(m_pTrackCollectionManager != nullptr);
    moveToThread(m_pTrackCollectionManager->thread());
}

void EnginePrimeExportJob::loadIds(QSet<CrateId> crateIds) {
    // Note: this slot exists to ensure the track collection is never accessed
    // from outside its own thread.
    DEBUG_ASSERT(thread() == m_pTrackCollectionManager->thread());
    QMutexLocker lock{&m_mainThreadLoadMutex};

    if (crateIds.isEmpty()) {
        // No explicit crate ids specified, meaning we want to export the
        // whole library, plus all non-empty crates.  Start by building a list
        // of unique track refs from all directories in the library.
        qDebug() << "Loading all track refs and crate ids...";
        QSet<TrackRef> trackRefs;
        auto dirs = m_pTrackCollectionManager->internalCollection()
                            ->getDirectoryDAO()
                            .getDirs();
        for (auto& dir : dirs) {
            trackRefs.unite(m_pTrackCollectionManager->internalCollection()
                                    ->getTrackDAO()
                                    .getAllTrackRefs(dir)
                                    .toSet());
        }

        m_trackRefs = trackRefs.toList();

        // Convert a list of track refs to a list of track ids, and use that
        // to identify all crates that contain those tracks.
        QList<TrackId> trackIds;
        for (auto& trackRef : trackRefs) {
            trackIds.append(trackRef.getId());
        }
        m_crateIds = m_pTrackCollectionManager->internalCollection()
                             ->crates()
                             .collectCrateIdsOfTracks(trackIds)
                             .toList();
    } else {
        // Explicit crates have been specified to export.
        qDebug() << "Loading track refs from" << crateIds.size() << "crate(s)";
        m_crateIds = crateIds.toList();

        // Identify track refs from the specified crates.
        m_trackRefs.clear();
        for (auto& crateId : crateIds) {
            auto result = m_pTrackCollectionManager->internalCollection()
                                  ->crates()
                                  .selectCrateTracksSorted(crateId);
            while (result.next()) {
                auto trackId = result.trackId();
                auto location = m_pTrackCollectionManager->internalCollection()
                                        ->getTrackDAO()
                                        .getTrackLocation(trackId);
                auto trackFile = TrackFile(location);
                m_trackRefs.append(TrackRef::fromFileInfo(trackFile, trackId));
            }
        }
    }

    // Inform the worker thread that some main-thread loading has completed.
    m_waitForMainThreadLoad.wakeAll();
}

void EnginePrimeExportJob::loadTrack(TrackRef trackRef) {
    // Note: this slot exists to ensure the track collection is never accessed
    // from outside its own thread.
    DEBUG_ASSERT(thread() == m_pTrackCollectionManager->thread());
    QMutexLocker lock{&m_mainThreadLoadMutex};

    // Load the track.
    m_pLastLoadedTrack = m_pTrackCollectionManager->getOrAddTrack(trackRef);

    // Load high-resolution waveform from analysis info.
    auto& analysisDao = m_pTrackCollectionManager->internalCollection()->getAnalysisDAO();
    auto waveformAnalyses = analysisDao.getAnalysesForTrackByType(
            m_pLastLoadedTrack->getId(), AnalysisDao::TYPE_WAVEFORM);
    if (!waveformAnalyses.isEmpty()) {
        auto& waveformAnalysis = waveformAnalyses.first();
        m_pLastLoadedWaveform.reset(
                WaveformFactory::loadWaveformFromAnalysis(waveformAnalysis));
    }

    // Inform the worker thread that some main-thread loading has completed.
    m_waitForMainThreadLoad.wakeAll();
}

void EnginePrimeExportJob::loadCrate(CrateId crateId) {
    // Note: this slot exists to ensure the track collection is never accessed
    // from outside its own thread.
    DEBUG_ASSERT(thread() == m_pTrackCollectionManager->thread());
    QMutexLocker lock{&m_mainThreadLoadMutex};

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

    // Inform the worker thread that some main-thread loading has completed.
    m_waitForMainThreadLoad.wakeAll();
}

void EnginePrimeExportJob::run() {
    // Crate music directory if it doesn't already exist.
    QDir().mkpath(m_request.musicFilesDir.path());

    // Load ids of tracks and crates to export.
    // Note that loading must happen on the same thread as the track collection
    // manager, which is not the same as this method's worker thread.
    {
        QMutexLocker lock{&m_mainThreadLoadMutex};
        QMetaObject::invokeMethod(
                this,
                "loadIds",
                Q_ARG(QSet<CrateId>, m_request.crateIdsToExport));

        // We expect the `loadIds()` method to fire the below wait condition.
        m_waitForMainThreadLoad.wait(&m_mainThreadLoadMutex);
    }

    // Measure progress as one 'count' for each track, each crate, plus some
    // additional counts for various other operations.
    double maxProgress = m_trackRefs.size() + m_crateIds.size() + 2;
    double currProgress = 0;
    emit jobMaximum(maxProgress);
    emit jobProgress(currProgress);

    // Ensure that the database exists, creating an empty one if not.
    bool created;
    djinterop::database db = el::create_or_load_database(
            m_request.engineLibraryDbDir.path().toStdString(),
            el::version_latest,
            created);
    ++currProgress;
    emit jobProgress(currProgress);

    // We will build up a map from Mixxx track id to EL track id during export.
    QHash<TrackId, int64_t> mixxxToEnginePrimeTrackIdMap;

    for (auto& trackRef : m_trackRefs) {
        // Load each track.
        // Note that loading must happen on the same thread as the track collection
        // manager, which is not the same as this method's worker thread.
        {
            QMutexLocker lock{&m_mainThreadLoadMutex};
            QMetaObject::invokeMethod(
                    this,
                    "loadTrack",
                    Q_ARG(TrackRef, trackRef));

            // We expect the `loadTrack()` method to fire the below wait condition.
            m_waitForMainThreadLoad.wait(&m_mainThreadLoadMutex);
        }

        if (m_cancellationRequested.loadAcquire() != 0) {
            qInfo() << "Cancelling export";
            return;
        }

        DEBUG_ASSERT(m_pLastLoadedTrack != nullptr);

        qInfo() << "Exporting track" << m_pLastLoadedTrack->getId().value()
                << "at" << m_pLastLoadedTrack->getFileInfo().location() << "...";
        exportTrack(m_request,
                db,
                mixxxToEnginePrimeTrackIdMap,
                m_pLastLoadedTrack,
                std::move(m_pLastLoadedWaveform));
        m_pLastLoadedTrack.reset();

        ++currProgress;
        emit jobProgress(currProgress);
    }

    // We will ensure that there is a special top-level crate representing the
    // root of all Mixxx-exported items.  Mixxx tracks and crates will exist
    // underneath this crate.
    auto optionalExtRootCrate = db.root_crate_by_name(kMixxxRootCrateName);
    auto extRootCrate = optionalExtRootCrate
            ? optionalExtRootCrate.value()
            : db.create_root_crate(kMixxxRootCrateName);
    for (const TrackRef& trackRef : m_trackRefs) {
        // Add each track to the root crate, even if it also belongs to others.
        if (!mixxxToEnginePrimeTrackIdMap.contains(trackRef.getId())) {
            qInfo() << "Not adding track" << trackRef.getId()
                    << "to any crates, as it was not exported";
            continue;
        }

        auto extTrackId = mixxxToEnginePrimeTrackIdMap.value(
                trackRef.getId());
        extRootCrate.add_track(extTrackId);
    }

    ++currProgress;
    emit jobProgress(currProgress);

    // Export all Mixxx crates
    for (const CrateId& crateId : m_crateIds) {
        // Load the current crate.
        // Note that loading must happen on the same thread as the track collection
        // manager, which is not the same as this method's worker thread.
        {
            QMutexLocker lock{&m_mainThreadLoadMutex};
            QMetaObject::invokeMethod(
                    this,
                    "loadCrate",
                    Q_ARG(CrateId, crateId));

            // We expect the `loadCrate()` method to fire the below wait condition.
            m_waitForMainThreadLoad.wait(&m_mainThreadLoadMutex);
        }

        if (m_cancellationRequested.loadAcquire() != 0) {
            qInfo() << "Cancelling export";
            return;
        }

        qInfo() << "Exporting crate" << m_lastLoadedCrate.getId().value() << "...";
        exportCrate(
                extRootCrate,
                mixxxToEnginePrimeTrackIdMap,
                m_lastLoadedCrate,
                m_lastLoadedCrateTrackIds);

        ++currProgress;
        emit jobProgress(currProgress);
    }

    qInfo() << "Engine Prime Export Job completed successfully";
}

void EnginePrimeExportJob::cancel() {
    m_cancellationRequested = 1;
}

} // namespace mixxx
