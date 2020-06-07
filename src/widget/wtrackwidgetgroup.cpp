
#include "widget/wtrackwidgetgroup.h"

#include <QDebug>
#include <QStylePainter>
#include <QUrl>

#include "control/controlobject.h"
#include "widget/wtrackmenu.h"

namespace {

int kDefaultTrackColorAplha = 255;

const WTrackMenu::Features kTrackMenuFeatures =
        WTrackMenu::Feature::Playlist |
        WTrackMenu::Feature::Crate |
        WTrackMenu::Feature::Metadata |
        WTrackMenu::Feature::Reset |
        WTrackMenu::Feature::BPM |
        WTrackMenu::Feature::Color |
        WTrackMenu::Feature::FileBrowser |
        WTrackMenu::Feature::Properties;

} // anonymous namespace

WTrackWidgetGroup::WTrackWidgetGroup(QWidget* pParent,
        UserSettingsPointer pConfig,
        TrackCollectionManager* pTrackCollectionManager)
        : WWidgetGroup(pParent),
          m_trackColorAlpha(kDefaultTrackColorAplha),
          m_pTrackMenu(make_parented<WTrackMenu>(
                  this, pConfig, pTrackCollectionManager, kTrackMenuFeatures)) {
}

void WTrackWidgetGroup::setup(const QDomNode& node, const SkinContext& context) {
    WWidgetGroup::setup(node, context);

    bool ok = false;
    int trackColorAlpha = context.selectInt(
            node,
            QStringLiteral("TrackColorAlpha"),
            &ok);
    if (ok) {
        m_trackColorAlpha = trackColorAlpha;
    }
}

void WTrackWidgetGroup::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.get(),
                &Track::changed,
                this,
                &WTrackWidgetGroup::slotTrackChanged);
        updateColor();
    }
}

void WTrackWidgetGroup::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateColor();
}

void WTrackWidgetGroup::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateColor();
}

void WTrackWidgetGroup::updateColor() {
    if (m_pCurrentTrack) {
        m_trackColor = mixxx::RgbColor::toQColor(m_pCurrentTrack->getColor());
        m_trackColor.setAlpha(m_trackColorAlpha);
    } else {
        m_trackColor = QColor();
    }
    update();
}

void WTrackWidgetGroup::paintEvent(QPaintEvent* pe) {
    WWidgetGroup::paintEvent(pe);

    if (m_trackColor.isValid()) {
        QStylePainter p(this);

        p.fillRect(rect(), QBrush(m_trackColor));
    }
}

void WTrackWidgetGroup::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    if (m_pCurrentTrack) {
        m_pTrackMenu->loadTrack(m_pCurrentTrack);
        // Create the right-click menu
        m_pTrackMenu->popup(event->globalPos());
    }
}
