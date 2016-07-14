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

void WMiniViewScrollBar::setLetters(const QVector<QPair<QChar, int> >& letters) {
    m_lettersMutex.lock();
    m_letters = letters;
    m_lettersMutex.unlock();
    computeLettersSize();
    update();
}

void WMiniViewScrollBar::paintEvent(QPaintEvent* event) {
    QScrollBar::paintEvent(event);

    if (m_showLetters) {
        lettersPaint(event);
    }
}

void WMiniViewScrollBar::resizeEvent(QResizeEvent* pEvent) {
    computeLettersSize();
    QScrollBar::resizeEvent(pEvent);
}

void WMiniViewScrollBar::lettersPaint(QPaintEvent*) {
    QPainter painter(this);
    painter.setBrush(palette().color(QPalette::Text));
    painter.setFont(font());
    
    int flags = Qt::AlignTop | Qt::AlignHCenter;

    // Get total size
    const QRect& total = rect();
    QPoint topLeft = total.topLeft();
    
    QMutexLocker lock(&m_computeMutex);
    for (const QPair<QChar, int>& p : m_computedSize) {

        // Get letter count
        int height = p.second;
        
        QPoint bottomRight = topLeft + QPoint(total.width(), height);
        painter.drawText(QRect(topLeft, bottomRight), flags, p.first);
        
        topLeft += QPoint(0, height);
    }
}

void WMiniViewScrollBar::computeLettersSize() {
    m_lettersMutex.lock();
    const QRect& total(rect());
    
    // Height of a letter
    int letterSize = fontMetrics().height();
    int totalLetters = m_letters.size();
    bool enoughSpace = total.height() >= letterSize*totalLetters;
    const int totalLinearSize = total.height();
    int difference = 0;
    int prevSpace = 0;
    int totalCount = 0;
    // Get the total count of letters appearance to make a linear interpolation
    // with the current widget height
    for (const QPair<QChar, int>& p : m_letters) {
        totalCount += p.second;
    }
    
    QMutexLocker locker(&m_computeMutex);
    m_computedSize = m_letters;
    m_lettersMutex.unlock();
    
    if (!enoughSpace) {
        // Remove the letters from smaller letter to biggest
        // since we need to preserve the current sort order we can't sort
        // the vector by size and we'll need to find each letter one by one
        int neededSpace = abs(totalLinearSize - letterSize*totalLetters);
        
        while (neededSpace > 0.5) {
            int index = findSmallest(m_computedSize);
            neededSpace -= letterSize;
            m_computedSize.remove(index);
        }
    }
    
    int totalSizeCheck = 0;
    for (QPair<QChar, int>& p : m_computedSize) {
        int height = interpolHeight(p.second, 
                                    0, totalCount,
                                    0, totalLinearSize);
        
        int auxDifference = abs(letterSize - height);
        
        if (height >= letterSize) {
            prevSpace = qMax(0, auxDifference - difference);
            difference = qMax(0, difference - auxDifference);
        } else {
            // Add a difference to remove it when a letter is bigger than the
            // size it needs
            difference += auxDifference;
            prevSpace = 0;            
        }
        int itemSize = letterSize + prevSpace;
        totalSizeCheck += itemSize;
        p.second = itemSize;
    }
    
    if (totalSizeCheck > totalLinearSize) {
        // Do the same algorithm reversed, it should take the space from the
        // biggest elements in the other direction.
        
        difference = totalSizeCheck - totalLinearSize;
        prevSpace = 0;
        for (int i = m_computedSize.size() - 1; i >= 0; --i) {
            int height = m_computedSize[i].second;
            int auxDifference = abs(letterSize - height);
            
            if (height >= letterSize) {
                prevSpace = qMax(0, auxDifference - difference);
                difference = qMax(0, difference - auxDifference);
            } else {
                // Add a difference to remove it when a letter is bigger than the
                // size it needs
                difference += auxDifference;
                prevSpace = 0;
            }
            m_computedSize[i].second = letterSize + prevSpace;
        }
    }
}

float WMiniViewScrollBar::interpolHeight(float current, float min1, float max1, 
                                       float min2, float max2) {
    float aux1 = (current - min1)*(max2 - min2);
    float res = (aux1/(max1 - min1)) + min2;
    return res;
}

int WMiniViewScrollBar::findSmallest(const QVector<QPair<QChar, int> >& vector) {
    if (vector.size() <= 0) {
        return -1;
    }
    
    int smallestIndex = 0;
    for (int i = 1; i < vector.size(); ++i) {
        if (vector[i].second < vector[smallestIndex].second) {
            smallestIndex = i;
        }
    }
    return smallestIndex;
}

