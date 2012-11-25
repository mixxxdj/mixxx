#ifndef PREVIEWBUTTONDELEGATE_H
#define PREVIEWBUTTONDELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QTableView>

#include "trackinfoobject.h"

class PreviewButtonDelegate : public QStyledItemDelegate {
  Q_OBJECT

  public:
    explicit PreviewButtonDelegate(QObject *parent = NULL, int column=0);
    virtual ~PreviewButtonDelegate();

    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

  signals:
    void loadTrackToPlayer(TrackPointer Track, QString group);

  public slots:
    void cellEntered(const QModelIndex &index);

  private:
    QTableView *m_pTableView;
    QPushButton *m_pButton;
    bool m_isOneCellInEditMode;
    QPersistentModelIndex m_currentEditedCellIndex;
    int m_column;
};

#endif // PREVIEWBUTTONDELEGATE_H
