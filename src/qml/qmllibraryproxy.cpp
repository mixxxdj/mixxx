#include "qml/qmllibraryproxy.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QLocale>
#include <QQmlEngine>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <cmath>

#include "control/controlobject.h"
#include "library/autodj/harmonicmixing.h"
#include "library/dao/analysisdao.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#ifdef __ENGINEPRIME__
#include "library/export/libraryexporter.h"
#endif
#include "moc_qmllibraryproxy.cpp"
#include "preferences/colorpalettesettings.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmllibrarytracklistmodel.h"
#include "qmltrackproxy.h"
#include "track/cue.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/db/dbconnectionpooled.h"

namespace mixxx {
namespace qml {

namespace {
const ConfigKey kHotcueDefaultColorIndexConfigKey("[Controls]", "HotcueDefaultColorIndex");
const ConfigKey kLoopDefaultColorIndexConfigKey("[Controls]", "LoopDefaultColorIndex");
const ConfigKey kJumpDefaultColorIndexConfigKey("[Controls]", "jump_default_color_index");

// Read by Auto DJ too, when it looks for the next harmonically compatible track.
const QString kAutoDjGroup = QStringLiteral("[Auto DJ]");
const QString kHarmonicBpmToleranceKey = QStringLiteral("HarmonicBpmTolerance");

constexpr mixxx::audio::FrameDiff_t kMinimumAudibleLoopSizeFrames = 150;

CuePointer findDeckHotcue(QmlTrackProxy* track, int hotcueNumber) {
    if (!track || !track->internal() || hotcueNumber <= 0) {
        return {};
    }
    return track->internal()->findHotcueByIndex(hotcueNumber - 1);
}

int defaultColorIndexForType(UserSettingsPointer pConfig, mixxx::CueType cueType) {
    switch (cueType) {
    case mixxx::CueType::Loop:
        return pConfig->getValue(kLoopDefaultColorIndexConfigKey, -1);
    case mixxx::CueType::Jump:
        return pConfig->getValue(kJumpDefaultColorIndexConfigKey, -1);
    default:
        return pConfig->getValue(kHotcueDefaultColorIndexConfigKey, -1);
    }
}

mixxx::RgbColor defaultColorForType(
        UserSettingsPointer pConfig,
        const ColorPalette& palette,
        mixxx::CueType cueType) {
    const int colorIndex = defaultColorIndexForType(pConfig, cueType);
    return (colorIndex < 0 || colorIndex >= palette.size())
            ? palette.defaultColor()
            : palette.at(colorIndex);
}

void updateCueTypeAndColorIfDefault(
        UserSettingsPointer pConfig,
        const CuePointer& pCue,
        mixxx::CueType newType) {
    VERIFY_OR_DEBUG_ASSERT(pConfig && pCue) {
        return;
    }

    ColorPaletteSettings colorPaletteSettings(pConfig);
    const ColorPalette palette = colorPaletteSettings.getHotcueColorPalette();
    const mixxx::RgbColor oldDefaultColor =
            defaultColorForType(pConfig, palette, pCue->getType());
    const bool cueUsesOldDefaultColor = pCue->getColor() == oldDefaultColor;

    pCue->setType(newType);
    if (cueUsesOldDefaultColor) {
        pCue->setColor(defaultColorForType(pConfig, palette, newType));
    }
}

mixxx::audio::FramePos getCurrentPlayPositionWithQuantize(
        const TrackPointer& pTrack,
        const QString& group) {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return mixxx::audio::kInvalidFramePos;
    }

