#include "widget/whotcuebutton.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "mixer/playerinfo.h"

WHotcueButton::WHotcueButton(QWidget* pParent)
        : WPushButton(pParent),
          m_hotcue(Cue::kNoHotCue),
          m_hoverCueColor(false),
          m_pCoColor(nullptr) {
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
    if (color < 0 && color > 0xFFFFFF) {
        return;
    }
    m_cueColor = QColor::fromRgb(color);
    updateStyleSheet();
}

void WHotcueButton::setLightTextColorChanged(QColor color) {
    if (m_lightTextColor != color) {
        m_lightTextColor = color;
        updateStyleSheet();
    }
}

void WHotcueButton::setDarkTextColorColorChanged(QColor color) {
    if (m_darkTextColor != color) {
        m_darkTextColor = color;
        updateStyleSheet();
    }
}

void WHotcueButton::updateStyleSheet() {
    if (!m_cueColor.isValid()) {
        return;
    }
    QColor textColor = Color::isDimmColor(m_cueColor) ? m_lightTextColor : m_darkTextColor;

    QString style =
            QStringLiteral("WWidget[displayValue=\"1\"] { background-color: ") +
            m_cueColor.name() +
            (textColor.isValid() ? (QStringLiteral("; color: ") +
                                           textColor.name())
                                 : QString()) +
            QStringLiteral("; }");

    if (m_hoverCueColor) {
        style +=
                QStringLiteral("WWidget[displayValue=\"1\"]:hover { background-color: ") +
                m_cueColor.lighter().name() +
                (textColor.isValid() ? (QStringLiteral("; color: ") +
                                               textColor.name())
                                     : QString()) +
                QStringLiteral("; }");
    }

    setStyleSheet(style);
}
