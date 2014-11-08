/***************************************************************************
                          dlgprefeq.h  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFEQ_H
#define DLGPREFEQ_H

#include <QWidget>
#include <QComboBox>

#include "ui_dlgprefeqdlg.h"
#include "configobject.h"
#include "controlobjectslave.h"
#include "preferences/dlgpreferencepage.h"
#include "effects/effectsmanager.h"

/**
  *@author John Sully
  */

class DlgPrefEQ : public DlgPreferencePage, public Ui::DlgPrefEQDlg  {
    Q_OBJECT
  public:
    DlgPrefEQ(QWidget *parent, EffectsManager* pEffectsManager,
              ConfigObject<ConfigValue>* _config);
    virtual ~DlgPrefEQ();

  public slots:
    void slotEqEffectChangedOnDeck(int effectIndex);
    void slotFilterEffectChangedOnDeck(int effectIndex);
    void slotAddComboBox(double numDecks);
    // Slot for toggling between advanced and basic views
    void slotPopulateDeckEffectSelectors();
    // Update Hi EQ
    void slotUpdateHiEQ();
    // Update Lo EQ
    void slotUpdateLoEQ();
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();
    void slotBypass(int state);
    // Update the Master EQ
    void slotUpdateFilter(int value);

  signals:
    void apply(const QString &);
    void effectOnChainSlot(const unsigned int, const unsigned int, QString);

  private:
    void loadSettings();
    void setDefaultShelves();
    double getEqFreq(int value, int minimum, int maximum);
    int getSliderPosition(double eqFreq, int minimum, int maximum);
    void validate_levels();
    void updateBandFilter(int index, double value);
    void setUpMasterEQ();

    ControlObjectSlave m_COLoFreq;
    ControlObjectSlave m_COHiFreq;
    ConfigObject<ConfigValue>* m_pConfig;
    double m_lowEqFreq, m_highEqFreq;

    // Members needed for changing the effects loaded on the EQ Effect Rack
    EffectsManager* m_pEffectsManager;
    EffectRackPointer m_pDeckEQEffectRack;
    EffectRackPointer m_pMasterEQEffectRack;
    QList<QComboBox*> m_deckEqEffectSelectors;
    QList<QComboBox*> m_deckFilterEffectSelectors;
    QList<ControlObject*> m_enableWaveformEqCOs;
    ControlObjectSlave* m_pNumDecks;
    QString m_eqRackGroup;

    bool m_inSlotPopulateDeckEffectSelectors;

    // Members needed for the Master EQ
    QList<QSlider*> m_masterEQSliders;
    EngineEffect* m_pEngineEffectMasterEQ;
};

#endif
