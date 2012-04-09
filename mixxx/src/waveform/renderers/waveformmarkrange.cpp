#include "waveformmarkrange.h"

#include <QPainter>

WaveformMarkRange::WaveformMarkRange(){
    m_markStartPoint = 0;
    m_markEndPoint = 0;
    m_markEnabled = 0;
}

void WaveformMarkRange::generatePixmap(int weidth, int height) {
    m_activePixmap = QPixmap(weidth, height);
    m_disabledPixmap = QPixmap(weidth, height);

    //fill needed cause they remain transparent
    m_activePixmap.fill(QColor(0,0,0,0));
    m_disabledPixmap.fill(QColor(0,0,0,0));

    QColor activeColor = m_activeColor;
    activeColor.setAlphaF(0.3);
    QBrush brush(activeColor);

    QPainter painter;
    painter.begin(&m_activePixmap);
    painter.fillRect(m_activePixmap.rect(), brush);
    painter.end();

    QColor disabledColor = m_disabledColor;
    disabledColor.setAlphaF(0.3);
    brush = QBrush(disabledColor);

    painter.begin(&m_disabledPixmap);
    painter.fillRect(m_disabledPixmap.rect(), brush);
    painter.end();
}
