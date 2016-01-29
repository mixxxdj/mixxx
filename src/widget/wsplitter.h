#ifndef WSPLITTER_H
#define WSPLITTER_H

#include <QDomNode>
#include <QEvent>
#include <QSplitter>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class WSplitter : public QSplitter, public WBaseWidget {
    Q_OBJECT
  public:
    WSplitter(QWidget* pParent, UserSettingsPointer pConfig);
    virtual ~WSplitter();

    void setup(QDomNode node, const SkinContext& context);

  protected:
    bool event(QEvent* pEvent);

  private slots:
    void slotSplitterMoved();

  private:
    UserSettingsPointer m_pConfig;
    ConfigKey m_configKey;
};

#endif /* WSPLITTER_H */
