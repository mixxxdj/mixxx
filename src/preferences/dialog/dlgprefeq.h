#pragma once

#include <QComboBox>
#include <QWidget>
#include <functional>

#include "control/controlproxy.h"
#include "effects/effectsmanager.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefeqdlg.h"
#include "preferences/usersettings.h"

class DlgPrefEQ : public DlgPreferencePage, public Ui::DlgPrefEQDlg  {
    Q_OBJECT
  public:
    DlgPrefEQ(
            QWidget* parent,
            std::shared_ptr<EffectsManager> pEffectsManager,
            UserSettingsPointer _config);
    virtual ~DlgPrefEQ();

    QUrl helpUrl() const override;

    QString getEQEffectGroupForDeck(int deck) const;

  public slots:
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    void slotEffectChangedOnDeck(int effectIndex);
    void slotQuickEffectChangedOnDeck(int effectIndex);
    void slotNumDecksChanged(double numDecks);
    void slotSingleEqChecked(int checked);
    // Slot for toggling between advanced and basic views
    void slotPopulateDeckEffectSelectors();
    // Update Hi EQ
    void slotUpdateHiEQ();
    // Update Lo EQ
    void slotUpdateLoEQ();

    void slotUpdateEqAutoReset(int);
    void slotUpdateGainAutoReset(int);
    void slotBypass(int state);
    // Update the Main EQ
    void slotApplyMainEQParameter(int value);
    void slotMainEQToDefault();
    void setMainEQParameter(int i, double value);
    void slotMainEqEffectChanged(int effectIndex);

  private:
    void loadSettings();
    void setDefaultShelves();
    double getEqFreq(int value, int minimum, int maximum);
    int getSliderPosition(double eqFreq, int minimum, int maximum);
    void validate_levels();
    void updateBandFilter(int index, double value);
    void setUpMainEQ();
    void setupMainEqWidgets();
    void applySelections();

    typedef bool (*EffectManifestFilterFnc)(EffectManifest* pManifest);
    const QList<EffectManifestPointer> getFilteredManifests(
            EffectManifestFilterFnc filterFunc) const;
    void populateDeckEqBoxList(
            const QList<QComboBox*>& boxList,
            EffectManifestFilterFnc filterFunc);
    void populateDeckQuickEffectBoxList(
            const QList<QComboBox*>& boxList);

    void applySelectionsToDecks();

    bool eventFilter(QObject* obj, QEvent* e) override;

    ControlProxy m_COLoFreq;
    ControlProxy m_COHiFreq;
    UserSettingsPointer m_pConfig;
    double m_lowEqFreq, m_highEqFreq;

    EffectChainPresetManagerPointer m_pChainPresetManager;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    EffectsBackendManagerPointer m_pBackendManager;
    QLabel* m_firstSelectorLabel;
    QList<QComboBox*> m_deckEqEffectSelectors;
    QList<QComboBox*> m_deckQuickEffectSelectors;
    ControlProxy* m_pNumDecks;

    bool m_inSlotPopulateDeckEffectSelectors;

    // Members needed for the Main EQ
    QList<QSlider*> m_mainEQSliders;
    QList<QLabel*> m_mainEQValues;
    QList<QLabel*> m_mainEQLabels;
    QWeakPointer<EffectSlot> m_pEffectMainEQ;

    bool m_bEqAutoReset;
    bool m_bGainAutoReset;

    QList<int> m_eqIndiciesOnUpdate;
};
