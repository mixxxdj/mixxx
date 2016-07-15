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
    int letterSize = fontMetrics().height();
    int w = width();
    
    // Draw each letter in its position
    for (CharPosition& p : m_computedSize) {
        if (p.position < 0) {
            continue;
        }
        
        QPoint topLeft = QPoint(0, p.position);
        QPoint bottomRight = topLeft + QPoint(w, letterSize);
        painter.drawText(QRect(topLeft, bottomRight), flags, p.character);
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
            CharPosition& last = m_letters.last();
            
            if (last.character == c) {
                ++last.position;
            } else {
                m_letters.append({c, 1});
            }
        }
    }
}

void WMiniViewScrollBar::computeLettersSize() {
    m_computedSize = m_letters;
    
    // Height of a letter
    int letterSize = fontMetrics().height();
    const int totalLinearSize = rect().height();
    int nextAvailableScrollPosition = 0;
    int optimalScrollPosition = 0;
    
    int totalCount = 0;
    // Get the total count of letters appearance to make a linear interpolation
    // with the current widget height
    for (CharPosition& p : m_letters) {
        totalCount += p.position;
    }
    
    for (CharPosition& p : m_computedSize) {
        int height = interpolHeight(p.position, 
                                    0, totalCount,
                                    0, totalLinearSize);
        
        if (optimalScrollPosition < nextAvailableScrollPosition) {
            // A negative position won't paint the character
            p.position = -1;
        } else {
            p.position = optimalScrollPosition;
            nextAvailableScrollPosition = optimalScrollPosition + letterSize;
        }
        optimalScrollPosition += height;
    }
}

void WMiniViewScrollBar::triggerUpdate() {
    refreshCharMap();
    computeLettersSize();
    update();
}

int WMiniViewScrollBar::findSmallest(const QVector<CharPosition>& vector) {
    if (vector.size() <= 0) {
        return -1;
    }
    
    int smallestIndex = 0;
    for (int i = 1; i < vector.size(); ++i) {
        if (vector[i].position < vector[smallestIndex].position) {
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

