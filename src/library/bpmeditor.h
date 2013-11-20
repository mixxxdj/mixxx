#ifndef BPMEDITOR_H
#define BPMEDITOR_H

#include <QWidget>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class BPMEditor : public QWidget {
    Q_OBJECT
  public:
    enum EditMode {Editable, ReadOnly};
    BPMEditor(EditMode mode, QWidget *parent =NULL);
    ~BPMEditor();

    void setData(const QModelIndex &index, int lockColumn);
    bool getLock();
    double getBPM();

  signals:
    void finishedEditing();

  private:
    QPushButton *m_pLock;
    QDoubleSpinBox *m_pBPMSpinBox;
    QLabel *m_pBPMLabel;
    QHBoxLayout *m_pLayout;
};

#endif // BPMEDITOR_H
