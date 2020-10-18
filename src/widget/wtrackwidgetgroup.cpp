
#include "widget/wtrackwidgetgroup.h"

#include <QDebug>
#include <QStylePainter>
#include <QUrl>

#include "control/controlobject.h"
#include "track/track.h"
#include "util/dnd.h"
#include "widget/wtrackmenu.h"

namespace {

constexpr int kDefaultTrackColorAlpha = 255;

constexpr WTrackMenu::Features kTrackMenuFeatures =
        WTrackMenu::Feature::SearchRelated |
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
        Library* pLibrary,
        const QString& group)
        : WWidgetGroup(pParent),
          m_group(group),
          m_pConfig(pConfig),
          m_trackColorAlpha(kDefaultTrackColorAlpha),
          m_pTrackMenu(make_parented<WTrackMenu>(
                  this, pConfig, pLibrary, kTrackMenuFeatures)) {
    setAcceptDrops(true);
}

WTrackWidgetGroup::~WTrackWidgetGroup() = default;

void WTrackWidgetGroup::setup(const QDomNode& node, const SkinContext& context) {
    WWidgetGroup::setup(node, context);

    bool ok = false;
    int trackColorAlpha = context.selectInt(
            node,
            QStringLiteral("TrackColorAlpha"),
            &ok);
    if (ok && trackColorAlpha >= 0 && trackColorAlpha <= 255) {
        m_trackColorAlpha = trackColorAlpha;
    }
}

void WTrackWidgetGroup::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pCurrentTrack = pTrack;
    connect(pTrack.get(),
            &Track::changed,
            this,
            &WTrackWidgetGroup::slotTrackChanged);
    updateColor();
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
        if (m_trackColor.isValid()) {
            m_trackColor.setAlpha(m_trackColorAlpha);
        }
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

void WTrackWidgetGroup::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons().testFlag(Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_group);
    }
}

void WTrackWidgetGroup::dragEnterEvent(QDragEnterEvent* event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
}

void WTrackWidgetGroup::dropEvent(QDropEvent* event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
}

void WTrackWidgetGroup::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    if (m_pCurrentTrack) {
        m_pTrackMenu->loadTrack(m_pCurrentTrack);
        // Create the right-click menu
        m_pTrackMenu->popup(event->globalPos());
    }
}
