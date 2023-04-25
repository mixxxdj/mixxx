#pragma once

#include <QComboBox>
#include <QWidget>
#include <functional>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "effects/effectsmanager.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmixerdlg.h"
#include "preferences/usersettings.h"

class DlgPrefMixer : public DlgPreferencePage, public Ui::DlgPrefMixerDlg {
    Q_OBJECT
  public:
    DlgPrefMixer(
            QWidget* parent,
            std::shared_ptr<EffectsManager> pEffectsManager,
            UserSettingsPointer _config);
    virtual ~DlgPrefMixer();

    QUrl helpUrl() const override;

  public slots:
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;
    void slotUpdateXFader();

  private slots:
    void slotEffectChangedOnDeck(int effectIndex);
    void slotQuickEffectChangedOnDeck(int effectIndex);
    void slotNumDecksChanged(double numDecks);
    void slotSingleEqCheckboxChanged(int checked);
    // Slot for toggling between advanced and basic views
    void slotPopulateDeckEffectSelectors();
    // Update Hi EQ
    void slotUpdateHiEQ();
    // Update Lo EQ
    void slotUpdateLoEQ();

    void slotUpdateEqAutoReset(int);
    void slotUpdateGainAutoReset(int);
    void slotBypassEqChanged(int state);
    // Update the Main EQ
    void slotApplyMainEQParameter(int value);
    void slotMainEQToDefault();
    void setMainEQParameter(int i, double value);
    void slotMainEqEffectChanged(int effectIndex);

  private:
    void drawXfaderDisplay();
    void setDefaultShelves();
    double getEqFreq(int value, int minimum, int maximum);
    int getSliderPosition(double eqFreq, int minimum, int maximum);
    void validate_levels();
    void updateBandFilter(int index, double value);
    void setUpMainEQ();
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

    UserSettingsPointer m_pConfig;

    // X-fader values
    int m_xFaderMode;
    double m_transform, m_cal;

    PollingControlProxy m_mode;
    PollingControlProxy m_curve;
    PollingControlProxy m_calibration;
    PollingControlProxy m_reverse;
    PollingControlProxy m_crossfader;

    bool m_xFaderReverse;

    ControlProxy m_COLoFreq;
    ControlProxy m_COHiFreq;
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
    bool m_bEqBypass;

    QList<int> m_eqIndiciesOnUpdate;
};
