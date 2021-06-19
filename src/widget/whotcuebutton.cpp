#include "widget/whotcuebutton.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "mixer/playerinfo.h"
#include "moc_whotcuebutton.cpp"
#include "track/track.h"

namespace {
constexpr int kDefaultDimBrightThreshold = 127;
} // namespace

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
}

void WHotcueButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    bool ok;
    int hotcue = context.selectInt(node, QStringLiteral("Hotcue"), &ok);
    if (ok && hotcue > 0) {
        m_hotcue = hotcue - 1;
    } else {
        SKIN_WARNING(node, context) << "Hotcue value invalid";
    }

    bool okay;
    m_cueColorDimThreshold = context.selectInt(node, QStringLiteral("DimBrightThreshold"), &okay);
    if (!okay) {
        m_cueColorDimThreshold = kDefaultDimBrightThreshold;
    }

    m_hoverCueColor = context.selectBool(node, QStringLiteral("Hover"), false);

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
            createConfigKey(QStringLiteral("activate")),
            nullptr,
            ControlParameterWidgetConnection::DIR_FROM_WIDGET,
            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
    addLeftConnection(pLeftConnection);

    auto* pDisplayConnection = new ControlParameterWidgetConnection(
            this,
            createConfigKey(QStringLiteral("enabled")),
            nullptr,
            ControlParameterWidgetConnection::DIR_TO_WIDGET,
            ControlParameterWidgetConnection::EMIT_NEVER);
    addConnection(pDisplayConnection);
    setDisplayConnection(pDisplayConnection);

    QDomNode con = context.selectNode(node, QStringLiteral("Connection"));
    if (!con.isNull()) {
        SKIN_WARNING(node, context) << "Additional Connections are not allowed";
    }
}

void WHotcueButton::mousePressEvent(QMouseEvent* e) {
    const bool rightClick = e->button() == Qt::RightButton;
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
            if (e->modifiers().testFlag(Qt::ShiftModifier)) {
                pTrack->removeCue(pHotCue);
                return;
            }
            m_pCueMenuPopup->setTrackAndCue(pTrack, pHotCue);
            // use the bottom left corner as starting point for popup
            m_pCueMenuPopup->popup(mapToGlobal(QPoint(0, height())));
        }
        return;
    }

    // Pass all other press events to the base class.
    WPushButton::mousePressEvent(e);
}

void WHotcueButton::mouseReleaseEvent(QMouseEvent* e) {
    const bool rightClick = e->button() == Qt::RightButton;
    if (rightClick) {
        // Don't handle stray release events
        return;
    }
    WPushButton::mouseReleaseEvent(e);
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
    case mixxx::CueType::AudibleSound:
        m_type = QStringLiteral("audiblesound");
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
