#include "widget/wtableminiview.h"

#include "util/stringhelper.h"

WTableMiniView::WTableMiniView(QWidget* parent)
        : WMiniViewScrollBar(parent) {
    
}

void WTableMiniView::setModel(QAbstractItemModel *model) {
    m_pModel = model;
    
    connect(m_pModel.data(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(refreshCharMap()));
    
    refreshCharMap();
}

void WTableMiniView::setSortColumn(int column) {
    m_sortColumn = column;
    refreshCharMap();
}

void WTableMiniView::refreshCharMap() {
    if (m_pModel.isNull()) {
        return;
    }
    
    m_count.clear();
    int size = m_pModel->rowCount();
    
    for (int i = 0; i < size; ++i) {
        const QModelIndex& index = m_pModel->index(i, m_sortColumn);
        QString text = index.data().toString();
        if (text.isEmpty()) {
            continue;
        }
        QChar c = StringHelper::getFirstChar(text);
        
        // Add character to letters order vector
        if (m_letters.size() <= 0 || c != m_letters.last()) {
            m_letters.append(c);
        }
        
        // Add character to character map
        auto it = m_count.find(c);
        if (it == m_count.end()) {
            m_count.insert(c, 1);
        } else {
            ++(*it);
        }
    }
    
    update();
}

