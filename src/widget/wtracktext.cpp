#include "widget/wtracktext.h"

#include <QDebug>
#include <QUrl>

#include "control/controlobject.h"
#include "moc_wtracktext.cpp"
#include "track/track.h"
#include "util/dnd.h"
#include "widget/wtrackmenu.h"

namespace {
constexpr WTrackMenu::Features kTrackMenuFeatures =
        WTrackMenu::Feature::SearchRelated |
        WTrackMenu::Feature::Playlist |
        WTrackMenu::Feature::Crate |
        WTrackMenu::Feature::Metadata |
        WTrackMenu::Feature::Reset |
        WTrackMenu::Feature::BPM |
        WTrackMenu::Feature::Color |
        WTrackMenu::Feature::FileBrowser |
        WTrackMenu::Feature::Properties |
        WTrackMenu::Feature::UpdateReplayGain;
} // namespace

WTrackText::WTrackText(QWidget* pParent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        const QString& group)
        : WLabel(pParent),
          m_group(group),
          m_pConfig(pConfig),
          m_pTrackMenu(make_parented<WTrackMenu>(
                  this, pConfig, pLibrary, kTrackMenuFeatures)) {
    setAcceptDrops(true);
}

WTrackText::~WTrackText() {
    // Required to allow forward declaration of WTrackMenu in header
}

void WTrackText::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pCurrentTrack = pTrack;
    connect(pTrack.get(),
            &Track::changed,
            this,
            &WTrackText::slotTrackChanged);
    updateLabel();
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
    if (event->buttons().testFlag(Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_group);
    }
}

void WTrackText::mouseDoubleClickEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    if (m_pCurrentTrack) {
        m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
        m_pTrackMenu->slotShowDlgTrackInfo();
    }
}

void WTrackText::dragEnterEvent(QDragEnterEvent *event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
}

void WTrackText::dropEvent(QDropEvent *event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
}

void WTrackText::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    if (m_pCurrentTrack) {
        m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
        // Create the right-click menu
        m_pTrackMenu->popup(event->globalPos());
    }
}
