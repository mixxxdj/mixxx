#ifndef CONTROLENCODER_H
#define CONTROLENCODER_H

#include <QByteArrayData>
#include <QString>

#include "control/controlobject.h"
#include "preferences/usersettings.h"

class ConfigKey;
class QObject;

class ControlEncoder : public ControlObject {
    Q_OBJECT
  public:
    ControlEncoder(const ConfigKey& key, bool bIgnoreNops = true);
};

#endif
