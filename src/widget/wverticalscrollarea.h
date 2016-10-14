#ifndef WVERTICALSCROLLAREA_H
#define WVERTICALSCROLLAREA_H

#include <QScrollArea>

class WVerticalScrollArea : public QScrollArea
{
    Q_OBJECT
  public:
    WVerticalScrollArea(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);

  public slots:
    void slotEnsureVisible(QWidget* widget);

  protected:
    bool eventFilter(QObject* o, QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    bool focusNextPrevChild(bool next) override;
    
  private:
    void calcSize();
};

#endif // WVERTICALSCROLLAREA_H
