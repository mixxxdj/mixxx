#include <QDebug>
#include <QStylePainter>
#include <QPaintEvent>
#include <QStyleOptionSlider>

#include "library/columncache.h"
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

void WMiniViewScrollBar::refreshCharMap() {    
    if (m_pModel.isNull()) {
        return;
    }
    
    int size = m_pModel->rowCount();
    const QModelIndex& rootIndex = m_pModel->index(0, 0);
    
    int digits = 0;
    bool isNumber;
    rootIndex.sibling(0, m_sortColumn).data(m_dataRole).toString().toInt(&isNumber);
    if (isNumber) {
        // Search the max number of digits, since we know that the model is
        // sorted then we search the first and the last element for the number
        // of digits
        QString first = rootIndex.sibling(0, m_sortColumn).data(m_dataRole).toString();
        QString last  = rootIndex.sibling(size - 1, m_sortColumn).data(m_dataRole).toString();
        
        digits = qMax(first.length(), last.length());
    }
    
    m_letters.clear();
    for (int i = 0; i < size; ++i) {
        const QModelIndex& index = rootIndex.sibling(i, m_sortColumn);
        QString text = index.data(m_dataRole).toString();
        QChar c;
        if (isNumber) {
            if (text.length() == digits) {
                c = text.at(0);
            } else {
                c = '0';
            }
        } else {
            c = StringHelper::getFirstCharForGrouping(text);
        }
        
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

void WMiniViewScrollBar::lettersPaint(QPaintEvent*) {
    QStylePainter painter(this);
    QStyleOptionSlider opt;
    opt.init(this);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation();
    opt.minimum = minimum();
    opt.maximum = maximum();
    opt.sliderPosition = sliderPosition();
    opt.sliderValue = value();
    opt.singleStep = singleStep();
    opt.pageStep = pageStep();
    painter.drawComplexControl(QStyle::CC_ScrollBar, opt);
    
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
    
    opt.rect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, 
                                       QStyle::SC_ScrollBarSlider, this);
    opt.subControls = QStyle::SC_ScrollBarSlider;
    opt.activeSubControls = QStyle::SC_ScrollBarSlider;
    painter.drawControl(QStyle::CE_ScrollBarSlider, opt);
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
                                    totalCount,
                                    totalLinearSize);
        float percentage = p.position/float(totalCount);
        
        if (optimalScrollPosition < nextAvailableScrollPosition ||
            percentage < 0.01) {
            // If a very small percentage letter takes the space for a letter
            // with more high ocupacy percentage this can be annoying
            
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

float WMiniViewScrollBar::interpolHeight(float current, float max1, float max2) {
    float res = current*(max2/max1);
    return res;
}

