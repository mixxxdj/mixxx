#include <QStyledItemDelegate>

class BackgroundColorDelegate : public QStyledItemDelegate {

public:
    BackgroundColorDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
};