    const double trackSamples = ControlObject::get(
            ConfigKey(group, QStringLiteral("track_samples")));
    auto position = mixxx::audio::FramePos::fromEngineSamplePos(
            ControlObject::get(ConfigKey(group, QStringLiteral("playposition"))) *
            trackSamples);
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (ControlObject::get(ConfigKey(group, QStringLiteral("quantize"))) > 0 && pBeats) {
        mixxx::audio::FramePos nextBeatPosition;
        mixxx::audio::FramePos prevBeatPosition;
        pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
        return (nextBeatPosition - position > position - prevBeatPosition)
                ? prevBeatPosition
                : nextBeatPosition;
    }
    return position;
}
} // namespace

QmlLibraryScannerProxy::QmlLibraryScannerProxy(LibraryScanner* libraryScanner,
        TrackCollectionManager* trackCollectionManager,
        QObject* parent)
        : QObject(parent),
          m_pLibraryScanner(libraryScanner),
          m_running(trackCollectionManager->isLibraryScanActive()),
          m_cancelling(false) {
    connect(libraryScanner,
            &LibraryScanner::progressLoading,
            this,
            &QmlLibraryScannerProxy::progress);
    connect(libraryScanner,
            &LibraryScanner::scanStarted,
            this,
            &QmlLibraryScannerProxy::started);
    connect(libraryScanner,
            &LibraryScanner::scanFinished,
            this,
            &QmlLibraryScannerProxy::finished);
    connect(this,
            &QmlLibraryScannerProxy::requestCancel,
            libraryScanner,
            &LibraryScanner::slotCancel);

    // Properties
    connect(libraryScanner,
            &LibraryScanner::scanStarted,
            this,
            [this]() {
                m_cancelling = false;
                m_running = true;
                emit stateChanged();
            });
    connect(libraryScanner,
            &LibraryScanner::scanFinished,
            this,
            [this]() {
                m_cancelling = false;
                m_running = false;
                emit stateChanged();
            });
}

QmlLibraryProxy::QmlLibraryProxy(QObject* parent)
        : QObject(parent),
          m_pModelProperty(new QmlLibraryTrackListModel(
                  QList<QmlLibraryTrackListColumn*>{}, s_pLibrary->trackTableModel(), this)),
          m_pScanner(new QmlLibraryScannerProxy(
                  s_pLibrary->trackCollectionManager()->scanner(),
                  s_pLibrary->trackCollectionManager(),
                  this)),
          m_pTrackCollectionManager(s_pLibrary->trackCollectionManager()) {
    connect(m_pScanner,
            &QmlLibraryScannerProxy::stateChanged,
            this,
            &QmlLibraryProxy::libraryScanActiveChanged);
    TrackCollectionManager* pTrackCollectionManager =
            s_pLibrary->trackCollectionManager();
    VERIFY_OR_DEBUG_ASSERT(pTrackCollectionManager) {
        return;
    }
    connect(pTrackCollectionManager,
            &TrackCollectionManager::libraryScanSummary,
            this,
            [this](const LibraryScanResultSummary&) {
                deliverPendingLibraryScanSummary();
            });
    deliverPendingLibraryScanSummary();
#ifdef __ENGINEPRIME__
    m_pLibraryExporter = s_pLibrary->makeLibraryExporter(nullptr);
    connect(s_pLibrary.get(),
            &Library::exportLibrary,
            m_pLibraryExporter.get(),
            &mixxx::LibraryExporter::slotRequestExport);
    connect(s_pLibrary.get(),
            &Library::exportCrate,
            m_pLibraryExporter.get(),
            &mixxx::LibraryExporter::slotRequestExportWithInitialCrate);
    connect(s_pLibrary.get(),
            &Library::exportPlaylist,
            m_pLibraryExporter.get(),
            &mixxx::LibraryExporter::slotRequestExportWithInitialPlaylist);
#endif
}

QmlLibraryProxy::~QmlLibraryProxy() = default;

void QmlLibraryProxy::refreshWaveformCacheDiskUsage() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary && QmlConfigProxy::get()) {
        return;
    }

    AnalysisDao analysisDao(QmlConfigProxy::get());
    const QSqlDatabase dbConnection =
            mixxx::DbConnectionPooled(s_pLibrary->dbConnectionPool());
    const size_t byteCount =
            analysisDao.getDiskUsageInBytes(
                    dbConnection, AnalysisDao::TYPE_WAVEFORM) +
            analysisDao.getDiskUsageInBytes(
                    dbConnection, AnalysisDao::TYPE_WAVESUMMARY);
    const QString diskUsage =
            QLocale().formattedDataSize(byteCount, 1, QLocale::DataSizeSIFormat);
    if (m_waveformCacheDiskUsage == diskUsage) {
        return;
    }
    m_waveformCacheDiskUsage = diskUsage;
    emit waveformCacheDiskUsageChanged();
}

