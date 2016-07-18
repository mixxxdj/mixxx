#include <QDebug>
#include <QStylePainter>
#include <QPaintEvent>
#include <QStyleOptionSlider>

#include "library/basesqltablemodel.h"
#include "library/columncache.h"
#include "util/stringhelper.h"

#include "wminiviewscrollbar.h"

namespace {
float interpolSize(float current, float max1, float max2) {
    float res = current*(max2/max1);
    return res;
}
}

WMiniViewScrollBar::WMiniViewScrollBar(QWidget* parent)
        : QScrollBar(parent),
          m_sortColumn(-1),
          m_dataRole(Qt::DisplayRole),
          m_showLetters(true) {
    setMouseTracking(true);
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

int WMiniViewScrollBar::sortColumn() const {
    return m_sortColumn;
}

void WMiniViewScrollBar::setRole(int role) {
    m_dataRole = role;
    triggerUpdate();
}

int WMiniViewScrollBar::role() const {
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
    if (!m_showLetters) {
        QScrollBar::paintEvent(event);
        return;
    }
    
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
    int flags = Qt::AlignTop | Qt::AlignHCenter;

    // Get total size
    int letterSize = fontMetrics().height();
    int w = width();
    
    // Draw each letter in its position
    for (const CharPosition& p : m_computedPosition) {
        if (p.position < 0) {
            continue;
        }
        QFont f(font());
        f.setBold(p.bold);
        painter.setFont(f);
        
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

void WMiniViewScrollBar::resizeEvent(QResizeEvent* pEvent) {
    computeLettersSize();
    QScrollBar::resizeEvent(pEvent);
}

void WMiniViewScrollBar::mouseMoveEvent(QMouseEvent* pEvent) {
    // Check mouse hover condition
    
    bool found = false;
    int letterSize = fontMetrics().height();
    int posVert = pEvent->pos().y();

    for (CharPosition& c : m_computedPosition) {
        if (posVert >= c.position && posVert < c.position + letterSize && !found) {
            c.bold = true;
            found = true;
        } else {
            c.bold = false;
        }
    } 
    update();
    QScrollBar::mouseMoveEvent(pEvent);
}

void WMiniViewScrollBar::mousePressEvent(QMouseEvent* pEvent) {
    QScrollBar::mousePressEvent(pEvent);
    
    bool found = false;
    auto itL = m_letters.begin();
    auto itC = m_computedPosition.begin();
    int totalSum = 0;
    
    // When setting the slider position the correct position is the sum of all
    // the previous elements of the selected element. 
    // In this scrollbar maximum = model.size(), minimum = 0
    while (!found && itL != m_letters.end() && itC != m_computedPosition.end()) {
        if (itC->bold) {
            found = true;
            setSliderPosition(totalSum);
        } else {
            totalSum += itL->count;
        }
        ++itL;
        ++itC;
    }
}

void WMiniViewScrollBar::leaveEvent(QEvent* pEvent) {
    for (CharPosition& c : m_computedPosition) {
        c.bold = false;
    }
    QScrollBar::leaveEvent(pEvent);
}

void WMiniViewScrollBar::refreshCharMap() {    
    if (m_pModel.isNull()) {
        return;
    }
    
    int size = m_pModel->rowCount();
    setMinimum(0);
    setMaximum(size);
    const QModelIndex& rootIndex = m_pModel->index(0, 0);
    
    int digits = 0;
    bool isNumber;
    rootIndex.sibling(0, m_sortColumn).data(m_dataRole).toString().toInt(&isNumber);
    if (isNumber) {
        // Search the max number of digits, since we know that the model is
        // sorted then we search the first and the last element for the number
        // of digits
        const QModelIndex& firstIndex = rootIndex.sibling(0, m_sortColumn);
        QString first = firstIndex.data(m_dataRole).toString();
        const QModelIndex& lastIndex = rootIndex.sibling(size - 1, m_sortColumn);
        QString last  = lastIndex.data(m_dataRole).toString();
        
        digits = qMax(first.length(), last.length());
    }
    
    bool isYear = false;
    QPointer<BaseSqlTableModel> pModel = qobject_cast<BaseSqlTableModel*>(m_pModel);
    if (!pModel.isNull()) {
        int index = pModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
        isYear = index == m_sortColumn;
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
        } else if (isYear && text.size() >= 3) {
            // 4 digits year, we take the decade which is the most 
            // representative in one single digit show
            c = text.at(2);
        } else {
            c = StringHelper::getFirstCharForGrouping(text);
        }
        
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
    // Initialize each position with the times each character appears
    m_computedPosition.resize(m_letters.size());
    for (int i = 0; i < m_letters.size(); ++i) {
        m_computedPosition[i].bold = false;
        m_computedPosition[i].position = m_letters[i].count;
        m_computedPosition[i].character = m_letters[i].character;
    }
    
    // Height of a letter
    const int letterSize = fontMetrics().height();
    const int totalLinearSize = rect().height();
    float nextAvailableScrollPosition = 0.0;
    float optimalScrollPosition = 0.0;
    
    int totalCount = 0;
    // Get the total count of letters appearance to make a linear interpolation
    // with the current widget height
    for (const CharCount& p : m_letters) {
        totalCount += p.count;
    }
    
    for (CharPosition& p : m_computedPosition) {
        float size = interpolSize(p.position, totalCount, totalLinearSize);
        float percentage = 100.0*p.position/float(totalCount);
        
        if ((optimalScrollPosition + size) < (nextAvailableScrollPosition + letterSize) 
                || percentage < 1.0) {
            // If a very small percentage letter takes the space for a letter
            // with more high ocupacy percentage this can be annoying
            
            // A negative position won't paint the character
            p.position = -1;
        } else {
            p.position = (int) optimalScrollPosition;
            nextAvailableScrollPosition = optimalScrollPosition + (float)(letterSize);
        }
        optimalScrollPosition += size;
    }
}

void WMiniViewScrollBar::triggerUpdate() {
    refreshCharMap();
    computeLettersSize();
    update();
}
