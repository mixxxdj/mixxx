#ifndef OscClientManager_H
#define OscClientManager_H

#include <QDateTime>
#include <QObject>
#include <QString>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "preferences/usersettings.h"
//#include "oscclient/defs_oscclient.h"
#include <QList>
#include <QTime>
#include <QTimer>

class EngineMaster;

class OscClientManager : public QObject {
  Q_OBJECT
public:
  OscClientManager(UserSettingsPointer &pConfig, EngineMaster *pEngine);
  virtual ~OscClientManager();
public slots:
  void sendState();
  void maybeSendState();
  void connectServer();

private:
  QTimer timer;
};

#endif // OscClientManager_H
