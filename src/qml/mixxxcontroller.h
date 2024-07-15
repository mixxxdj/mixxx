#ifndef MIXXX_MIXXXCONTROLLER_H
#define MIXXX_MIXXXCONTROLLER_H

#include <QtQml/qqmlregistration.h>

#include <QObject>
#include <QtQml>

#include "mixxxscreen.h"

namespace mixxx {
namespace qml {

class MixxxController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString controllerId MEMBER m_controllerId)
    Q_PROPERTY(bool debugMode MEMBER m_debugMode)
    Q_PROPERTY(QQmlListProperty<MixxxScreen> screens MEMBER m_screens)
    Q_CLASSINFO("DefaultProperty", "screens")

  public:
    void init();
    void shutdown();

  private:
    QString m_controllerId;
    bool m_debugMode;
    QQmlListProperty<MixxxScreen> m_screens;
};

} // namespace qml
} // namespace mixxx

#endif // MIXXX_MIXXXCONTROLLER_H
