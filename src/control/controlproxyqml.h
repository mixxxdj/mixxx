#pragma once

#include <QtQml>
#include <memory>

#include "control/controlproxy.h"

class ControlProxyQml : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString group READ getGroup WRITE setGroup)
    Q_PROPERTY(QString key READ getKey WRITE setKey)
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

  signals:
    void valueChanged(double newValue);
    void parameterChanged(double newParameter);

  private slots:
    void controlProxyValueChanged(double newValue);

  private:
    // QML cannot pass arguments to C++ constructors so this class
    // needs to rely on the QML object setting the group and key
    // properties to initialize the ControlProxy.
    void initialize();

    QString m_emptyString;
    ConfigKey m_coKey;
    std::unique_ptr<ControlProxy> m_pControlProxy;
};
