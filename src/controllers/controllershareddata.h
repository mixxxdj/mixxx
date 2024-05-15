#pragma once

#include <QVariant>

#include "util/assert.h"

class ControllerNamespacedSharedData;

/// ControllerSharedData is a wrapper that allows controllers script runtimes
/// to share arbitrary data via a the JavaScript interface. Controllers don't
/// access this object directly, and instead uses the
/// ControllerNamespacedSharedData wrapper to isolate a specific namespace and
/// prevent potential clash
class ControllerSharedData : public QObject {
    Q_OBJECT
  public:
    ControllerSharedData(QObject* parent)
            : QObject(parent), m_value() {
    }

    QVariant get(const QString& ns) const {
        return m_value.value(ns);
    }

    /// @brief Create a a namespace wrapper that can be used by a controller.
    /// The caller is owning the wrapper
    /// @param ns The namespace to restrict access to
    /// @return The pointer to the newly allocated wrapper
    ControllerNamespacedSharedData* namespaced(const QString& ns);

  public slots:
    void set(const QString& ns, const QVariant& value) {
        m_value[ns] = value;
        emit updated(ns, m_value[ns]);
    }

  signals:
    void updated(const QString& ns, const QVariant& value);

  private:
    QHash<QString, QVariant> m_value;
};

/// ControllerNamespacedSharedData is a wrapper that restrict access to a given
/// namespace. It doesn't hold any data and can safely be deleted at all time,
/// but only provide the namespace abstraction for controller to interact with
/// via a the JavaScript interface
class ControllerNamespacedSharedData : public QObject {
    Q_OBJECT
  public:
    ControllerNamespacedSharedData(ControllerSharedData* parent, const QString& ns);

    QVariant get() const {
        return static_cast<ControllerSharedData*>(parent())->get(m_namespace);
    }

  public slots:
    void set(const QVariant& value) {
        static_cast<ControllerSharedData*>(parent())->set(m_namespace, value);
    }

  signals:
    void updated(const QVariant& value);

  private:
    QString m_namespace;
};
