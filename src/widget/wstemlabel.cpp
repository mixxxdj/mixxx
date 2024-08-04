#include "wstemlabel.h"

#include "moc_wstemlabel.cpp"

WStemLabel::WStemLabel(QWidget* pParent)
        : WLabel(pParent),
          m_stemInfo(QString(), QColor()),
          m_stemNo(0) {
}

void WStemLabel::setup(const QDomNode& node, const SkinContext& context) {
    m_stemNo = context.selectInt(node, "StemNum");
}

void WStemLabel::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    m_stemInfo = StemInfo();
    updateLabel();
}

void WStemLabel::slotTrackLoaded(TrackPointer track) {
    if (!track) {
        return;
    }

    auto stemInfo = track->getStemInfo();

    if (stemInfo.isEmpty()) {
        return;
    }

    m_stemInfo = stemInfo[m_stemNo - 1];
    updateLabel();
}

void WStemLabel::updateLabel() {
    QColor color = m_stemInfo.getColor();
    QString text = m_stemInfo.getLabel();
    setTextColor(color);
    setLabelText(text);
}

void WStemLabel::setTextColor(const QColor& color) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, color);
    this->setPalette(palette);
}

void WStemLabel::setLabelText(const QString& text) {
    this->setText(text);
}
