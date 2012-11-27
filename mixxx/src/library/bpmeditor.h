#ifndef BPMEDITOR_H
#define BPMEDITOR_H

#include <QtGui>
#include <QDoubleSpinBox>
#include <QHBoxLayout>

#include "bpmbutton.h"

class BPMEditor : public QWidget{
    Q_OBJECT
  public:
    enum EditMode {Editable, ReadOnly};
    BPMEditor(const QStyleOptionViewItem& option, QWidget *parent =0);
    ~BPMEditor();

    void paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode,
                bool isSelected, int lockColumn, const QModelIndex &index) const;
    void setData(const QModelIndex &index, int lockColumn);

  protected:
    void paintEvent(QPaintEvent *event);

  private:
    BPMButton *m_pLock;
    QDoubleSpinBox *m_pBPM;
    QHBoxLayout *m_pLayout;
    bool m_isSelected;
};

#endif // BPMEDITOR_H
