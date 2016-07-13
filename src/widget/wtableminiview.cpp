#include "widget/wtableminiview.h"

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
        QChar c = getFirstChar(text);
        
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

QChar WTableMiniView::getFirstChar(const QString& text) {
    QChar c = text.at(0);                
    if (!c.isLetter()) {
        return c;
    }
    
    // This removes the accents of the characters
    // We only can remove the accents if its a latin character
    if (c.toLatin1() != 0) {
        QString s1 = text.normalized(QString::NormalizationForm_KD);
        s1.remove(QRegExp("[^a-zA-Z]"));
        
        if (s1.size() > 0) {
            c = s1.at(0).toUpper();
        }
    }
    return c;
}


