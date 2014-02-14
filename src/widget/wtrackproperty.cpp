
#include <QDebug>
#include <QUrl>

#include "controlobject.h"
#include "widget/wtrackproperty.h"

WTrackProperty::WTrackProperty(const char* group, ConfigObject<ConfigValue>* pConfig, QWidget* pParent)
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
        QList<QUrl> locationUrls;
        locationUrls.append(QUrl::fromLocalFile(m_pCurrentTrack->getLocation()));

        QMimeData* mimeData = new QMimeData();
        mimeData->setUrls(locationUrls);

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap(":images/library/ic_library_drag_and_drop.png"));
        drag->exec(Qt::CopyAction);
    }
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() &&
            event->mimeData()->urls().size() > 0) {
        // Accept if the Deck isn't playing or the settings allow to interrupt a playing deck
        if ((!ControlObject::get(ConfigKey(m_pGroup, "play")) ||
             m_pConfig->getValueString(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck")).toInt())) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
}

void WTrackProperty::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls() &&
            event->mimeData()->urls().size() > 0) {
        QUrl url = event->mimeData()->urls().first();
        QString filename = url.toLocalFile();
        // If the file is on a network share, try just converting the URL to a string
        if (filename == "") {
            filename = url.toString();
        }
        event->accept();
        emit(trackDropped(filename, m_pGroup));
    } else {
        event->ignore();
    }
}
