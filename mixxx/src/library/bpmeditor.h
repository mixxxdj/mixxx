#ifndef BPMEDITOR_H
#define BPMEDITOR_H

#include <QWidget>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QHBoxLayout>

#include "bpmbutton.h"

class BPMEditor : public QWidget{
    Q_OBJECT
  public:
    enum EditMode {Editable, ReadOnly};
    BPMEditor(const QStyleOptionViewItem& option,EditMode mode, QWidget *parent =0);
    ~BPMEditor();

    void setData(const QModelIndex &index, int lockColumn);
    bool getLock();
    double getBPM();

  signals:
    void finishedEditing();

  private:
    BPMButton *m_pLock;
    QDoubleSpinBox *m_pBPM;
    QHBoxLayout *m_pLayout;
    bool m_isSelected;
};

#endif // BPMEDITOR_H
