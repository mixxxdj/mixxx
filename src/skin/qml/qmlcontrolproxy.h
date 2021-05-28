#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <QtQml>
#include <memory>

#include "control/controlproxy.h"

namespace mixxx {
namespace skin {
namespace qml {

class QmlControlProxy : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
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
    explicit QmlControlProxy(QObject* parent = nullptr);

    /// Implementing the QQmlParserStatus interface requires overriding this
    /// method, but we don't need it.
    // Invoked after class creation, but before any properties have been set.
    void classBegin() override{};

    /// QML cannot pass arguments to C++ constructors so this class needs to
    /// rely on the QML object setting the group and key properties to
    /// initialize the ControlProxy. We want to deplay the initialization of
    /// the underlying ControlProxy until the object has been fully created and
    /// all properties (group and key in particular) have been set.  Perform
    /// some initialization here now that the object is fully created.
    void componentComplete() override;

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
    /// Emits both the valueChanged and parameterChanged signals
    void slotControlProxyValueChanged(double newValue);

  private:
    /// (Re-)Initializes or resets the pointer to the underlying control proxy.
    /// Called for the first time when component construction has been
    /// completed. From that moment on, it's called whenever the group or key
    /// changes.
    void reinitializeFromKey();

    ConfigKey m_coKey;

    /// Set to true in the componentComplete() method, which is called when the
    /// QML object creation is complete.
    bool m_isComponentComplete;
    std::unique_ptr<ControlProxy> m_pControlProxy;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