bool QmlLibraryProxy::clearCachedWaveforms() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary && QmlConfigProxy::get()) {
        return false;
    }

    AnalysisDao analysisDao(QmlConfigProxy::get());
    const QSqlDatabase dbConnection =
            mixxx::DbConnectionPooled(s_pLibrary->dbConnectionPool());
    const bool waveformDeleted = analysisDao.deleteAnalysesByType(
            dbConnection, AnalysisDao::TYPE_WAVEFORM);
    const bool summaryDeleted = analysisDao.deleteAnalysesByType(
            dbConnection, AnalysisDao::TYPE_WAVESUMMARY);
    refreshWaveformCacheDiskUsage();
    return waveformDeleted && summaryDeleted;
}

void QmlLibraryProxy::deliverPendingLibraryScanSummary() {
    const auto pendingResult = m_pTrackCollectionManager->takePendingLibraryScanSummary();
    if (!pendingResult) {
        return;
    }
    const auto& result = *pendingResult;
    const UserSettingsPointer pConfig = QmlConfigProxy::get();
    if (!pConfig ||
            !pConfig->getValue<bool>(
                    mixxx::library::prefs::kShowScanSummaryConfigKey, true)) {
        return;
    }
    if (result.autoscan &&
            result.numNewTracks == 0 &&
            result.numNewMissingTracks == 0 &&
            result.numRediscoveredTracks == 0) {
        return;
    }

    const QString title = tr("Library scan finished");
    if (result.noDirectoriesConfigured) {
        emit libraryScanSummaryAvailable(title,
                tr("No music directories configured for scanning."),
                tr("Add directories in the library preferences."));
        return;
    }

    const QString text = tr("Scan took %1").arg(result.durationString);
    QStringList details;
    if (result.numNewTracks == 0 &&
            result.numMovedTracks == 0 &&
            result.numNewMissingTracks == 0 &&
            result.numRediscoveredTracks == 0) {
        details.append(tr("No changes detected."));
    } else {
        if (result.numNewTracks != 0) {
            details.append(tr("%n new track(s) found", nullptr, result.numNewTracks));
        }
        if (result.numMovedTracks != 0) {
            details.append(
                    tr("%n moved track(s) detected", nullptr, result.numMovedTracks));
        }
        if (result.numNewMissingTracks != 0) {
            details.append(tr("%n track(s) missing (%1 total)",
                    nullptr,
                    result.numNewMissingTracks)
                            .arg(result.numMissingTracks));
        }
        if (result.numRediscoveredTracks != 0) {
            details.append(tr("%n track(s) rediscovered",
                    nullptr,
                    result.numRediscoveredTracks));
        }
    }
    details.append(tr("%n track(s) in total", nullptr, result.tracksTotal));
    emit libraryScanSummaryAvailable(title, text, details.join(QLatin1Char('\n')));
}

QmlLibraryTrackListModel* QmlLibraryProxy::model() const {
    return make_qml_owned<QmlLibraryTrackListModel>(
            QList<QmlLibraryTrackListColumn*>{}, s_pLibrary->trackTableModel())
            .get();
}
void QmlLibraryProxy::analyze(const QmlTrackProxy* track) const {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }
    emit s_pLibrary->analyzeTracks({track->internal()->getId()});
}

