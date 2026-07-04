#pragma once

#include <memory>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "effects/defs.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmixerdlg.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class QComboBox;
class QWidget;
class EffectsManager;

class DlgPrefMixer : public DlgPreferencePage, public Ui::DlgPrefMixerDlg {
    Q_OBJECT
  public:
    DlgPrefMixer(
            QWidget* parent,
            std::shared_ptr<EffectsManager> pEffectsManager,
            UserSettingsPointer _config);

    QUrl helpUrl() const override;

  public slots:
    void slotApply() override;
    /// Update the widgets with values from config / EffectsManager
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    /// Create EQ & QuickEffect selectors and deck label for each added deck
    void slotNumDecksChanged(double numDecks);
    void slotEQEffectSelectionChanged(int effectIndex);
    void slotQuickEffectSelectionChanged(int effectIndex);
    void slotEqOnlyToggled(bool checked);
    void slotSingleEqToggled(bool checked);
    void slotEqAutoResetToggled(bool checked);
    void slotGainAutoResetToggled(bool checked);
#ifdef __STEM__
    void slotStemAutoResetToggled(bool checked);
#endif
    void slotBypassEqToggled(bool checked);
    /// Create, populate and show/hide EQ & QuickEffect selectors, considering the
    /// number of decks and the 'Single EQ' checkbox
    void slotPopulateDeckEqSelectors();
    void slotPopulateQuickEffectSelectors();

    void slotUpdateXFader();
    void updateXFaderWidgets();

    void slotXFaderReverseBoxToggled();
    void slotXFaderModeBoxToggled();
    void slotXFaderSliderChanged();

    void slotXFaderCurveControlChanged(double v);
    void slotXFaderCalibrationControlChanged(double v);
    void slotXFaderModeControlChanged(double v);
    void slotXFaderReverseControlChanged(double v);

    void slotHiEqSliderChanged();
    void slotLoEqSliderChanged();

    /// Update the Main EQ
    void slotMainEQParameterSliderChanged(int value);
    void slotMainEQToDefault();
    void slotMainEqEffectChanged(int effectIndex);

  private:
    void drawXfaderDisplay();

    void setDefaultShelves();
    double getEqFreq(int value, int minimum, int maximum);
    int getSliderPosition(double eqFreq, int minimum, int maximum);
    void validateEQShelves();

    void applyXFader();

    void applyDeckEQs();
    void applyQuickEffects();

    void storeEqShelves();

    void setUpMainEQ();
    void updateMainEQ();

    typedef bool (*EffectManifestFilterFnc)(EffectManifest* pManifest);
    const QList<EffectManifestPointer> getDeckEqManifests() const;
    const QList<EffectManifestPointer> getMainEqManifests() const;

    UserSettingsPointer m_pConfig;

    // X-fader values
    int m_xFaderMode;
    double m_xFaderCurve, m_xFaderCal;

    parented_ptr<ControlProxy> m_xfModeCO;
    parented_ptr<ControlProxy> m_xfCurveCO;
    parented_ptr<ControlProxy> m_xfReverseCO;
    parented_ptr<ControlProxy> m_xfCalibrationCO;
    PollingControlProxy m_crossfader;

    bool m_xFaderReverse;
    parented_ptr<QGraphicsScene> m_pxfScene;

    PollingControlProxy m_COLoFreq;
    PollingControlProxy m_COHiFreq;
    double m_lowEqFreq, m_highEqFreq;

    EffectChainPresetManagerPointer m_pChainPresetManager;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    EffectsBackendManagerPointer m_pBackendManager;
    QList<QComboBox*> m_deckEqEffectSelectors;
    QList<QComboBox*> m_deckQuickEffectSelectors;
    parented_ptr<ControlProxy> m_pNumDecks;

    bool m_ignoreEqQuickEffectBoxSignals;

    // Members needed for the Main EQ
    QList<QSlider*> m_mainEQSliders;
    QList<QLabel*> m_mainEQValues;
    QList<QLabel*> m_mainEQLabels;
    QWeakPointer<EffectSlot> m_pEffectMainEQ;

    bool m_singleEq;
    bool m_eqEffectsOnly;
    bool m_eqAutoReset;
    bool m_gainAutoReset;
#ifdef __STEM__
    bool m_stemAutoReset;
#endif
    bool m_eqBypass;

    bool m_initializing;
    bool m_updatingMainEQ;
    bool m_applyingDeckEQs;
    bool m_applyingQuickEffects;

    QList<int> m_eqIndiciesOnUpdate;
    QList<int> m_quickEffectIndiciesOnUpdate;
};
