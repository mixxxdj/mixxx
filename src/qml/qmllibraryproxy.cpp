#include "qml/qmllibraryproxy.h"

#include <QAbstractItemModel>
#include <QQmlEngine>
#include <cmath>

#include "control/controlobject.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "moc_qmllibraryproxy.cpp"
#include "preferences/colorpalettesettings.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmllibrarytracklistmodel.h"
#include "qmltrackproxy.h"
#include "track/cue.h"
#include "track/track.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

namespace {
const ConfigKey kHotcueDefaultColorIndexConfigKey("[Controls]", "HotcueDefaultColorIndex");
const ConfigKey kLoopDefaultColorIndexConfigKey("[Controls]", "LoopDefaultColorIndex");
const ConfigKey kJumpDefaultColorIndexConfigKey("[Controls]", "jump_default_color_index");

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

QmlLibraryProxy::QmlLibraryProxy(
        std::shared_ptr<Library> pLibrary, QObject* parent)
        : QObject(parent),
          m_pLibrary(pLibrary),
          m_pModelProperty(new QmlLibraryTrackListModel(
                  QList<QmlLibraryTrackListColumn*>{}, m_pLibrary->trackTableModel(), this)),
          m_pScanner(new QmlLibraryScannerProxy(
                  m_pLibrary->trackCollectionManager()->scanner(), this)) {
}

QmlLibraryScannerProxy::QmlLibraryScannerProxy(LibraryScanner* libraryScanner, QObject* parent)
        : QObject(parent),
          m_pLibraryScanner(libraryScanner),
          m_running(false),
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

QmlLibraryProxy::~QmlLibraryProxy() = default;

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
    if (!track || !track->internal() || !pCue) {
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
    return new QmlLibraryProxy(s_pLibrary, pQmlEngine);
}

QmlLibraryProxy::AddResult QmlLibraryProxy::addSource(
        const QString& newPath) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::AddResult::InvalidOrMissingDirectory;
    }
    QDir directory(newPath);
    Sandbox::createSecurityTokenForDir(directory);
    return static_cast<QmlLibraryProxy::AddResult>(
            s_pLibrary->trackCollectionManager()->addDirectory(
                    mixxx::FileInfo(newPath)));
}

QmlLibraryProxy::RemoveResult QmlLibraryProxy::removeSource(
        const QString& oldPath, SourceRemovalType removalType) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::RemoveResult::NotFound;
    }

    DirectoryDAO::RemoveResult result =
            s_pLibrary->trackCollectionManager()->removeDirectory(mixxx::FileInfo(oldPath));
    if (result != DirectoryDAO::RemoveResult::Ok) {
        return static_cast<QmlLibraryProxy::RemoveResult>(result);
    }

    switch (removalType) {
    case SourceRemovalType::KeepTracks:
        break;
    case SourceRemovalType::HideTracks:
        // Mark all tracks in this directory as deleted but DON'T purge them
        // in case the user re-adds them manually.
        s_pLibrary->trackCollectionManager()->hideAllTracks(oldPath);
        break;
    case SourceRemovalType::PurgeTracks:
        // The user requested that we purge all metadata.
        s_pLibrary->trackCollectionManager()->purgeAllTracks(oldPath);
        break;
    default:
        DEBUG_ASSERT(!"unreachable");
    }
    return static_cast<QmlLibraryProxy::RemoveResult>(result);
}

QmlLibraryProxy::RelocateResult QmlLibraryProxy::relinkSource(
        const QString& oldPath, const QString& newPath) {
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet!";
        return QmlLibraryProxy::RelocateResult::SqlError;
    }
    return static_cast<QmlLibraryProxy::RelocateResult>(
            s_pLibrary->trackCollectionManager()->relocateDirectory(
                    oldPath, newPath));
}

// Static
qsizetype QmlLibraryProxy::sources_count(QQmlListProperty<QmlLibrarySource>* pList) {
    QmlLibraryProxy* pLibrary = static_cast<QmlLibraryProxy*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pLibrary) {
        return 0;
    }
    return pLibrary->m_pLibrary->trackCollectionManager()
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
    QmlLibraryProxy* pLibrary = static_cast<QmlLibraryProxy*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pLibrary) {
        return nullptr;
    }
    return make_qml_owned<QmlLibrarySource>(
            pLibrary->m_pLibrary->trackCollectionManager()
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
