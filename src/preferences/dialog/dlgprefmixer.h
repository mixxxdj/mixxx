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
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    void slotNumDecksChanged(double numDecks);
    void slotEQEffectChangedOnDeck(int effectIndex);
    void slotQuickEffectChangedOnDeck(int effectIndex);
    void slotSingleEqToggled(bool checked);
    void slotEqAutoResetToggled(bool checked);
    void slotGainAutoResetToggled(bool checked);
    void slotBypassEqToggled(bool checked);
    // Create, populate and show/hide EQ & QuickEffect selectors, considering the
    // number of decks and the 'Single EQ' checkbox
    void slotPopulateDeckEffectSelectors();

    void slotUpdateXFader();
    void slotUpdateHiEQ();
    void slotUpdateLoEQ();

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
    void validateEQShelves();
    void setUpMainEQ();

    void applySelectionsToDecks();

    typedef bool (*EffectManifestFilterFnc)(EffectManifest* pManifest);
    const QList<EffectManifestPointer> getFilteredManifests(
            EffectManifestFilterFnc filterFunc) const;
    void populateDeckEqBoxList(
            const QList<QComboBox*>& boxList,
            EffectManifestFilterFnc filterFunc);
    void populateDeckQuickEffectBoxList(
            const QList<QComboBox*>& boxList);


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

    bool m_singleEq;
    bool m_eqEffectsOnly;
    bool m_eqAutoReset;
    bool m_gainAutoReset;
    bool m_eqBypass;

    bool m_initializing;

    QList<int> m_eqIndiciesOnUpdate;
};
