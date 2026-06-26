#include "qml/qmllibraryproxy.h"

#include <QAbstractItemModel>
#include <QCursor>
#include <QQmlEngine>
#include <cmath>

#include "control/controlobject.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "moc_qmllibraryproxy.cpp"
#include "preferences/colorpalettesettings.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmllibrarytracklistmodel.h"
#include "qmltrackproxy.h"
#include "track/cue.h"
#include "track/track.h"
#include "track/track_decl.h"
#include "util/assert.h"
#include "widget/wtrackmenu.h"

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

QmlLibraryProxy::QmlLibraryProxy(QObject* parent)
        : QObject(parent) {
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

void QmlLibraryProxy::showDeckTrackMenu(
        QmlTrackProxy* track,
        const QString& group,
        const QString& property,
        int globalXPosition,
        int globalYPosition) {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }

    ensureDeckTrackMenu();
    VERIFY_OR_DEBUG_ASSERT(m_pDeckTrackMenu) {
        return;
    }

    m_pDeckTrackMenu->loadTrack(track->internal(), group);
    m_pDeckTrackMenu->updateMenus();
    m_pDeckTrackMenu->setTrackPropertyName(property);
    const QPoint globalPosition = globalXPosition >= 0 && globalYPosition >= 0
            ? QPoint(globalXPosition, globalYPosition)
            : QCursor::pos();
    m_pDeckTrackMenu->popup(globalPosition);
}

void QmlLibraryProxy::showDeckTrackProperties(
        QmlTrackProxy* track,
        const QString& group,
        const QString& property) {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }

    ensureDeckTrackMenu();
    VERIFY_OR_DEBUG_ASSERT(m_pDeckTrackMenu) {
        return;
    }

    m_pDeckTrackMenu->loadTrack(track->internal(), group);
    m_pDeckTrackMenu->setTrackPropertyName(property);
    m_pDeckTrackMenu->slotShowDlgTrackInfo();
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

void QmlLibraryProxy::ensureDeckTrackMenu() {
    if (m_pDeckTrackMenu) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(s_pLibrary && QmlConfigProxy::get()) {
        return;
    }

    m_pDeckTrackMenu = std::make_unique<WTrackMenu>(
            nullptr,
            QmlConfigProxy::get(),
            s_pLibrary.get(),
            WTrackMenu::kDeckTrackMenuFeatures);
    connect(m_pDeckTrackMenu.get(),
            &WTrackMenu::trackMenuVisible,
            this,
            [this](bool visible) {
                ControlObject::set(
                        ConfigKey(m_pDeckTrackMenu->getDeckGroup(), kShowTrackMenuKey),
                        visible ? 1.0 : 0.0);
            });
    connect(m_pDeckTrackMenu.get(),
            &WTrackMenu::saveCurrentViewState,
            s_pLibrary.get(),
            &Library::slotSaveCurrentViewState);
    connect(m_pDeckTrackMenu.get(),
            &WTrackMenu::restoreCurrentViewStateOrIndex,
            s_pLibrary.get(),
            &Library::slotRestoreCurrentViewState);
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

} // namespace qml
} // namespace mixxx
