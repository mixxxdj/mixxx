#include <QPainter>
#include <QStyleOptionSlider>

#include "wminiviewscrollbar.h"

WMiniViewScrollBar::WMiniViewScrollBar(QWidget* parent)
        : QScrollBar(parent) {

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

void WMiniViewScrollBar::lettersPaint(QPaintEvent* event) {
    QPainter painter(this);

    QStyleOptionSlider style;
    style.init(this);

    style.orientation = orientation();
    style.maximum = maximum();
    style.minimum = minimum();
    style.sliderPosition = sliderPosition();
    style.sliderValue = value();
    style.singleStep = singleStep();
    style.pageStep = pageStep();

    // Get total count
    int totalCount = 0;
    for (int x : m_count) {
        totalCount += x;
    }

    // Get total size
    const QRect& total = style.rect;
    QPoint topLeft = total.topLeft();

    for (const QChar& c : m_letters) {

        // Get letter count
        int count = m_count[c];
        int height = interpolHeight(count, 0, totalCount,
                                    total.topLeft().y(), total.bottomLeft().y());
    }
}

int WMiniViewScrollBar::interpolHeight(int current, int min1, int max1, int min2,
                                       int max2) {
    int aux1 = (current - min1)*(max2 - min2);
    return (aux1/(max1 - min1)) + min2;
}

