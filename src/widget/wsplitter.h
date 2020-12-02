#ifndef WSPLITTER_H
#define WSPLITTER_H

#include <QByteArrayData>
#include <QDomNode>
#include <QEvent>
#include <QSplitter>
#include <QString>

#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class QEvent;
class QObject;
class QWidget;

class WSplitter : public QSplitter, public WBaseWidget {
    Q_OBJECT
  public:
    WSplitter(QWidget* pParent, UserSettingsPointer pConfig);

    void setup(const QDomNode& node, const SkinContext& context);

  protected:
    bool event(QEvent* pEvent) override;

  private slots:
    void slotSplitterMoved();

  private:
    UserSettingsPointer m_pConfig;
    ConfigKey m_configKey;
};

#endif /* WSPLITTER_H */
