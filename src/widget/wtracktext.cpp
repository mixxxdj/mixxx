
#include <QDebug>
#include <QUrl>

#include "control/controlobject.h"
#include "widget/wtrackmenu.h"
#include "widget/wtracktext.h"
#include "util/dnd.h"

namespace {
const WTrackMenu::Features trackMenuFeatures =
        WTrackMenu::Feature::Playlist |
        WTrackMenu::Feature::Crate |
        WTrackMenu::Feature::Metadata |
        WTrackMenu::Feature::Reset |
        WTrackMenu::Feature::BPM |
        WTrackMenu::Feature::Color |
        WTrackMenu::Feature::FileBrowser |
        WTrackMenu::Feature::Properties;
}

WTrackText::WTrackText(QWidget* pParent,
        UserSettingsPointer pConfig,
        TrackCollectionManager* pTrackCollectionManager,
        const char* group)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(pConfig),
          m_pTrackMenu(make_parented<WTrackMenu>(
                  this, pConfig, pTrackCollectionManager, trackMenuFeatures)) {
    setAcceptDrops(true);
}

WTrackText::~WTrackText() {
    // Required to allow forward declaration of WTrackMenu in header
}

void WTrackText::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.get(),
                &Track::changed,
                this,
                &WTrackText::slotTrackChanged);
        updateLabel();
    }
}

void WTrackText::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateLabel();
}

void WTrackText::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
}

void WTrackText::updateLabel() {
    if (m_pCurrentTrack) {
        setText(m_pCurrentTrack->getInfo());
    } else {
        setText("");
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

void WTrackText::contextMenuEvent(QContextMenuEvent* event) {
    if (m_pCurrentTrack) {
        m_pTrackMenu->loadTrack(m_pCurrentTrack);
        // Create the right-click menu
        m_pTrackMenu->popup(event->globalPos());
    }
}
