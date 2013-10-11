#ifndef PROPERTYBINDER_H
#define PROPERTYBINDER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QVariant>

#include "configobject.h"

class ControlObject;
class ControlObjectThread;

class PropertyBinder : public QObject {
    Q_OBJECT
  public:
    PropertyBinder(QWidget* pWidget, QString propertyName,
            ControlObject* pControl, ConfigObject<ConfigValue>* pConfig);
    virtual ~PropertyBinder();
  private slots:
    void slotValueChanged(double dValue);
  private:
    QString m_propertyName;
    QWidget* m_pWidget;
    ControlObjectThread* m_pControlThread;
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif /* PROPERTYBINDER_H */
