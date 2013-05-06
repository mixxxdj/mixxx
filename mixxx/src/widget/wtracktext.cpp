
#include "widget/wtracktext.h"

WTrackText::WTrackText(QWidget* pParent)
        : WLabel(pParent) {

}

WTrackText::~WTrackText() {

}

void WTrackText::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
                this, SLOT(updateLabel(TrackInfoObject*)));
        updateLabel(track.data());
    }
}

void WTrackText::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), 0, this, 0);
    }
    m_pCurrentTrack.clear();
    m_pLabel->setText("");
}

void WTrackText::updateLabel(TrackInfoObject*) {
    if (m_pCurrentTrack) {
        m_pLabel->setText(m_pCurrentTrack->getInfo());
    }
}
