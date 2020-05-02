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

    int presetIndex(const QString& presetName) const;
    int presetIndex(EffectChainPresetPointer pChainPreset) const;
    EffectChainPresetPointer presetAtIndex(int index) const;

    void importPreset();
    void exportPreset(const QString& chainPresetName);
    void renamePreset(const QString& oldName);
    void deletePreset(const QString& chainPresetName);

    void setPresetOrder(const QStringList& chainPresetList);

    EffectChainPresetPointer getPreset(const QString& name) const {
        return m_effectChainPresets.value(name);
    }

    void savePreset(EffectChainPresetPointer pPreset);
    void savePreset(EffectChainSlotPointer pChainSlot);

    EffectsXmlData readEffectsXml(QStringList deckStrings);
    void saveEffectsXml(EffectsXmlData data);

  signals:
    void effectChainPresetListUpdated();

  private:
    void loadEffectChainPresets();

    QHash<QString, EffectChainPresetPointer> m_effectChainPresets;
    QList<EffectChainPresetPointer> m_effectChainPresetsSorted;

    UserSettingsPointer m_pConfig;
    EffectsBackendManagerPointer m_pBackendManager;

    EffectChainPresetPointer m_pDefaultQuickEffectChainPreset;
};

typedef QSharedPointer<EffectChainPresetManager> EffectChainPresetManagerPointer;
