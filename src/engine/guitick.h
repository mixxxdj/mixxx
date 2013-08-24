#ifndef GUITICK_H
#define GUITICK_H

#include <QObject>

class GuiTick : public QObject {
    Q_OBJECT
  public:
    GuiTick(QObject* pParent=NULL);
    ~GuiTick();
    void process();
};

#endif // GUITICK_H
