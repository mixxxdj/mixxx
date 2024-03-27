#pragma once

#include <QVariant>

#include "util/assert.h"

/// ControllerRuntimeData is a wrapper that allows controllers script runtimes
/// to share arbitrary data via a the JavaScript interface. It doesn't enforce
/// any type consistency and it is the script responsibility to use this data in
/// a considerate, non-destructive way (append to lists, extend to objects,
/// ...), as well as expecting that others won't do so.
class ControllerRuntimeData : public QObject {
    Q_OBJECT
  public:
    ControllerRuntimeData(QObject* parent)
            : QObject(parent), m_value() {
    }

    const QVariant& get() const {
        return m_value;
    }

  public slots:
    void set(const QVariant& value) {
        m_value = value;
        emit updated(m_value);
    }

  signals:
    void updated(const QVariant& value);

  private:
    QVariant m_value;
};
