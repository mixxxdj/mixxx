#ifdef __DJINTEROP__
#include "library/export/engineprimeexportjob.h"

#include <optional>
#include <stdexcept>
#include <QStringList>

#include <djinterop/djinterop.hpp>

#include "library/crate/crate.h"
#include "library/trackcollection.h"
#include "track/track.h"
#include "waveform/waveformfactory.h"

namespace el = djinterop::enginelibrary;

namespace mixxx {

namespace {

const std::string MixxxRootCrateName = "Mixxx";

const QStringList SupportedFileTypes = {"mp3", "flac", "ogg"};

std::optional<djinterop::musical_key> toDjinteropKey(
        mixxx::track::io::key::ChromaticKey key) {
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

    // TODO(mr-smidge) - Set average loudness.  Where does Mixxx store this?
    externalTrack.set_average_loudness(0.5f);

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
        if (pCue.get() == nullptr || pCue->getType() != CueType::HotCue) {
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
        int64_t externalWaveformSize = (sampleCount / samplesPerEntry) + 1;
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

    qInfo() << "Wrote all meta-data for track";
}

void exportTrack(TrackCollection& trackCollection,
        const EnginePrimeExportRequest& request,
        djinterop::database& db,
        QHash<TrackId, int64_t>& mixxxToEnginePrimeTrackIdMap,
        const TrackPointer pTrack) {
    // Load high-resolution waveform from analysis info.
    auto& analysisDao = trackCollection.getAnalysisDAO();
    std::unique_ptr<Waveform> pWaveform;
    auto waveformAnalyses = analysisDao.getAnalysesForTrackByType(
            pTrack->getId(), AnalysisDao::TYPE_WAVEFORM);
    if (!waveformAnalyses.isEmpty()) {
        auto& waveformAnalysis = waveformAnalyses.first();
        pWaveform.reset(
                WaveformFactory::loadWaveformFromAnalysis(waveformAnalysis));
    }

    // Only export supported file types.
    if (!SupportedFileTypes.contains(pTrack->getType())) {
        qInfo() << "Skipping file" << pTrack->getFileInfo().fileName()
            << "(id" << pTrack->getId() << ") as its file type"
            << pTrack->getType() << "is not supported";
        return;
    }

    // Copy the file, if required.
    auto musicFileRelativePath = exportFile(request, pTrack);

    // Export meta-data.
    exportMetadata(db, mixxxToEnginePrimeTrackIdMap, pTrack, std::move(pWaveform), musicFileRelativePath);
}

void exportCrate(TrackCollection& trackCollection,
        djinterop::crate& extRootCrate,
        QHash<TrackId, int64_t>& mixxxToEnginePrimeTrackIdMap,
        const CrateId& crateId) {
    // Load the crate (synchronously, as TrackCollection should not be accessed
    // in an unchecked parallel manner).
    Crate crate;
    trackCollection.crates().readCrateById(crateId, &crate);

    // Create a new crate as a sub-crate of the top-level Mixxx crate.
    auto extCrate = extRootCrate.create_sub_crate(crate.getName().toStdString());

    // Loop through all track ids in this crate and add.
    auto result = trackCollection.crates().selectCrateTracksSorted(crateId);
    while (result.next()) {
        auto extTrackId = mixxxToEnginePrimeTrackIdMap[result.trackId()];
        extCrate.add_track(extTrackId);
    }
}

} // namespace

EnginePrimeExportJob::EnginePrimeExportJob(QObject* parent,
        TrackCollectionManager& trackCollectionManager,
        TrackLoader& trackLoader,
        EnginePrimeExportRequest request)
        : QThread(parent), m_trackCollectionManager(trackCollectionManager), m_trackLoader(trackLoader), m_request{std::move(request)} {
    connect(&m_trackLoader, &TrackLoader::trackLoaded, this, &EnginePrimeExportJob::trackLoaded);
}

void EnginePrimeExportJob::trackLoaded(TrackRef trackRef, TrackPointer trackPtr) {
    // See if this track is in our queue.
    auto index = m_trackQueue.indexOf(trackRef);
    if (index == -1) {
        // Not a track we're interested in.
        return;
    }

    qInfo() << "Exporting track" << trackRef.getId().value() << "at"
            << trackRef.getLocation() << "...";
    exportTrack(*m_trackCollectionManager.internalCollection(), m_request, *m_pDb, m_mixxxToEnginePrimeTrackIdMap, trackPtr);

    // Removing this track from the queue and notify it has been exported.
    QMutexLocker lock{&m_trackMutex};
    m_trackQueue.removeAt(index);
    m_waitAnyTrack.wakeAll();
}

// Obtain a set of all track refs across all directories in the whole Mixxx library.
QSet<TrackRef> EnginePrimeExportJob::getAllTrackRefs() const {
    QSet<TrackRef> trackRefs;
    auto dirs = m_trackCollectionManager.internalCollection()
                        ->getDirectoryDAO()
                        .getDirs();
    for (auto iter = dirs.cbegin(); iter != dirs.cend(); ++iter) {
        trackRefs.unite(m_trackCollectionManager.internalCollection()
                                ->getTrackDAO()
                                .getAllTrackRefs(*iter)
                                .toSet());
    }
    return trackRefs;
}

// Obtain a set of track refs in a set of crates.
QSet<TrackRef> EnginePrimeExportJob::getTracksRefsInCrates(
        const QSet<CrateId>& crateIds) const {
    QSet<TrackRef> trackRefs;
    for (auto iter = crateIds.cbegin(); iter != crateIds.cend(); ++iter) {
        auto result = m_trackCollectionManager.internalCollection()
                              ->crates()
                              .selectCrateTracksSorted(*iter);
        while (result.next()) {
            auto trackId = result.trackId();
            auto location = m_trackCollectionManager.internalCollection()
                                    ->getTrackDAO()
                                    .getTrackLocation(trackId);
            auto trackFile = TrackFile(location);
            trackRefs.insert(TrackRef::fromFileInfo(trackFile, trackId));
        }
    }
    return trackRefs;
}

void EnginePrimeExportJob::run() {
    // Crate music directory if it doesn't already exist.
    QDir().mkpath(m_request.musicFilesDir.path());

    // Determine how many tracks and crates we have to export, and use to
    // calculate job progress.
    QSet<TrackRef> trackRefs;
    QSet<CrateId> crateIds;
    if (m_request.exportSelectedCrates) {
        trackRefs = getTracksRefsInCrates(m_request.crateIdsToExport);
        crateIds = m_request.crateIdsToExport;
    } else {
        // Note that we do not currently export empty crates.
        trackRefs = getAllTrackRefs();

        // Make a list of track ids, and get crates that contain those tracks.
        QList<TrackId> trackIds;
        for (auto iter = trackRefs.cbegin(); iter != trackRefs.cend(); ++iter) {
            trackIds.append(iter->getId());
        }
        crateIds = m_trackCollectionManager.internalCollection()
                           ->crates()
                           .collectCrateIdsOfTracks(trackIds);
    }

    // Measure progress as one 'count' for each track, each crate, plus an
    // additional count for setting up the database at the start.
    double maxProgress = trackRefs.size() + crateIds.size() + 1;
    double currProgress = 0;
    emit(jobMaximum(maxProgress));
    emit(jobProgress(currProgress));

    // Ensure that the database exists, creating an empty one if not.
    bool created;
    {
        auto db = el::create_or_load_database(
                m_request.engineLibraryDbDir.path().toStdString(),
                el::version_latest,
                created);
        m_pDb.reset(new djinterop::database{db});
    }
    ++currProgress;
    emit(jobProgress(currProgress));

    // We will build up a map from Mixxx track id to EL track id during export.
    m_mixxxToEnginePrimeTrackIdMap.clear();

    // Load all tracks (asynchronously).
    m_trackQueue = trackRefs.toList();
    for (auto iter = m_trackQueue.cbegin(); iter != m_trackQueue.cend(); ++iter) {
        m_trackLoader.invokeSlotLoadTrack(*iter);
    }

    // Run a consumer queue, waiting for tracks that have been loaded.
    int tracksDone = 0;
    while (!m_trackQueue.isEmpty()) {
        // Wait for a track to be loaded and exported.
        {
            QMutexLocker lock(&m_trackMutex);
            m_waitAnyTrack.wait(&m_trackMutex);
        }

        ++currProgress;
        ++tracksDone;
        qInfo() << "Track export progress" << tracksDone << "/" << trackRefs.size();
        emit(jobProgress(currProgress));
    }

    // We will ensure that there is a special top-level crate representing the
    // root of all Mixxx-exported items.  Mixxx tracks and crates will exist
    // underneath this crate.
    auto optionalExtRootCrate = m_pDb->root_crate_by_name(MixxxRootCrateName);
    auto extRootCrate = optionalExtRootCrate
            ? optionalExtRootCrate.value()
            : m_pDb->create_root_crate(MixxxRootCrateName);
    for (const TrackRef& trackRef : trackRefs) {
        // Add each track to the root crate, even if it also belongs to others.
        if (!m_mixxxToEnginePrimeTrackIdMap.contains(trackRef.getId())) {
            qInfo() << "Not adding track" << trackRef.getId()
                    << "to any crates, as it was not exported";
            continue;
        }

        auto extTrackId = m_mixxxToEnginePrimeTrackIdMap.value(
                trackRef.getId());
        extRootCrate.add_track(extTrackId);
    }

    // Export all Mixxx crates
    for (const CrateId& crateId : crateIds) {
        qInfo() << "Exporting crate" << crateId.value() << "...";
        exportCrate(*m_trackCollectionManager.internalCollection(),
                extRootCrate,
                m_mixxxToEnginePrimeTrackIdMap,
                crateId);

        ++currProgress;
        emit(jobProgress(currProgress));
    }

    qInfo() << "Engine Prime Export Job completed successfully";
}

void EnginePrimeExportJob::cancel()
{
    // TODO(mr-smidge): implement cancellation!
    qInfo() << "Would cancel...";
}

} // namespace mixxx
#endif
