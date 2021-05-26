#pragma once

#include <QtQml>
#include <memory>

#include "control/controlproxy.h"

class ControlProxyQml : public QObject {
    Q_OBJECT
// The REQUIRED flag only exists in Qt 5.14 and later.
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged REQUIRED)
    Q_PROPERTY(QString key READ getKey WRITE setKey NOTIFY keyChanged REQUIRED)
#else
    Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged)
    Q_PROPERTY(QString key READ getKey WRITE setKey NOTIFY keyChanged)
#endif
    Q_PROPERTY(QString keyValid READ isKeyValid NOTIFY keyValidChanged)
    Q_PROPERTY(QString initialized READ isInitialized NOTIFY initializedChanged)
    Q_PROPERTY(double value READ getValue WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(double parameter READ getParameter WRITE setParameter NOTIFY parameterChanged)

  public:
    ControlProxyQml(QObject* parent = nullptr);

    void setGroup(const QString& group);
    const QString& getGroup() const;

    void setKey(const QString& key);
    const QString& getKey() const;

    void setValue(double newValue);
    double getValue() const;

    void setParameter(double newValue);
    double getParameter() const;

    bool isKeyValid() const;
    bool isInitialized() const;

  signals:
    void groupChanged(const QString& group);
    void keyChanged(const QString& key);
    void keyValidChanged(bool valid);
    void initializedChanged(bool initialized);
    void valueChanged(double newValue);
    void parameterChanged(double newParameter);

  private slots:
    void slotControlProxyValueChanged(double newValue);

  private:
    // QML cannot pass arguments to C++ constructors so this class
    // needs to rely on the QML object setting the group and key
    // properties to initialize the ControlProxy.
    bool initialize();
    bool tryInitialize();

    QString m_emptyString;
    ConfigKey m_coKey;
    std::unique_ptr<ControlProxy> m_pControlProxy;
};
