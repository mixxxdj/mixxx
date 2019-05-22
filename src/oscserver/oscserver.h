#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include "util/memory.h"

#include <QRegularExpression>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "lo/lo.h"
#include "oscserver/defs_oscserver.h"
#include "preferences/usersettings.h"

class OscServer : public QObject {
  Q_OBJECT

public:
  OscServer(UserSettingsPointer pConfig);
  virtual ~OscServer();

  bool init();
  void quit();

private:
  void setNeedsUpdate(bool needsUpdate);
  // Sets whether to display a warning (error = true) or a success message
  // (error = false) on the OSC preferences page
  void setError(bool error);

private slots:
  void slotNeedsUpdate(double needsUpdate);

private:
  UserSettingsPointer m_pConfig;
  std::unique_ptr<ControlProxy> m_pUpdateProxy;
  std::unique_ptr<ControlProxy> m_pErrorProxy;

  lo_server_thread m_st;
};

#endif
