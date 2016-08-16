#ifndef WVERTICALSCROLLAREA_H
#define WVERTICALSCROLLAREA_H

#include <QScrollArea>

class WVerticalScrollArea : public QScrollArea
{
  public:
    WVerticalScrollArea(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);

  protected:
    virtual void resizeEvent(QResizeEvent* e);
};

#endif // WVERTICALSCROLLAREA_H
