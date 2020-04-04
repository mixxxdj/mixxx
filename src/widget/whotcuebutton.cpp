#include "widget/whotcuebutton.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "mixer/playerinfo.h"

WHotcueButton::WHotcueButton(QWidget* pParent)
        : WPushButton(pParent),
          m_hotcue(Cue::kNoHotCue),
          m_hoverCueColor(false),
          m_pCoColor(nullptr),
          m_cueColorDimmed(false),
          m_isCueColorLight(false),
          m_isCueColorDark(false) {
}

void WHotcueButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    m_group = context.selectString(node, QStringLiteral("Group"));
    bool ok;
    int hotcue = context.selectInt(node, QStringLiteral("Hotcue"), &ok);
    if (ok && hotcue > 0) {
        m_hotcue = hotcue - 1;
    } else {
        SKIN_WARNING(node, context) << "Hotcue value invalid";
    }
    m_hoverCueColor = context.selectBool(node, QStringLiteral("Hover"), false);

    m_pCueMenuPopup = make_parented<WCueMenuPopup>(context.getConfig(), this);
    ColorPaletteSettings colorPaletteSettings(context.getConfig());
    auto colorPalette = colorPaletteSettings.getHotcueColorPalette();
    m_pCueMenuPopup->setColorPalette(colorPalette);

    setFocusPolicy(Qt::NoFocus);

    m_pCoColor = make_parented<ControlProxy>(createConfigKey(QStringLiteral("color")), this);
    m_pCoColor->connectValueChanged(this, &WHotcueButton::slotColorChanged);
    slotColorChanged(m_pCoColor->get());

    auto pLeftConnection = new ControlParameterWidgetConnection(
            this,
            createConfigKey(QStringLiteral("activate")),
            nullptr,
            ControlParameterWidgetConnection::DIR_FROM_WIDGET,
            ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE);
    addLeftConnection(pLeftConnection);

    auto pDisplayConnection = new ControlParameterWidgetConnection(
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
        if (readDisplayValue() == 1) {
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
            m_pCueMenuPopup->setTrackAndCue(pTrack, pHotCue);
            m_pCueMenuPopup->popup(e->globalPos());
        }
        return;
    }

    // Pass all other press events to the base class.
    WPushButton::mousePressEvent(e);
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
    QColor cueColor = QColor::fromRgb(color);
    m_cueColorDimmed = Color::isDimmColor(cueColor);

    QString style =
            QStringLiteral("WWidget[displayValue=\"1\"] { background-color: ") +
            cueColor.name() +
            QStringLiteral("; }");

    if (m_hoverCueColor) {
        style +=
                QStringLiteral("WWidget[displayValue=\"1\"]:hover { background-color: ") +
                cueColor.lighter(m_cueColorDimmed ? 120 : 80).name() +
                QStringLiteral("; }");
    }

    setStyleSheet(style);
    restyleAndRepaint();
}

void WHotcueButton::restyleAndRepaint() {
    if (readDisplayValue()) {
        // Adjust properties for Qss file
        m_isCueColorLight = !m_cueColorDimmed;
        m_isCueColorDark = m_cueColorDimmed;
    } else {
        // We are now at the background set by qss.
        // Since we don't know the color reset both
        m_isCueColorLight = false;
        m_isCueColorDark = false;
    }
    WPushButton::restyleAndRepaint();
}
