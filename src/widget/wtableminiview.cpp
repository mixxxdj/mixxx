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
    
    int size = m_pModel->rowCount();
    QVector<QPair<QChar, int> > letters;
    
    for (int i = 0; i < size; ++i) {
        const QModelIndex& index = m_pModel->index(i, m_sortColumn);
        QString text = index.data().toString();
        QChar c = StringHelper::getFirstCharForGrouping(text);
        
        if (letters.size() <= 0) {
            letters.append(qMakePair(c, 1));
        } else {
            QPair<QChar, int> &last = letters.last();
            
            if (last.first == c) {
                ++last.second;
            } else {
                letters.append(qMakePair(c, 1));
            }
        }
    }
    
    setLetters(letters);
}

