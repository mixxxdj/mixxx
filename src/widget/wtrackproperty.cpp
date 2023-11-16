#include "widget/wtrackproperty.h"

#include <QDebug>

#include "moc_wtrackproperty.cpp"
#include "skin/legacy/skincontext.h"
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
        WTrackMenu::Feature::Analyze |
        WTrackMenu::Feature::BPM |
        WTrackMenu::Feature::Color |
        WTrackMenu::Feature::RemoveFromDisk |
        WTrackMenu::Feature::FileBrowser |
        WTrackMenu::Feature::Properties |
        WTrackMenu::Feature::UpdateReplayGainFromPregain |
        WTrackMenu::Feature::FindOnWeb |
        WTrackMenu::Feature::SelectInLibrary;
} // namespace

WTrackProperty::WTrackProperty(
        QWidget* pParent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        const QString& group)
        : WLabel(pParent),
          m_group(group),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary) {
    setAcceptDrops(true);
}

WTrackProperty::~WTrackProperty() {
    // Required to allow forward declaration of WTrackMenu in header
}

void WTrackProperty::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    m_property = context.selectString(node, "Property");

    // Check if property with that name exists in Track class
    if (Track::staticMetaObject.indexOfProperty(m_property.toUtf8().constData()) == -1) {
        qWarning() << "WTrackProperty: Unknown track property:" << m_property;
    }
}

void WTrackProperty::slotTrackLoaded(TrackPointer pTrack) {
    if (!pTrack) {
        return;
    }
    m_pCurrentTrack = pTrack;
    connect(pTrack.get(),
            &Track::changed,
            this,
            &WTrackProperty::slotTrackChanged);
    updateLabel();
}

void WTrackProperty::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateLabel();
}

void WTrackProperty::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
}

void WTrackProperty::updateLabel() {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toUtf8().constData());
        if (property.isValid() && property.canConvert<QString>()) {
            setText(property.toString());
            return;
        }
    }
    setText("");
}

void WTrackProperty::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons().testFlag(Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_group);
    }
}

void WTrackProperty::mouseDoubleClickEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    if (m_pCurrentTrack) {
        ensureTrackMenuIsCreated();
        m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
        m_pTrackMenu->showDlgTrackInfo(m_property);
    }
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent* event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_group, m_pConfig);
}

void WTrackProperty::dropEvent(QDropEvent* event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_group, m_pConfig);
}

void WTrackProperty::contextMenuEvent(QContextMenuEvent* event) {
    event->accept();
    if (m_pCurrentTrack) {
        ensureTrackMenuIsCreated();
        m_pTrackMenu->loadTrack(m_pCurrentTrack, m_group);
        // Create the right-click menu
        m_pTrackMenu->popup(event->globalPos());
    }
}

void WTrackProperty::ensureTrackMenuIsCreated() {
    if (m_pTrackMenu.get() == nullptr) {
        m_pTrackMenu = make_parented<WTrackMenu>(
                this, m_pConfig, m_pLibrary, kTrackMenuFeatures);
    }
}
