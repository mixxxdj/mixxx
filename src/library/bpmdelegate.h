#ifndef BPMDELEGATE_H
#define BPMDELEGATE_H

#include <QStyledItemDelegate>

#include "bpmeditor.h"

class BPMDelegate : public QStyledItemDelegate {
  Q_OBJECT

  public:
    explicit BPMDelegate(QObject *parent = NULL,int column=0, int columnLock=0);
    virtual ~BPMDelegate();

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


  public slots:
    void commitAndCloseEditor();

  private:
    BPMEditor *m_pEditor;
    int m_column;
    int m_columnLock;
};

#endif // BUTTONCOLUMNDELEGATE_H
