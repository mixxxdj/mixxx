
#include <QDebug>
#include <QUrl>

#include "controlobject.h"
#include "widget/wtrackproperty.h"
#include "util/dnd.h"

WTrackProperty::WTrackProperty(const char* group,
                               ConfigObject<ConfigValue>* pConfig,
                               QWidget* pParent)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(pConfig) {
    setAcceptDrops(true);
}

WTrackProperty::~WTrackProperty() {
}

void WTrackProperty::setup(QDomNode node, const SkinContext& context) {
    WLabel::setup(node, context);

    m_property = context.selectString(node, "Property");
}

void WTrackProperty::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
                this, SLOT(updateLabel(TrackInfoObject*)));
        updateLabel(track.data());
    }
}

void WTrackProperty::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), 0, this, 0);
    }
    m_pCurrentTrack.clear();
    setText("");
}

void WTrackProperty::updateLabel(TrackInfoObject*) {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toAscii().constData());
        if (property.isValid() && qVariantCanConvert<QString>(property)) {
            setText(property.toString());
        }
    }
}

void WTrackProperty::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this);
    }
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent *event) {
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

void WTrackProperty::dropEvent(QDropEvent *event) {
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
