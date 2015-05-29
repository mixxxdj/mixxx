#include <QStyledItemDelegate>

class CrateHighlightDelegate : public QStyledItemDelegate {

public:
    CrateHighlightDelegate(QObject* parent = 0);

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const;
};
