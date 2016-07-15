#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionSlider>

#include "util/stringhelper.h"

#include "wminiviewscrollbar.h"

WMiniViewScrollBar::WMiniViewScrollBar(QWidget* parent)
        : QScrollBar(parent),
          m_sortColumn(-1),
          m_dataRole(Qt::DisplayRole),
          m_showLetters(true) {
}

void WMiniViewScrollBar::setShowLetters(bool show) {
    m_showLetters = show;
}

bool WMiniViewScrollBar::showLetters() const {
    return m_showLetters;
}

void WMiniViewScrollBar::setSortColumn(int column) {
    m_sortColumn = column;
    triggerUpdate();
}

int WMiniViewScrollBar::sortColumn() {
    return m_sortColumn;
}

void WMiniViewScrollBar::setRole(int role) {
    m_dataRole = role;
    triggerUpdate();
}

int WMiniViewScrollBar::role() {
    return m_dataRole;
}

void WMiniViewScrollBar::setModel(QAbstractItemModel* model) {
    m_pModel = model;
    if (!m_pModel.isNull()) {
        connect(m_pModel.data(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(refreshCharMap()));
        
        triggerUpdate();
    }
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
    
    for (CharCount& p : m_computedSize) {

        // Get letter count
        int height = p.count;
        
        QPoint bottomRight = topLeft + QPoint(total.width(), height);
        painter.drawText(QRect(topLeft, bottomRight), flags, p.character);
        
        topLeft += QPoint(0, height);
    }
}

void WMiniViewScrollBar::refreshCharMap() {    
    if (m_pModel.isNull()) {
        return;
    }
    
    int size = m_pModel->rowCount();
    const QModelIndex& rootIndex = m_pModel->index(0, 0);
    /*bool isNumber = (m_sortColumn == ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    int digits = 0;
    if (isNumber) {
        
        // Search the max number of digits, since we know that the model is
        // sorted then we search the first and the last element for the number
        // of digits
        const QModelIndex& index = m_pModel->index()
    }*/
    
    m_letters.clear();
    for (int i = 0; i < size; ++i) {
        const QModelIndex& index = rootIndex.sibling(i, m_sortColumn);
        QString text = index.data(m_dataRole).toString();
        QChar c = StringHelper::getFirstCharForGrouping(text);
        
        if (m_letters.size() <= 0) {
            m_letters.append({c, 1});
        } else {
            CharCount& last = m_letters.last();
            
            if (last.character == c) {
                ++last.count;
            } else {
                m_letters.append({c, 1});
            }
        }
    }
}

void WMiniViewScrollBar::computeLettersSize() {
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
    for (CharCount& p : m_letters) {
        totalCount += p.count;
    }
    
    m_computedSize = m_letters;
    
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
    for (CharCount& p : m_computedSize) {
        int height = interpolHeight(p.count, 
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
        p.count = itemSize;
    }
    
    if (totalSizeCheck > totalLinearSize) {
        // Do the same algorithm reversed, it should take the space from the
        // biggest elements in the other direction.
        
        difference = totalSizeCheck - totalLinearSize;
        prevSpace = 0;
        for (int i = m_computedSize.size() - 1; i >= 0; --i) {
            int height = m_computedSize[i].count;
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
            m_computedSize[i].count = letterSize + prevSpace;
        }
    }
}

void WMiniViewScrollBar::triggerUpdate() {
    refreshCharMap();
    computeLettersSize();
    update();
}

int WMiniViewScrollBar::findSmallest(const QVector<CharCount>& vector) {
    if (vector.size() <= 0) {
        return -1;
    }
    
    int smallestIndex = 0;
    for (int i = 1; i < vector.size(); ++i) {
        if (vector[i].count < vector[smallestIndex].count) {
            smallestIndex = i;
        }
    }
    return smallestIndex;
}

float WMiniViewScrollBar::interpolHeight(float current, float min1, float max1, 
                                         float min2, float max2) {
    float aux1 = (current - min1)*(max2 - min2);
    float res = (aux1/(max1 - min1)) + min2;
    return res;
}