QVariantList QmlLibraryProxy::findCompatibleTracks(
        QmlTrackProxy* pReferenceTrack, int maxResults) {
    QVariantList compatibleTracks;
    if (pReferenceTrack == nullptr || maxResults <= 0 ||
            m_pTrackCollectionManager == nullptr) {
        return compatibleTracks;
    }

    const TrackPointer pReferenceTrackInternal = pReferenceTrack->internal();
    if (!pReferenceTrackInternal) {
        return compatibleTracks;
    }

    const QList<mixxx::track::io::key::ChromaticKey> compatibleKeys =
            mixxx::HarmonicMixing::compatibleKeys(pReferenceTrackInternal->getKey());
    if (compatibleKeys.isEmpty()) {
        return compatibleTracks;
    }

    // Share the tolerance with Auto DJ, so both agree on what counts as a
    // compatible tempo.
    const int tolerancePercent = s_pConfig
            ? s_pConfig->getValue(ConfigKey(kAutoDjGroup, kHarmonicBpmToleranceKey),
                      static_cast<int>(mixxx::HarmonicMixing::kDefaultBpmTolerancePercent))
            : static_cast<int>(mixxx::HarmonicMixing::kDefaultBpmTolerancePercent);

    // Half and double tempo are mixed the same way, so they are accepted too.
    // An unknown reference tempo leaves the list empty, which mixes by key only.
    const double referenceBpm = pReferenceTrackInternal->getBpm();
    QList<mixxx::HarmonicMixing::BpmRange> bpmRanges;
    for (const double tempoFactor : {1.0, 2.0, 0.5}) {
        const mixxx::HarmonicMixing::BpmRange range =
                mixxx::HarmonicMixing::bpmRange(
                        referenceBpm * tempoFactor,
                        static_cast<double>(tolerancePercent));
        if (range.isValid()) {
            bpmRanges.append(range);
        }
    }

    QStringList keyPlaceholders;
    keyPlaceholders.reserve(compatibleKeys.size());
    for (int i = 0; i < compatibleKeys.size(); ++i) {
        keyPlaceholders.append(QStringLiteral(":key%1").arg(i));
    }

    QString bpmFilter;
    for (int i = 0; i < bpmRanges.size(); ++i) {
        if (!bpmFilter.isEmpty()) {
            bpmFilter += QStringLiteral(" OR ");
        }
        bpmFilter += QStringLiteral("l.") + LIBRARYTABLE_BPM +
                QStringLiteral(" BETWEEN :bpm%1Min AND :bpm%2Max").arg(i).arg(i);
    }
    if (!bpmFilter.isEmpty()) {
        bpmFilter = QStringLiteral(" AND (") + bpmFilter + QLatin1Char(')');
    }

    const QString strQuery = QStringLiteral("SELECT l.") +
            LIBRARYTABLE_ARTIST + QStringLiteral(", l.") +
            LIBRARYTABLE_TITLE + QStringLiteral(", l.") +
            LIBRARYTABLE_BPM + QStringLiteral(", l.") +
            LIBRARYTABLE_KEY_ID + QStringLiteral(", l.") +
            LIBRARYTABLE_TIMESPLAYED + QStringLiteral(", tl.") +
            TRACKLOCATIONSTABLE_LOCATION +
            QStringLiteral(" FROM ") + LIBRARY_TABLE + QStringLiteral(" l") +
            QStringLiteral(" INNER JOIN ") + TRACKLOCATIONS_TABLE +
            QStringLiteral(" tl ON l.") + LIBRARYTABLE_LOCATION +
            QStringLiteral(" = tl.") + LIBRARYTABLE_ID +
            QStringLiteral(" WHERE l.") + LIBRARYTABLE_KEY_ID +
            QStringLiteral(" IN (") +
            keyPlaceholders.join(QLatin1Char(',')) + QLatin1Char(')') +
            bpmFilter +
            QStringLiteral(" AND l.") + LIBRARYTABLE_ID +
            QStringLiteral(" != :referenceId") +
            // Columns that predate a default are nullable, and NULL means "not
            // deleted", so they must not hide the track.
            QStringLiteral(" AND IFNULL(l.") + LIBRARYTABLE_MIXXXDELETED +
            QStringLiteral(", 0) != 1") +
            QStringLiteral(" AND IFNULL(tl.fs_deleted, 0) != 1") +
            QStringLiteral(" ORDER BY l.") + LIBRARYTABLE_TIMESPLAYED +
            QStringLiteral(" DESC, RANDOM() LIMIT :limit");

    QSqlQuery sqlQuery(
            m_pTrackCollectionManager->internalCollection()->database());
    if (!sqlQuery.prepare(strQuery)) {
        qWarning() << "QmlLibraryProxy: could not prepare the compatibility query:"
                   << sqlQuery.lastError();
        return compatibleTracks;
    }

    for (int i = 0; i < compatibleKeys.size(); ++i) {
        sqlQuery.bindValue(QStringLiteral(":key%1").arg(i),
                static_cast<int>(compatibleKeys.at(i)));
    }
    for (int i = 0; i < bpmRanges.size(); ++i) {
        sqlQuery.bindValue(QStringLiteral(":bpm%1Min").arg(i), bpmRanges.at(i).min);
        sqlQuery.bindValue(QStringLiteral(":bpm%1Max").arg(i), bpmRanges.at(i).max);
    }
    sqlQuery.bindValue(QStringLiteral(":referenceId"),
            pReferenceTrackInternal->getId().toVariant());
    sqlQuery.bindValue(QStringLiteral(":limit"), maxResults);

    if (!sqlQuery.exec()) {
        qWarning() << "QmlLibraryProxy: the compatibility query failed:"
                   << sqlQuery.lastError();
        return compatibleTracks;
    }

    while (sqlQuery.next()) {
        QVariantMap compatibleTrack;
        compatibleTrack.insert(QStringLiteral("artist"), sqlQuery.value(0).toString());
        compatibleTrack.insert(QStringLiteral("title"), sqlQuery.value(1).toString());
        compatibleTrack.insert(QStringLiteral("bpm"), sqlQuery.value(2).toDouble());
        // Numeric key, fed to Mixxx.KeyUtils.keyToString() by the skin so that
        // the user's key notation is respected.
        compatibleTrack.insert(QStringLiteral("key"), sqlQuery.value(3).toDouble());
        compatibleTrack.insert(QStringLiteral("timesPlayed"), sqlQuery.value(4).toInt());
        compatibleTrack.insert(QStringLiteral("locationUrl"),
                QUrl::fromLocalFile(sqlQuery.value(5).toString()).toString());
        compatibleTracks.append(compatibleTrack);
    }
    return compatibleTracks;
}

