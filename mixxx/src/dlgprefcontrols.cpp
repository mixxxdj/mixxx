/***************************************************************************
                          dlgprefcontrols.cpp  -  description
                             -------------------
    begin                : Sat Jul 5 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgprefcontrols.h"
#include "qcombobox.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "mixxxview.h"
#include <qdir.h>

DlgPrefControls::DlgPrefControls(QWidget *parent, ControlObject *pControl, MixxxView *pView, ConfigObject<ConfigValue> *pConfig) : DlgPrefControlsDlg(parent,"")
{
    m_pConfig = pConfig;
    
    //
    // Rate slider configuration
    //

    m_pControlRate1 = (ControlPotmeter *)pControl->getControl(ConfigKey("[Channel1]","rate"));
    m_pControlRate2 = (ControlPotmeter *)pControl->getControl(ConfigKey("[Channel2]","rate"));
    m_pControlRateDir1 = pControl->getControl(ConfigKey("[Channel1]","rate_dir"));
    m_pControlRateDir2 = pControl->getControl(ConfigKey("[Channel2]","rate_dir"));    

    // Set default direction as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateDir")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateDir"),ConfigValue(0));

    float fDir = 1.;
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateDir")).toInt()==1.)
        fDir = -1.;
    m_pControlRateDir1->setValueFromApp(fDir);
    m_pControlRateDir2->setValueFromApp(fDir);

    connect(ComboBoxRateDir,   SIGNAL(activated(int)), this, SLOT(slotSetRateDir(int)));

    // Set default range as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateRange")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateRange"),ConfigValue(1));

    slotSetRateRange(m_pConfig->getValueString(ConfigKey("[Controls]","RateRange")).toInt());
    connect(ComboBoxRateRange, SIGNAL(activated(int)), this, SLOT(slotSetRateRange(int)));

    //
    // Visuals
    //
    
    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","Visuals")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(0));

    // Update combo box
    ComboBoxVisuals->insertItem("Waveform");
    ComboBoxVisuals->insertItem("Simple");
    ComboBoxVisuals->setCurrentItem(m_pConfig->getValueString(ConfigKey("[Controls]","Visuals")).toInt());
    
    connect(ComboBoxVisuals,   SIGNAL(activated(int)), this, SLOT(slotSetVisuals(int)));
        
    //
    // Skin configurations
    //
    ComboBoxSkinconf->clear();

    QString qSkinPath(pConfig->getValueString(ConfigKey("[Config]","Path")));
    QDir dir(qSkinPath.append("skins/"));
    dir.setFilter(QDir::Dirs);
    const QFileInfoList *list = dir.entryInfoList();
    if (list!=0)
    {
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing
        int j=0;
        while ((fi=it.current()))
        {
            if (fi->fileName()!="." && fi->fileName()!="..")
            {
                ComboBoxSkinconf->insertItem(fi->fileName());
                if (fi->fileName() == pConfig->getValueString(ConfigKey("[Config]","Skin")))
                    ComboBoxSkinconf->setCurrentItem(j);
                ++j;
            }
            ++it;
        }
    }

    connect(ComboBoxSkinconf, SIGNAL(activated(int)), this, SLOT(slotSetSkin(int)));
}

DlgPrefControls::~DlgPrefControls()
{
}

void DlgPrefControls::slotUpdate()
{
    ComboBoxRateRange->clear();
    ComboBoxRateRange->insertItem(" 8% (Technics SL1210)");
    ComboBoxRateRange->insertItem("10% (Omnitronic 3xxx)");
    ComboBoxRateRange->insertItem("20%");
    ComboBoxRateRange->insertItem("30%");
    ComboBoxRateRange->insertItem("40%");
    ComboBoxRateRange->insertItem("50%");
    ComboBoxRateRange->insertItem("60%");
    ComboBoxRateRange->insertItem("70%");
    ComboBoxRateRange->insertItem("80%");
    ComboBoxRateRange->insertItem("90%");

    float idx = 10.*m_pControlRate1->getMax();
    if (m_pControlRate1->getMin()==0.08)
        idx = 0.;

    ComboBoxRateRange->setCurrentItem((int)idx);

    ComboBoxRateDir->clear();
    ComboBoxRateDir->insertItem("Up increase speed");
    ComboBoxRateDir->insertItem("Down increase speed (Technics SL1210)");

    if (m_pControlRateDir1->getValue()==1)
        ComboBoxRateDir->setCurrentItem(0);
    else
        ComboBoxRateDir->setCurrentItem(1);
}

void DlgPrefControls::slotSetRateRange(int pos)
{
    float range = (float)(pos)/10.;
    if (pos==0)
        range = 0.08f;

    // Set the rate range
    m_pControlRate1->setRange(-range, range);
    m_pControlRate2->setRange(-range, range);
}

void DlgPrefControls::slotSetRateDir(int)
{
    float dir = 1.;
    if (ComboBoxRateDir->currentItem()==1)
        dir = -1.;

    // Set rate dir
    m_pControlRateDir1->setValueFromApp(dir);
    m_pControlRateDir2->setValueFromApp(dir);
}

void DlgPrefControls::slotSetVisuals(int)
{
    m_pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(ComboBoxVisuals->currentItem()));
    textLabel->setText("Restart Mixxx before the change of visuals will take effect.");
}

void DlgPrefControls::slotSetSkin(int)
{
    m_pConfig->set(ConfigKey("[Config]","Skin"), ComboBoxSkinconf->currentText());
    textLabel->setText("Restart Mixxx before the new skin will be loaded.");
}

void DlgPrefControls::slotApply()
{
    // Write rate range to config file
    float idx = 10.*m_pControlRate1->getMax();
    if (m_pControlRate1->getMin()==0.08)
        idx = 0.;
    m_pConfig->set(ConfigKey("[Controls]","RateRange"), ConfigValue((int)idx));

    // Write rate direction to config file
    if (m_pControlRateDir1->getValue()==1)
        m_pConfig->set(ConfigKey("[Controls]","RateDir"), ConfigValue(0));
    else
        m_pConfig->set(ConfigKey("[Controls]","RateDir"), ConfigValue(1));

    // Save preferences
    m_pConfig->Save();
}


                                                                                                 




    
