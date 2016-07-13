#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionSlider>

#include "wminiviewscrollbar.h"

WMiniViewScrollBar::WMiniViewScrollBar(QWidget* parent)
        : QScrollBar(parent),
          m_showLetters(true) {

}

void WMiniViewScrollBar::setShowLetters(bool show) {
    m_showLetters = show;
}

bool WMiniViewScrollBar::showLetters() const {
    return m_showLetters;
}

void WMiniViewScrollBar::paintEvent(QPaintEvent* event) {
    QScrollBar::paintEvent(event);

    if (m_showLetters) {
        lettersPaint(event);
    }
}

void WMiniViewScrollBar::lettersPaint(QPaintEvent*) {
    QPainter painter(this);

    QStyleOptionSlider styleOpts;
    styleOpts.init(this);
    
    // Get total count
    int totalCount = 0;
    for (int x : m_count) {
        totalCount += x;
    }

    // Get total size
    const QRect& total = styleOpts.rect;
    QPoint topLeft = total.topLeft();
    QFont f(font());
    f.setPointSize(f.pointSize()/1.5);

    for (const QChar& c : m_letters) {

        // Get letter count
        int count = m_count[c];
        int height = interpolHeight(count, 0, totalCount,
                                    total.topLeft().y(), total.bottomLeft().y());
        
        QPoint bottomRight = topLeft + QPoint(total.width(), height);
        painter.setBrush(palette().color(QPalette::Text));
        painter.setFont(f);
        painter.drawText(QRect(topLeft, bottomRight), Qt::AlignTop | Qt::AlignHCenter, QString(c));
        
        topLeft += QPoint(0, height);
    }
}

int WMiniViewScrollBar::interpolHeight(int current, int min1, int max1, int min2,
                                       int max2) {
    int aux1 = (current - min1)*(max2 - min2);
    return (aux1/(max1 - min1)) + min2;
}

