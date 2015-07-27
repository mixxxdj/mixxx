#ifndef WSPLITTER_H
#define WSPLITTER_H

#include <QSplitter>
#include <QEvent>
#include <QDomNode>

#include "widget/wbasewidget.h"
#include "skin/skincontext.h"

class WSplitter : public QSplitter, public WBaseWidget {
    Q_OBJECT
  public:
    WSplitter(QWidget* pParent);
    virtual ~WSplitter();

    void setup(QDomNode node, const SkinContext& context);

  protected:
    bool event(QEvent* pEvent);
};

#endif /* WSPLITTER_H */
