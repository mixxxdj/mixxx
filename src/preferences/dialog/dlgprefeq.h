#pragma once

#include <QComboBox>
#include <QWidget>

#include "control/controlproxy.h"
#include "effects/effectrack.h"
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
    QString getQuickEffectGroupForDeck(int deck) const;

  public slots:
    void slotEqEffectChangedOnDeck(int effectIndex);
    void slotQuickEffectChangedOnDeck(int effectIndex);
    void slotNumDecksChanged(double numDecks);
    void slotSingleEqChecked(int checked);
    // Slot for toggling between advanced and basic views
    void slotPopulateDeckEffectSelectors();
    // Update Hi EQ
    void slotUpdateHiEQ();
    // Update Lo EQ
    void slotUpdateLoEQ();
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;
    void slotUpdateEqAutoReset(int);
    void slotUpdateGainAutoReset(int);
    void slotBypass(int state);
    // Update the Main EQ
    void slotUpdateMainEQParameter(int value);
    void slotMainEQToDefault();
    void setMainEQParameter(int i, double value);
    void slotMainEqEffectChanged(int effectIndex);

  signals:
    void apply(const QString &);
    void effectOnChainSlot(const unsigned int, const unsigned int, const QString&);

  private:
    void loadSettings();
    void setDefaultShelves();
    double getEqFreq(int value, int minimum, int maximum);
    int getSliderPosition(double eqFreq, int minimum, int maximum);
    void validate_levels();
    void updateBandFilter(int index, double value);
    void setUpMainEQ();
    void applySelections();

    ControlProxy m_COLoFreq;
    ControlProxy m_COHiFreq;
    UserSettingsPointer m_pConfig;
    double m_lowEqFreq, m_highEqFreq;

    // Members needed for changing the effects loaded on the EQ Effect Rack
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    EqualizerRackPointer m_pEQEffectRack;
    QuickEffectRackPointer m_pQuickEffectRack;
    OutputEffectRackPointer m_pOutputEffectRack;
    QLabel* m_firstSelectorLabel;
    QList<QComboBox*> m_deckEqEffectSelectors;
    QList<QComboBox*> m_deckQuickEffectSelectors;
    QList<bool> m_filterWaveformEffectLoaded;
    QList<ControlObject*> m_filterWaveformEnableCOs;
    ControlProxy* m_pNumDecks;

    bool m_inSlotPopulateDeckEffectSelectors;

    // Members needed for the Main EQ
    QList<QSlider*> m_masterEQSliders;
    QList<QLabel*> m_masterEQValues;
    QList<QLabel*> m_masterEQLabels;
    QWeakPointer<Effect> m_pEffectMainEQ;

    bool m_bEqAutoReset;
    bool m_bGainAutoReset;
};
