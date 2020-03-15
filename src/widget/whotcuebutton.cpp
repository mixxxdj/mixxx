#include "widget/whotcuebutton.h"

#include <QtDebug>
#include "mixer/playerinfo.h"

WHotcueButton::WHotcueButton(QWidget* pParent)
        : WPushButton(pParent),
          m_hotcue(Cue::kNoHotCue) {
}

void WHotcueButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    m_group = context.selectString(node, "Group");
    bool ok;
    int hotcue = context.selectString(node, "Hotcue").toInt(&ok);
    if (ok && hotcue > 0) {
        m_hotcue = hotcue - 1;
    }

    m_pCueMenuPopup = make_parented<WCueMenuPopup>(context.getConfig(), this);
    ColorPaletteSettings colorPaletteSettings(context.getConfig());
    auto colorPalette = colorPaletteSettings.getHotcueColorPalette();
    m_pCueMenuPopup->setColorPalette(colorPalette);

    setFocusPolicy(Qt::NoFocus);
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
