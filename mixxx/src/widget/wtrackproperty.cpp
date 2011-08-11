#include "widget/wtrackproperty.h"

WTrackProperty::WTrackProperty(QWidget* pParent)
        : WLabel(pParent) {

}

WTrackProperty::~WTrackProperty() {

}

void WTrackProperty::setup(QDomNode node) {
    WLabel::setup(node);

    m_property = selectNodeQString(node, "Property");
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
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), 0, this, 0);
    }
    m_pCurrentTrack.clear();
    m_pLabel->setText("");
}

void WTrackProperty::updateLabel(TrackInfoObject*) {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toAscii().constData());
        if (property.isValid() && qVariantCanConvert<QString>(property)) {
            m_pLabel->setText(property.toString());
        }
    }
}
