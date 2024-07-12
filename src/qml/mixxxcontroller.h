#ifndef MIXXX_MIXXXCONTROLLER_H
#define MIXXX_MIXXXCONTROLLER_H

#include <QtQml/qqmlregistration.h>

#include <QObject>

namespace mixxx {
namespace qml {

class MixxxController : public QObject {
    struct ControllerInfo {
        Q_GADGET
      public:
        QString name;
        QString author;
        QString forums;
        QString manual;
        // Other?
    };

    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ControllerInfo info READ info WRITE setInfo)
};

} // namespace qml
} // namespace mixxx

#endif // MIXXX_MIXXXCONTROLLER_H
