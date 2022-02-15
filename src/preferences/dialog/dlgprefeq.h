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
    DlgPrefEQ(QWidget *parent, EffectsManager* pEffectsManager,
              UserSettingsPointer _config);
    virtual ~DlgPrefEQ();

    QUrl helpUrl() const override;

    QString getEQEffectGroupForDeck(int deck) const;
    QString getQuickEffectGroupForDeck(int deck) const;

  public slots:
    void slotNumDecksChanged(double numDecks);
    void slotSingleEqChecked(int checked);
    void slotPopulateDeckEffectSelectors();
    void slotUpdateHiEQ();
    void slotUpdateLoEQ();
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;
    void slotBypass(int state);
    void slotUpdateMasterEQParameter(int value);
    void slotMasterEQToDefault();
    void setMasterEQParameter(int i, double value);
    void slotMasterEqEffectChanged(int effectIndex);

  signals:
    void apply(const QString &);
    void effectOnChainSlot(const unsigned int, const unsigned int, const QString&);

  private:
    void loadSettings();
    void loadEffectSelection(int oldDeckCount);
    void setDefaultShelves();
    double getEqFreq(int value, int minimum, int maximum);
    int getSliderPosition(double eqFreq, int minimum, int maximum);
    void validate_levels();
    void updateBandFilter(int index, double value);
    void setUpMasterEQ();
    void applySelections();

    ControlProxy m_COLoFreq;
    ControlProxy m_COHiFreq;
    UserSettingsPointer m_pConfig;
    double m_lowEqFreq, m_highEqFreq;

    // Members needed for changing the effects loaded on the EQ Effect Rack
    EffectsManager* m_pEffectsManager;
    EqualizerRackPointer m_pEQEffectRack;
    QuickEffectRackPointer m_pQuickEffectRack;
    OutputEffectRackPointer m_pOutputEffectRack;
    QLabel* m_firstSelectorLabel;
    QList<QComboBox*> m_deckEqEffectSelectors;
    QList<QComboBox*> m_deckQuickEffectSelectors;
    QList<bool> m_filterWaveformEffectLoaded;
    QList<ControlObject*> m_filterWaveformEnableCOs;
    ControlProxy* m_pNumDecks;
    int m_numDecks;

    // Members needed for the Master EQ
    QList<QSlider*> m_masterEQSliders;
    QList<QLabel*> m_masterEQValues;
    QList<QLabel*> m_masterEQLabels;
    QWeakPointer<Effect> m_pEffectMasterEQ;
};
