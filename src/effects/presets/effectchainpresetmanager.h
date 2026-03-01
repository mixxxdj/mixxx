#pragma once

#include <QHash>
#include <QList>

#include "effects/backends/effectsbackendmanager.h"
#include "preferences/usersettings.h"

struct EffectsXmlData {
    QHash<QString, EffectManifestPointer> eqEffectManifests;
    QHash<QString, EffectChainPresetPointer> quickEffectChainPresets;
    QHash<QString, EffectChainPresetPointer> quickStemEffectChainPresets;
    QList<EffectChainPresetPointer> standardEffectChainPresets;
    EffectChainPresetPointer outputChainPreset;
};

struct EffectXmlDataSingleDeck {
    EffectManifestPointer eqEffectManifest;
    EffectChainPresetPointer quickEffectChainPreset;
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

    EffectManifestPointer getDefaultEqEffect();
    EffectChainPresetPointer getDefaultQuickEffectPreset();

    static EffectChainPresetPointer createEmptyNamelessChainPreset();

    EffectsXmlData readEffectsXml(const QDomDocument& doc, const QStringList& deckStrings);
    EffectXmlDataSingleDeck readEffectsXmlSingleDeck(
            const QDomDocument& doc, const QString& deckString);
    EffectChainPresetPointer readEffectsXmlSingleDeckStem(
            const QDomDocument& doc, const QString& deckStemString);
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
