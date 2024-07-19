#ifndef MIXXX_MIXXXCONTROLLER_H
#define MIXXX_MIXXXCONTROLLER_H

#include <QtQml/qqmlregistration.h>

#include <QObject>
#include <QtQml>

namespace mixxx {
namespace qml {

class MixxxController : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString controllerId MEMBER m_controllerId)
    Q_PROPERTY(bool debugMode MEMBER m_debugMode)
    Q_PROPERTY(QQmlListProperty<QObject> childComponents MEMBER m_pChildren)

    Q_CLASSINFO("DefaultProperty", "childComponents")

  public:
    explicit MixxxController(QObject* parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

  signals:
    void init();
    void shutdown();

  private:
    QString m_controllerId;
    bool m_debugMode;
    QList<QObject*> m_children;
    QQmlListProperty<QObject> m_pChildren;

  private slots:
    void initChildrenComponents();
    void shutdownChildrenComponents();
};

} // namespace qml
} // namespace mixxx

#endif // MIXXX_MIXXXCONTROLLER_H
