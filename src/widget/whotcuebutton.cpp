#include "widget/whotcuebutton.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>

#include "mixer/playerinfo.h"
#include "moc_whotcuebutton.cpp"
#include "track/track.h"
#include "util/dnd.h"
#include "widget/controlwidgetconnection.h"

namespace {
constexpr int kDefaultDimBrightThreshold = 127;
const QString kMimeTextDelimiter = QStringLiteral("\n");
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

    auto* pLeftConnection = new ControlParameterWidgetConnection(
            this,
            getLeftClickConfigKey(), // "activate"
            nullptr,
            ControlParameterWidgetConnection::DIR_FROM_WIDGET,
            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
    addLeftConnection(pLeftConnection);

    auto* pDisplayConnection = new ControlParameterWidgetConnection(
            this,
            createConfigKey(QStringLiteral("status")),
            nullptr,
            ControlParameterWidgetConnection::DIR_TO_WIDGET,
            ControlParameterWidgetConnection::EMIT_NEVER);
    addConnection(pDisplayConnection);
    setDisplayConnection(pDisplayConnection);

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
            QList<CuePointer> cueList = pTrack->getCuePoints();
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
    WPushButton::mousePressEvent(pEvent);
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
    // Only allow moving set hotcues to empty or set slots.
    // Note that Track::swapHotcues() allows both directions.
    if (m_hotcue == Cue::kNoHotCue) {
        return;
    }

    if (DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
        QDrag* pDrag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        mimeData->setText(mimeTextIdentifier() + kMimeTextDelimiter + QString::number(m_hotcue));
        pDrag->setMimeData(mimeData);
        // Use the currently rendered button as dnd cursor.
        const QPixmap currLook = grab(rect().marginsRemoved(m_dndRectMargins));
        pDrag->setDragCursor(currLook, Qt::MoveAction);
        // Release this button after failed or successful drop.
        // This prevents both the preview and the pressed state from getting stuck.
        connect(pDrag,
                &QDrag::destroyed,
                this,
                &WHotcueButton::release);
        pDrag->exec();
    }
}

void WHotcueButton::dragEnterEvent(QDragEnterEvent* pEvent) {
    if (pEvent->source() == this) {
        pEvent->ignore();
        return;
    }
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
    if (!pTrack) {
        pEvent->ignore();
        return;
    }
    const QString& mimeText = pEvent->mimeData()->text();
    QStringList mimeTextLines = mimeText.split(kMimeTextDelimiter);
    if (mimeTextLines.size() != 2 || mimeTextLines.at(0) != mimeTextIdentifier()) {
        pEvent->ignore();
        return;
    }
    bool okay = false;
    int otherCueNum = mimeTextLines.at(1).toInt(&okay);
    if (okay && otherCueNum != m_hotcue) {
        pEvent->acceptProposedAction();
    }
}

void WHotcueButton::dropEvent(QDropEvent* pEvent) {
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
    if (!pTrack) {
        return;
    }
    const QString& mimeText = pEvent->mimeData()->text();
    QStringList mimeTextLines = mimeText.split(kMimeTextDelimiter);
    int otherCueNum = mimeTextLines.at(1).toInt();
    pTrack->swapHotcues(otherCueNum, m_hotcue);
    // Alternative way to release drag source button:
    // WHotcueButton* srcBtn = qobject_cast<WHotcueButton*>(pEvent->source());
    // DEBUG_ASSERT(srcBtn);
    // srcBtn->release();
}

const QString WHotcueButton::mimeTextIdentifier() const {
    return QStringLiteral("hotcue_") + m_group;
}

void WHotcueButton::release() {
    // Actions from WPushButton::mouseReleaseEvent()
    // TODO Create a QMouseVent and call mouseReleaseEvent instead of this hack?
    setControlParameterLeftUp(0);
    m_bPressed = false;
    restyleAndRepaint();
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
