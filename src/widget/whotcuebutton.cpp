#include "widget/whotcuebutton.h"

#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>

#include "mixer/playerinfo.h"
#include "moc_whotcuebutton.cpp"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/dnd.h"
#include "util/valuetransformer.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wbasewidget.h"

namespace {
constexpr int kDefaultDimBrightThreshold = 127;
const QString kDragDataType = QStringLiteral("hotcueDragInfo");
} // anonymous namespace

WHotcueButton::WHotcueButton(const QString& group, QWidget* pParent)
        : WPushButton(pParent),
          m_group(group),
          m_hotcue(Cue::kNoHotCue),
          m_hoverCueColor(false),
          m_pCoColor(nullptr),
          m_cueColorDimThreshold(kDefaultDimBrightThreshold),
          m_bCueColorDimmed(false),
          m_bCueColorIsLight(false),
          m_bCueColorIsDark(false) {
    setAcceptDrops(true);
}

void WHotcueButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    bool ok;
    int hotcue = context.selectInt(node, QStringLiteral("Hotcue"), &ok);
    if (ok && hotcue > 0) {
        m_hotcue = hotcue - 1;
    } else {
        SKIN_WARNING(node,
                context,
                QStringLiteral("Hotcue index '%1' invalid")
                        .arg(context.selectString(node, QStringLiteral("Hotcue"))));
    }

    bool okay;
    m_cueColorDimThreshold = context.selectInt(node, QStringLiteral("DimBrightThreshold"), &okay);
    if (!okay) {
        m_cueColorDimThreshold = kDefaultDimBrightThreshold;
    }

    m_hoverCueColor = context.selectBool(node, QStringLiteral("Hover"), false);

    // For dnd/swapping hotcues we use the rendered widget pixmap as dnd cursor.
    // Unfortnately the margin that constraints the bg color is not considered,
    // so we shrink the rect by custom margins.
    okay = false;
    int dndMargin = context.selectInt(node, QStringLiteral("DndRectMargin"), &okay);
    if (okay && dndMargin > 0) {
        m_dndRectMargins = QMargins(dndMargin, dndMargin, dndMargin, dndMargin);
    }

    m_pCueMenuPopup = make_parented<WCueMenuPopup>(context.getConfig(), this);
    ColorPaletteSettings colorPaletteSettings(context.getConfig());
    auto colorPalette = colorPaletteSettings.getHotcueColorPalette();
    m_pCueMenuPopup->setColorPalette(colorPalette);

    setFocusPolicy(Qt::NoFocus);

    m_pCoColor = make_parented<ControlProxy>(
            createConfigKey(QStringLiteral("color")),
            this,
            ControlFlag::NoAssertIfMissing);
    m_pCoColor->connectValueChanged(this, &WHotcueButton::slotColorChanged);
    slotColorChanged(m_pCoColor->get());

    m_pCoType = make_parented<ControlProxy>(
            createConfigKey(QStringLiteral("type")),
            this,
            ControlFlag::NoAssertIfMissing);
    m_pCoType->connectValueChanged(this, &WHotcueButton::slotTypeChanged);
    slotTypeChanged(m_pCoType->get());

    addConnection(std::make_unique<ControlParameterWidgetConnection>(
                          this,
                          getLeftClickConfigKey(), // "activate"
                          nullptr,
                          ControlParameterWidgetConnection::DIR_FROM_WIDGET,
                          ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE),
            WBaseWidget::ConnectionSide::Left);

    addAndSetDisplayConnection(std::make_unique<ControlParameterWidgetConnection>(
                                       this,
                                       createConfigKey(QStringLiteral("status")),
                                       nullptr,
                                       ControlParameterWidgetConnection::DIR_TO_WIDGET,
                                       ControlParameterWidgetConnection::EMIT_NEVER),
            WBaseWidget::ConnectionSide::None);

    QDomNode con = context.selectNode(node, QStringLiteral("Connection"));
    if (!con.isNull()) {
        SKIN_WARNING(node, context, QStringLiteral("Additional Connections are not allowed"));
    }
}

void WHotcueButton::mousePressEvent(QMouseEvent* pEvent) {
    const bool rightClick = pEvent->button() == Qt::RightButton;
    if (rightClick) {
        if (isPressed()) {
            // Discard right clicks when already left clicked.
            // Otherwise the pop up menu receives the release event and the
            // button stucks in the pressed stage.
            return;
        }
        if (readDisplayValue()) {
            // hot cue is set
            TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
            if (!pTrack) {
                return;
            }

            CuePointer pHotCue;
            const QList<CuePointer> cueList = pTrack->getCuePoints();
            for (const auto& pCue : cueList) {
                if (pCue->getHotCue() == m_hotcue) {
                    pHotCue = pCue;
                    break;
                }
            }
            if (!pHotCue) {
                return;
            }
            if (pEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                pTrack->removeCue(pHotCue);
                return;
            }
            m_pCueMenuPopup->setTrackCueGroup(pTrack, pHotCue, m_group);
            // use the bottom left corner as starting point for popup
            m_pCueMenuPopup->popup(mapToGlobal(QPoint(0, height())));
        }
        return;
    }

    // Pass all other press events to the base class.
    // Except when Shift is pressed which is used to swap hotcues without
    // starting the preview.
    if (!pEvent->modifiers().testFlag(Qt::ShiftModifier)) {
        WPushButton::mousePressEvent(pEvent);
    }
}

void WHotcueButton::mouseReleaseEvent(QMouseEvent* pEvent) {
    const bool rightClick = pEvent->button() == Qt::RightButton;
    if (rightClick) {
        // Don't handle stray release events
        return;
    }
    WPushButton::mouseReleaseEvent(pEvent);
}

