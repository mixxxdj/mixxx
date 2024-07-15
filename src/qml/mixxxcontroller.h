#ifndef MIXXX_MIXXXCONTROLLER_H
#define MIXXX_MIXXXCONTROLLER_H

#include <QtQml/qqmlregistration.h>

#include <QObject>

namespace mixxx {
namespace qml {

class MixxxController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString controllerId MEMBER m_controllerId)
    Q_PROPERTY(bool debugMode MEMBER m_debugMode)

  public:
    void init();
    void shutdown();

  private:
    QString m_controllerId;
    bool m_debugMode;
};

} // namespace qml
} // namespace mixxx

#endif // MIXXX_MIXXXCONTROLLER_H
