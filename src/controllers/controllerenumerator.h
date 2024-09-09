#pragma once

#include <QList>
#include <QObject>

class Controller;

/// Base class handling discovery and enumeration of DJ controllers.
///
/// This class handles discovery and enumeration of DJ controllers and
/// must be inherited by a class that implements it on some API.
class ControllerEnumerator : public QObject {
    Q_OBJECT
  public:
    ControllerEnumerator() = default;
    // In this function, the inheriting class must delete the Controllers it
    // creates
    ~ControllerEnumerator() override = default;

    // Controller pointers are borrowed and instead tied to the lifetime of the enumerator
    // calling queryDevices will invalidate all pointers received previously!!
    virtual QList<Controller*> queryDevices() = 0;

    // Sub-classes return true here if their devices must be polled to get data
    // from the controller.
    virtual bool needPolling() {
        return false;
    }
};