void WHotcueButton::mouseMoveEvent(QMouseEvent* pEvent) {
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
    if (!pTrack) {
        return;
    }

    // Maybe set up a QDrag for swapping hotcues.
    // Only allow moving set hotcues to empty or set slots.
    // Note that Track::swapHotcues() allows both directions.
    if (m_hotcue == Cue::kNoHotCue) {
        return;
    }

    if (DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
        const TrackId id = pTrack->getId();
        VERIFY_OR_DEBUG_ASSERT(id.isValid()) {
            return;
        }
        QDrag* pDrag = new QDrag(this);
        HotcueDragInfo dragData(id, m_hotcue);
        auto mimeData = std::make_unique<QMimeData>();
        mimeData->setData(kDragDataType, dragData.toByteArray());
        pDrag->setMimeData(mimeData.release());

        // Use the currently rendered button as dnd cursor
        // (incl. hover and pressed style).
        // Note: for some reason, both grab() and render() render with sharp corners,
        // ie. qss 'border-radius' is not applied to the drag image.
        const QPixmap currLook = grab(rect().marginsRemoved(m_dndRectMargins));
        pDrag->setDragCursor(currLook, Qt::MoveAction);

        m_dragging = true;
        pDrag->exec();
        m_dragging = false;

        // Release this button afterwards.
        // This prevents both the preview and the pressed state from getting stuck.
        QEvent leaveEv(QEvent::Leave);
        QApplication::sendEvent(this, &leaveEv);
    }
}

void WHotcueButton::dragEnterEvent(QDragEnterEvent* pEvent) {
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
    if (!pTrack) {
        return;
    }
    QByteArray mimeDataBytes = pEvent->mimeData()->data(kDragDataType);
    if (mimeDataBytes.isEmpty()) {
        return;
    }
    HotcueDragInfo dragData = HotcueDragInfo::fromByteArray(mimeDataBytes);
    if (dragData.isValid() &&
            dragData.trackId == pTrack->getId()) {
        pEvent->acceptProposedAction();
    }
}

void WHotcueButton::dropEvent(QDropEvent* pEvent) {
    if (pEvent->source() == this) {
        pEvent->ignore();
        return;
    }
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
    if (!pTrack) {
        return;
    }
    QByteArray mimeDataBytes = pEvent->mimeData()->data(kDragDataType);
    if (mimeDataBytes.isEmpty()) {
        return;
    }
    HotcueDragInfo dragData = HotcueDragInfo::fromByteArray(mimeDataBytes);
    if (dragData.isValid() &&
            dragData.trackId == pTrack->getId() &&
            dragData.hotcue != m_hotcue) {
        pTrack->swapHotcues(dragData.hotcue, m_hotcue);
    }
}

ConfigKey WHotcueButton::createConfigKey(const QString& name) {
    ConfigKey key;
    key.group = m_group;
    // Add one to hotcue so that we don't have a hotcue_0
    key.item = QStringLiteral("hotcue_") + QString::number(m_hotcue + 1) + QChar('_') + name;
    return key;
}

void WHotcueButton::slotColorChanged(double color) {
    VERIFY_OR_DEBUG_ASSERT(color >= 0 && color <= 0xFFFFFF) {
        return;
    }
    QColor cueColor = QColor::fromRgb(static_cast<QRgb>(color));
    m_bCueColorDimmed = Color::isDimColorCustom(cueColor, m_cueColorDimThreshold);

    QString style =
            QStringLiteral(
                    "WWidget[displayValue=\"1\"], "
                    "WWidget[displayValue=\"2\"] { background-color: ") +
            cueColor.name() +
            QStringLiteral("; }");

    if (m_hoverCueColor) {
        style +=
                QStringLiteral(
                        "WWidget[displayValue=\"1\"]:hover, "
                        "WWidget[displayValue=\"2\"]:hover { background-color: ") +
                cueColor.lighter(m_bCueColorDimmed ? 120 : 80).name() +
                QStringLiteral("; }");
    }

    setStyleSheet(style);
    restyleAndRepaint();
}

void WHotcueButton::slotTypeChanged(double type) {
    switch (static_cast<mixxx::CueType>(static_cast<int>(type))) {
    case mixxx::CueType::Invalid:
        m_type = QLatin1String("");
        break;
    case mixxx::CueType::HotCue:
        m_type = QStringLiteral("hotcue");
        break;
    case mixxx::CueType::MainCue:
        m_type = QStringLiteral("maincue");
        break;
    case mixxx::CueType::Beat:
        m_type = QStringLiteral("beat");
        break;
    case mixxx::CueType::Loop:
        m_type = QStringLiteral("loop");
        break;
    case mixxx::CueType::Jump:
        m_type = QStringLiteral("jump");
        break;
    case mixxx::CueType::Intro:
        m_type = QStringLiteral("intro");
        break;
    case mixxx::CueType::Outro:
        m_type = QStringLiteral("outro");
        break;
    case mixxx::CueType::N60dBSound:
        m_type = QStringLiteral("n60dbsound");
        break;
    default:
        DEBUG_ASSERT(!"Unknown cue type!");
        m_type = QLatin1String("");
    }
    restyleAndRepaint();
}

void WHotcueButton::restyleAndRepaint() {
    if (readDisplayValue()) {
        // Adjust properties for Qss file
        m_bCueColorIsLight = !m_bCueColorDimmed;
        m_bCueColorIsDark = m_bCueColorDimmed;
    } else {
        // We are now at the background set by qss.
        // Since we don't know the color reset both
        m_bCueColorIsLight = false;
        m_bCueColorIsDark = false;
    }
    WPushButton::restyleAndRepaint();
}
