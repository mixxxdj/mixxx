
#include "widget/wtracktext.h"

WTrackText::WTrackText(QWidget* pParent)
        : WLabel(pParent) {

}

WTrackText::~WTrackText() {

}

void WTrackText::slotTrackLoaded(TrackPointer track) {
    m_pLabel->setText(track->getInfo());
}

void WTrackText::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    m_pLabel->setText("");
}
