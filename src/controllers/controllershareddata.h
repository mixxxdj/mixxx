#pragma once

#include <QHash>
#include <QObject>
#include <QVariant>

class ControllerNamespacedSharedData;

/// ControllerSharedData is the central triple-keyed data store that allows
/// controller script engines to share arbitrary data. Values are organized
/// as namespace → entity → key → value. Controllers interact through
/// ControllerNamespacedSharedData wrappers that confine them to their own
/// namespace.
///
/// See the Controller Shared Data proposal for details.
class ControllerSharedData : public QObject {
    Q_OBJECT
  public:
    explicit ControllerSharedData(QObject* parent = nullptr);

    /// Get a value for a specific namespace/entity/key triple.
    /// Returns a null QVariant if not found.
    QVariant get(const QString& ns,
            const QString& entity,
            const QString& key) const;

    /// Create a namespace-scoped wrapper for use by a controller.
    /// The caller owns the returned wrapper.
    /// @param ns The namespace to restrict access to
    /// @return The pointer to the newly allocated wrapper
    ControllerNamespacedSharedData* namespaced(const QString& ns);

  public slots:
    /// Set a value for a specific namespace/entity/key triple.
    /// Emits the updated() signal with the originating sender pointer so
    /// receivers can suppress self-notifications.
    void set(const QString& ns,
            const QString& entity,
            const QString& key,
            const QVariant& value,
            QObject* sender = nullptr);

  signals:
    /// Emitted whenever a value changes. The sender pointer identifies who
    /// initiated the change so listeners can avoid circular updates.
    void updated(const QString& ns,
            const QString& entity,
            const QString& key,
            const QVariant& value,
            QObject* sender);

  private:
    // namespace -> entity -> key -> value
    QHash<QString, QHash<QString, QHash<QString, QVariant>>> m_values;
};

/// ControllerNamespacedSharedData restricts access to a single namespace.
/// It does not hold data itself — all storage is in ControllerSharedData.
class ControllerNamespacedSharedData : public QObject {
    Q_OBJECT
  public:
    ControllerNamespacedSharedData(
            ControllerSharedData* parent, const QString& ns);

    QVariant get(const QString& entity, const QString& key) const;

    const QString& ns() const {
        return m_namespace;
    }

  public slots:
    /// Set a value, forwarding to the parent store.
    /// @param sender The object that initiated the set (used to suppress
    ///        self-notifications).
    void set(const QString& entity,
            const QString& key,
            const QVariant& value,
            QObject* sender = nullptr);

  signals:
    /// Emitted when any value in this namespace changes, excluding changes
    /// originating from the given sender.
    void updated(const QString& entity,
            const QString& key,
            const QVariant& value,
            QObject* sender);

  private:
    QString m_namespace;
};
