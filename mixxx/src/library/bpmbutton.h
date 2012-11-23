#ifndef BPMBUTTON_H
#define BPMBUTTON_H

#include <QAbstractButton>
#include <QPixmap>
#include <QPaintEvent>

class BPMButton : public QAbstractButton
{
    Q_OBJECT
    
  public:
    BPMButton(QWidget *parent=0);
    ~BPMButton();
    void setCheckedImage(QPixmap &image);
    void setUncheckedImage(QPixmap &image);
    QSize sizeHint() const;
  protected:
    void paintEvent(QPaintEvent *e);
  private:
    QPixmap m_Checked;
    QPixmap m_Unchecked;
};

#endif //BPMBUTTON_H
