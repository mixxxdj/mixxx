#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

#include "control/controlobject.h"

/// Information about a control action
struct ControlActionInfo {
    ConfigKey key;
    QString category;
    QString displayName;
    QString description;

    ControlActionInfo() {
    }

    ControlActionInfo(const ConfigKey& k, const QString& cat, const QString& name, const QString& desc)
        : key(k),
          category(cat),
          displayName(name),
          description(desc) {
    }
};

/// Registry of all available control actions for keyboard mapping
class KeyboardActionRegistry : public QObject {
    Q_OBJECT

  public:
    explicit KeyboardActionRegistry(QObject* parent = nullptr);
    ~KeyboardActionRegistry() override;

    /// Get all available actions
    QList<ControlActionInfo> getAllActions() const;

    /// Get actions filtered by category
    QList<ControlActionInfo> getActionsByCategory(const QString& category) const;

    /// Get all available categories
    QStringList getCategories() const;

    /// Search actions by name or description
    QList<ControlActionInfo> searchActions(const QString& query) const;

    /// Get action info for a specific ConfigKey
    ControlActionInfo getActionInfo(const ConfigKey& key) const;

    /// Check if a control exists
    bool hasAction(const ConfigKey& key) const;

  private:
    /// Build the action registry
    void buildRegistry();

    /// Register a control action
    void registerAction(const ConfigKey& key,
                       const QString& category,
                       const QString& displayName,
                       const QString& description);

    /// Register common deck controls
    void registerDeckControls(const QString& group);

    /// Register library controls
    void registerLibraryControls();

    /// Register master controls
    void registerMasterControls();

    /// Register effect controls
    void registerEffectControls();

    /// Register sampler controls
    void registerSamplerControls();

    QMap<ConfigKey, ControlActionInfo> m_actions;
    QMap<QString, QList<ConfigKey>> m_categorizedActions;
};