void QmlLibraryProxy::createCrate() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    s_pLibrary->slotCreateCrate();
}

void QmlLibraryProxy::createPlaylist() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    s_pLibrary->slotCreatePlaylist();
}

bool QmlLibraryProxy::enginePrimeExportAvailable() const {
#ifdef __ENGINEPRIME__
    return true;
#else
    return false;
#endif
}

void QmlLibraryProxy::exportLibrary() {
#ifdef __ENGINEPRIME__
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    emit s_pLibrary->exportLibrary();
#endif
}

void QmlLibraryProxy::rescanLibrary() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    if (libraryScanActive()) {
        return;
    }
    TrackCollectionManager* pTrackCollectionManager =
            s_pLibrary->trackCollectionManager();
    VERIFY_OR_DEBUG_ASSERT(pTrackCollectionManager) {
        return;
    }
    pTrackCollectionManager->startLibraryScan();
}

void QmlLibraryProxy::searchInCurrentView() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    s_pLibrary->slotSearchInCurrentView();
}

void QmlLibraryProxy::searchInTracksLibrary() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    s_pLibrary->slotSearchInAllTracks();
}

void QmlLibraryProxy::showAutoDJ() {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return;
    }
    s_pLibrary->showAutoDJ();
}

QString QmlLibraryProxy::deckHotcueLabel(
        QmlTrackProxy* track,
        int hotcueNumber) const {
    const CuePointer pCue = findDeckHotcue(track, hotcueNumber);
    return pCue ? pCue->getLabel() : QString();
}

bool QmlLibraryProxy::setDeckHotcueLabel(
        QmlTrackProxy* track,
        int hotcueNumber,
        const QString& label) {
    const CuePointer pCue = findDeckHotcue(track, hotcueNumber);
    if (!pCue) {
        return false;
    }
    pCue->setLabel(label);
    return true;
}

