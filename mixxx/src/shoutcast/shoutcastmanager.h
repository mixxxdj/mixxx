#ifndef SHOUTCASTMANAGER_H
#define SHOUTCASTMANAGER_H

#include <QObject>

#include "configobject.h"

class EngineMaster;

class ShoutcastManager : public QObject {
    Q_OBJECT
  public:
    ShoutcastManager(ConfigObject<ConfigValue>* pConfig, EngineMaster* pEngine);
    virtual ~ShoutcastManager();

    // Returns true if the Shoutcast connection is enabled. Note this only
    // indicates whether the connection is enabled, not whether it is connected.
    bool isEnabled();

  public slots:
    // Set whether or not the Shoutcast connection is enabled.
    void setEnabled(bool enabled);

  signals:
    void shoutcastEnabled(bool);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
};


#endif /* SHOUTCASTMANAGER_H */
