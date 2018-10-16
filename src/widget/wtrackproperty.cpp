
#include <QDebug>
#include <QUrl>

#include "control/controlobject.h"
#include "widget/wtrackproperty.h"
#include "util/dnd.h"

WTrackProperty::WTrackProperty(const char* group,
                               UserSettingsPointer pConfig,
                               QWidget* pParent)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(pConfig) {
    setAcceptDrops(true);
}

void WTrackProperty::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    m_property = context.selectString(node, "Property");
}

void WTrackProperty::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.get(), SIGNAL(changed(Track*)),
                this, SLOT(updateLabel(Track*)));
        updateLabel(track.get());
    }
}

void WTrackProperty::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    setText("");
}

void WTrackProperty::updateLabel(Track* /*unused*/) {
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
            emit(trackDropped(files.at(0).absoluteFilePath(), m_pGroup));
            return;
        }
    }
    event->ignore();
}
