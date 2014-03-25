#ifndef WSPLITTER_H
#define WSPLITTER_H

#include <QDomNode>
#include <QEvent>
#include <QSplitter>

#include "configobject.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class WSplitter : public QSplitter, public WBaseWidget {
    Q_OBJECT
  public:
    WSplitter(QWidget* pParent, ConfigObject<ConfigValue> *pConfig);
    virtual ~WSplitter();

    void setup(QDomNode node, const SkinContext& context);

  protected:
    bool event(QEvent* pEvent);

  private slots:
    void slotSplitterMoved();

  private:
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif /* WSPLITTER_H */
