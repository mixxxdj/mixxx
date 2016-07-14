#include "widget/wtreeminiview.h"

#include "library/treeitemmodel.h"
#include "util/stringhelper.h"

WTreeMiniView::WTreeMiniView(QWidget* parent)
        : WMiniViewScrollBar(parent) {
    
}

void WTreeMiniView::refreshCharMap() {
    if (m_pModel.isNull()) {
        return;
    }
    
    const QModelIndex& rootIndex = m_pModel->index(0, 0);
    int size = m_pModel->rowCount();
    QVector<QPair<QChar, int> > letters;
    
    for (int i = 0; i < size; ++i) {
        const QModelIndex& index = rootIndex.sibling(i, 0);
        QString text = index.data(TreeItemModel::RoleDataPath).toString();
        QChar c = StringHelper::getFirstCharForGrouping(text);
        
        if (letters.size() <= 0) {
            letters.append(qMakePair(c, 1));
        } else {
            QPair<QChar, int>& last = letters.last();
            
            if (last.first == c) {
                ++last.second;
            } else {
                letters.append(qMakePair(c, 1));
            }
        }
    }
    
    setLetters(letters);
}

