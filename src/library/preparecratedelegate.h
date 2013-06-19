#include <QItemDelegate>
class QPixmap;

class PrepareCrateDelegate : public QItemDelegate {
    public:
PrepareCrateDelegate(QObject* parent);
~PrepareCrateDelegate();
void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    private:
        QPixmap* m_pCratePixmap;
};