bool QmlLibraryProxy::setDeckHotcueType(
        QmlTrackProxy* track,
        const QString& group,
        int hotcueNumber,
        const QString& action) {
    const CuePointer pCue = findDeckHotcue(track, hotcueNumber);
    if (!pCue) {
        return false;
    }

    UserSettingsPointer pConfig = QmlConfigProxy::get();
    VERIFY_OR_DEBUG_ASSERT(pConfig) {
        return false;
    }

    const TrackPointer pTrack = track->internal();
    if (action == QStringLiteral("standard")) {
        if (pCue->getType() != mixxx::CueType::HotCue) {
            updateCueTypeAndColorIfDefault(pConfig, pCue, mixxx::CueType::HotCue);
        }
        return true;
    }

    if (action == QStringLiteral("loop-auto")) {
        Cue::StartAndEndPositions cueStartEnd = pCue->getStartAndEndPosition();
        if (pCue->getType() == mixxx::CueType::Jump) {
            const auto endPosition = cueStartEnd.endPosition;
            if (cueStartEnd.endPosition < cueStartEnd.startPosition) {
                cueStartEnd.endPosition = cueStartEnd.startPosition;
                cueStartEnd.startPosition = endPosition;
            }
            pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
        }
        if (!cueStartEnd.endPosition.isValid() ||
                cueStartEnd.endPosition <= cueStartEnd.startPosition) {
            const double beatloopSize = ControlObject::get(
                    ConfigKey(group, QStringLiteral("beatloop_size")));
            const mixxx::BeatsPointer pBeats = pTrack->getBeats();
            if (beatloopSize <= 0 || !pBeats) {
                return false;
            }
            const auto position = pBeats->findNBeatsFromPosition(
                    cueStartEnd.startPosition, beatloopSize);
            if (position <= pCue->getPosition()) {
                return false;
            }
            pCue->setEndPosition(position);
        }
        updateCueTypeAndColorIfDefault(pConfig, pCue, mixxx::CueType::Loop);
        return true;
    }

    if (action == QStringLiteral("loop-manual")) {
        if (pCue->getType() == mixxx::CueType::Jump &&
                pCue->getPosition() > pCue->getEndPosition()) {
            Cue::StartAndEndPositions cueStartEnd = pCue->getStartAndEndPosition();
            const auto endPosition = cueStartEnd.endPosition;
            cueStartEnd.endPosition = cueStartEnd.startPosition;
            cueStartEnd.startPosition = endPosition;
            pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
        }
        const auto newPosition = getCurrentPlayPositionWithQuantize(pTrack, group);
        if (newPosition <= pCue->getPosition()) {
            return false;
        }
        pCue->setEndPosition(newPosition);
        updateCueTypeAndColorIfDefault(pConfig, pCue, mixxx::CueType::Loop);
        return true;
    }

    if (action == QStringLiteral("jump-auto")) {
        Cue::StartAndEndPositions cueStartEnd = pCue->getStartAndEndPosition();
        if (pCue->getType() == mixxx::CueType::Loop ||
                pCue->getType() == mixxx::CueType::Jump) {
            const auto endPosition = cueStartEnd.endPosition;
            cueStartEnd.endPosition = cueStartEnd.startPosition;
            cueStartEnd.startPosition = endPosition;
        }
        if (!cueStartEnd.endPosition.isValid()) {
            const auto newPosition = getCurrentPlayPositionWithQuantize(pTrack, group);
            if (std::abs(newPosition - cueStartEnd.startPosition) <=
                    kMinimumAudibleLoopSizeFrames) {
                return false;
            }
            cueStartEnd.endPosition = newPosition;
        }
        pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
        updateCueTypeAndColorIfDefault(pConfig, pCue, mixxx::CueType::Jump);
        return true;
    }

    if (action == QStringLiteral("jump-manual")) {
        Cue::StartAndEndPositions cueStartEnd = pCue->getStartAndEndPosition();
        const auto newPosition = getCurrentPlayPositionWithQuantize(pTrack, group);
        if (newPosition == cueStartEnd.startPosition) {
            return false;
        }
        cueStartEnd.endPosition = newPosition;
        pCue->setStartAndEndPosition(cueStartEnd.startPosition, cueStartEnd.endPosition);
        updateCueTypeAndColorIfDefault(pConfig, pCue, mixxx::CueType::Jump);
        return true;
    }

    return false;
}

QmlLibraryProxy::JumpDirection QmlLibraryProxy::deckHotcueJumpDirection(
        QmlTrackProxy* track,
        const QString& group,
        int hotcueNumber) const {
    const CuePointer pCue = findDeckHotcue(track, hotcueNumber);
    if (!pCue) {
        return JumpDirection::Impossible;
    }

    if (pCue->getType() == mixxx::CueType::HotCue) {
        const Cue::StartAndEndPositions cueStartEnd = pCue->getStartAndEndPosition();
        auto newPosition = cueStartEnd.endPosition;
        if (!newPosition.isValid()) {
            newPosition = getCurrentPlayPositionWithQuantize(track->internal(), group);
        }
        if (!newPosition.isValid() ||
                std::abs(newPosition - cueStartEnd.startPosition) <=
                        kMinimumAudibleLoopSizeFrames) {
            return JumpDirection::Impossible;
        }
        return newPosition < cueStartEnd.startPosition
                ? JumpDirection::Forward
                : JumpDirection::Backward;
    }

    const bool isForward = pCue->getType() != mixxx::CueType::Jump ||
            pCue->getPosition() > pCue->getEndPosition();
    return isForward ? JumpDirection::Forward : JumpDirection::Backward;
}

