#pragma once

#include <QHash>
#include <QList>

#include "effects/backends/effectsbackendmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "preferences/usersettings.h"

class EffectsManager;

struct EffectsXmlData {
    QHash<QString, EffectChainPresetPointer> quickEffectChainPresets;
    QList<EffectChainPresetPointer> standardEffectChainPresets;
};

/// EffectChainPresetManager maintains a list of custom EffectChainPresets in the
/// "effects/chains" folder in the user settings folder. The state of loaded
/// effects are saved as EffectChainPresets in the effects.xml file in the user
/// settings folder, which is used to restore the state of effects on startup.
class EffectChainPresetManager : public QObject {
    Q_OBJECT

  public:
    EffectChainPresetManager(
            UserSettingsPointer pConfig,
            EffectsBackendManagerPointer pBackendManager);
    ~EffectChainPresetManager() = default;

    const QList<EffectChainPresetPointer> getPresetsSorted() const {
        return m_effectChainPresetsSorted;
    }

    const QList<EffectChainPresetPointer> getQuickEffectPresetsSorted() const {
        return m_quickEffectChainPresetsSorted;
    }

    int numPresets() const {
        return m_effectChainPresetsSorted.size();
    }

    int numQuickEffectPresets() const {
        return m_quickEffectChainPresetsSorted.size();
    }

    int presetIndex(const QString& presetName) const;
    int presetIndex(EffectChainPresetPointer pChainPreset) const;
    EffectChainPresetPointer presetAtIndex(int index) const;

    int quickEffectPresetIndex(const QString& presetName) const;
    int quickEffectPresetIndex(EffectChainPresetPointer pChainPreset) const;
    EffectChainPresetPointer quickEffectPresetAtIndex(int index) const;

    bool importPreset();
    void exportPreset(const QString& chainPresetName);
    bool renamePreset(const QString& oldName);
    bool deletePreset(const QString& chainPresetName);

    void resetToDefaults();

    void setPresetOrder(const QStringList& chainPresetList);
    void setQuickEffectPresetOrder(const QStringList& chainPresetList);

    EffectChainPresetPointer getPreset(const QString& name) const {
        return m_effectChainPresets.value(name);
    }

    void savePresetAndReload(EffectChainPointer pChainSlot);
    bool savePreset(EffectChainPresetPointer pPreset);
    void updatePreset(EffectChainPointer pChainSlot);

    EffectsXmlData readEffectsXml(const QDomDocument& doc, const QStringList& deckStrings);
    void saveEffectsXml(QDomDocument* pDoc, const EffectsXmlData& data);

  signals:
    void effectChainPresetRenamed(const QString& oldName, const QString& newName);
    void effectChainPresetListUpdated();
    void quickEffectChainPresetListUpdated();

  private:
    bool savePresetXml(EffectChainPresetPointer pPreset);

    void importUserPresets();
    void importDefaultPresets();
    void generateDefaultQuickEffectPresets();
    void prependRemainingPresetsToLists();

    QHash<QString, EffectChainPresetPointer> m_effectChainPresets;
    // The sort orders are chosen by the user in DlgPrefEffects.
    QList<EffectChainPresetPointer> m_effectChainPresetsSorted;
    QList<EffectChainPresetPointer> m_quickEffectChainPresetsSorted;

    UserSettingsPointer m_pConfig;
    EffectsBackendManagerPointer m_pBackendManager;
};

typedef QSharedPointer<EffectChainPresetManager> EffectChainPresetManagerPointer;
