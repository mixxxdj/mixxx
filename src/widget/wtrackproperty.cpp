
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
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
    }
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent *event) {
    if (DragAndDropHelper::allowLoadToPlayer(m_pGroup, m_pConfig) &&
            DragAndDropHelper::dragEnterAccept(*event->mimeData(), m_pGroup,
                                               true, false)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void WTrackProperty::dropEvent(QDropEvent *event) {
    if (DragAndDropHelper::allowLoadToPlayer(m_pGroup, m_pConfig)) {
        QList<QFileInfo> files = DragAndDropHelper::dropEventFiles(
                *event->mimeData(), m_pGroup, true, false);
        if (!files.isEmpty()) {
            event->accept();
            emit(trackDropped(files.at(0).canonicalFilePath(), m_pGroup));
            return;
        }
    }
    event->ignore();
}
