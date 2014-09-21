
#include <QDebug>
#include <QUrl>

#include "controlobject.h"
#include "widget/wtracktext.h"
#include "util/dnd.h"

WTrackText::WTrackText(const char *group, ConfigObject<ConfigValue> *pConfig, QWidget* pParent)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(pConfig) {
    setAcceptDrops(true);
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
    setText("");
}

void WTrackText::updateLabel(TrackInfoObject*) {
    if (m_pCurrentTrack) {
        setText(m_pCurrentTrack->getInfo());
    }
}

void WTrackText::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this);
    }
}

void WTrackText::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() &&
            event->mimeData()->urls().size() > 0) {
        // Accept if the Deck isn't playing or the settings allow to interrupt a playing deck
        if ((!ControlObject::get(ConfigKey(m_pGroup, "play")) ||
             m_pConfig->getValueString(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck")).toInt())) {
            QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(
                event->mimeData()->urls(), true, false);
            if (!files.isEmpty()) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void WTrackText::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(
                event->mimeData()->urls(), true, false);
        if (!files.isEmpty()) {
            event->accept();
            emit(trackDropped(files.at(0).canonicalFilePath(), m_pGroup));
            return;
        }
    }
    event->ignore();
}
