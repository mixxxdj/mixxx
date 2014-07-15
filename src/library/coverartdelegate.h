#ifndef COVERARTDELEGATE_H
#define COVERARTDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>

class CoverArtDelegate : public QStyledItemDelegate {
  Q_OBJECT

  public:
    explicit CoverArtDelegate(QObject* parent = NULL);
    virtual ~CoverArtDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

  private:
    QTableView* m_pTableView;
};

#endif // COVERARTDELEGATE_H