void QmlLibraryProxy::cleanupDeckHotcuePopup(
        QmlTrackProxy* track,
        int hotcueNumber) {
    const CuePointer pCue = findDeckHotcue(track, hotcueNumber);
    if (pCue &&
            pCue->getType() == mixxx::CueType::HotCue &&
            pCue->getEndPosition().isValid()) {
        pCue->setEndPosition(mixxx::audio::FramePos());
    }
}

// static
QmlLibraryProxy* QmlLibraryProxy::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet";
        return nullptr;
    }
    return new QmlLibraryProxy(pQmlEngine);
}

QmlLibraryProxy::AddResult QmlLibraryProxy::addSource(
        const QUrl& newPath) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::AddResult::InvalidOrMissingDirectory;
    }
    QDir directory(newPath.toLocalFile());
    Sandbox::createSecurityTokenForDir(directory);
    return static_cast<QmlLibraryProxy::AddResult>(
            s_pLibrary->trackCollectionManager()->addDirectory(
                    mixxx::FileInfo(newPath.toLocalFile())));
}

QmlLibraryProxy::RemoveResult QmlLibraryProxy::removeSource(
        const QUrl& oldPath, SourceRemovalType removalType) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::RemoveResult::NotFound;
    }

    DirectoryDAO::RemoveResult result =
            s_pLibrary->trackCollectionManager()->removeDirectory(
                    mixxx::FileInfo(oldPath.toLocalFile()));
    if (result != DirectoryDAO::RemoveResult::Ok) {
        return static_cast<QmlLibraryProxy::RemoveResult>(result);
    }

    switch (removalType) {
    case SourceRemovalType::KeepTracks:
        break;
    case SourceRemovalType::HideTracks:
        // Mark all tracks in this directory as deleted but DON'T purge them
        // in case the user re-adds them manually.
        s_pLibrary->trackCollectionManager()->hideAllTracks(oldPath.toLocalFile());
        break;
    case SourceRemovalType::PurgeTracks:
        // The user requested that we purge all metadata.
        s_pLibrary->trackCollectionManager()->purgeAllTracks(oldPath.toLocalFile());
        break;
    default:
        DEBUG_ASSERT(!"unreachable");
    }
    return static_cast<QmlLibraryProxy::RemoveResult>(result);
}

QmlLibraryProxy::RelocateResult QmlLibraryProxy::relinkSource(
        const QUrl& oldPath, const QUrl& newPath) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::RelocateResult::SqlError;
    }
    return static_cast<QmlLibraryProxy::RelocateResult>(
            s_pLibrary->trackCollectionManager()->relocateDirectory(
                    oldPath.toLocalFile(), newPath.toLocalFile()));
}

// Static
qsizetype QmlLibraryProxy::sources_count(QQmlListProperty<QmlLibrarySource>* pList) {
    VERIFY_OR_DEBUG_ASSERT(pList && pList->object && s_pLibrary) {
        return 0;
    }
    return s_pLibrary->trackCollectionManager()
            ->internalCollection()
            ->getRootDirectories()
            .size();
}

// Static
QmlLibrarySource* QmlLibraryProxy::sources_at(
        QQmlListProperty<QmlLibrarySource>* pList, qsizetype index) {
    VERIFY_OR_DEBUG_ASSERT(pList && pList->object) {
        return nullptr;
    }
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        return nullptr;
    }
    return make_qml_owned<QmlLibrarySource>(
            s_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->getRootDirectories()
                    .at(index));
}

// Static
void QmlLibraryProxy::sources_clear(QQmlListProperty<QmlLibrarySource>*) {
    DEBUG_ASSERT(!"unsupported operation");
}

} // namespace qml
} // namespace mixxx
