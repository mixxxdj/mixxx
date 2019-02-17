
#include <QDebug>
#include <QUrl>

#include "control/controlobject.h"
#include "widget/wtracktext.h"
#include "util/dnd.h"

WTrackText::WTrackText(const char *group, UserSettingsPointer pConfig, QWidget* pParent)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(pConfig) {
    setAcceptDrops(true);
}

void WTrackText::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.get(), SIGNAL(changed(Track*)),
                this, SLOT(updateLabel(Track*)));
        updateLabel(track.get());
    }
}

void WTrackText::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    setText("");
}

void WTrackText::updateLabel(Track* /*unused*/) {
    if (m_pCurrentTrack) {
        setText(m_pCurrentTrack->getInfo());
    }
}

void WTrackText::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
    }
}

void WTrackText::dragEnterEvent(QDragEnterEvent *event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_pGroup, m_pConfig);
}

void WTrackText::dropEvent(QDropEvent *event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_pGroup, m_pConfig);
}
