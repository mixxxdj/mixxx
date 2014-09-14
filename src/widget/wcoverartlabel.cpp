#include "wcoverartlabel.h"

WCoverArtLabel::WCoverArtLabel(QWidget* parent)
        : QLabel(parent),
          m_coverInfo(CoverInfo()) {
}

WCoverArtLabel::~WCoverArtLabel() {
}

void WCoverArtLabel::setCoverArt(CoverInfo info, QPixmap pixmap) {
    m_coverInfo = info;
    QPixmap scaled = pixmap.scaled(100, 100,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    setPixmap(scaled);
    QSize frameSize = scaled.size();
    frameSize += QSize(2,2); // margin
    setMinimumSize(frameSize);
    setMaximumSize(frameSize);
}

void WCoverArtLabel::enterEvent(QEvent*) {
    DlgCoverArtFullSize::instance()->init(m_coverInfo);
}

void WCoverArtLabel::leaveEvent(QEvent*) {
    DlgCoverArtFullSize::instance()->close();
}
