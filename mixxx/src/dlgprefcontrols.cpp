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
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "mixxxview.h"
#include "wnumberpos.h"
#include "wnumberbpm.h"
#include <qdir.h>
#include <qtooltip.h>
#include "enginebuffer.h"
#include <qspinbox.h>
#include <qwidget.h>

DlgPrefControls::DlgPrefControls(QWidget *parent, MixxxView *pView, MixxxApp *mixxx, ConfigObject<ConfigValue> *pConfig) : DlgPrefControlsDlg(parent,"")
{
    m_pView = pView;
    m_pConfig = pConfig;
	m_mixxx = mixxx;
    
    //
    // Rate slider configuration
    //

    m_pControlRate1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","rate")));
    m_pControlRate2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","rate")));
    m_pControlRateDir1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","rate_dir")));
    m_pControlRateDir2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","rate_dir")));
    m_pControlRateRange1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","rateRange")));
    m_pControlRateRange2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","rateRange")));

    // Set default direction as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateDir")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateDir"),ConfigValue(0));

    // Position display configuration
    ComboBoxPosition->insertItem("Position");
    ComboBoxPosition->insertItem("Remaining");
    if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"),ConfigValue(0));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).toInt() == 1)
    {
        if (pView->m_pNumberPosCh1)
            pView->m_pNumberPosCh1->setRemain(true);
        if (pView->m_pNumberPosCh2)
            pView->m_pNumberPosCh2->setRemain(true);
        ComboBoxPosition->setCurrentItem(1);
    }
    else
    {
        if (pView->m_pNumberPosCh1)
            pView->m_pNumberPosCh1->setRemain(false);
        if (pView->m_pNumberPosCh2)
            pView->m_pNumberPosCh2->setRemain(false);
        ComboBoxPosition->setCurrentItem(0);
    }
    connect(ComboBoxPosition,   SIGNAL(activated(int)), this, SLOT(slotSetPositionDisplay(int)));

    float fDir = 1.;
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateDir")).toInt()==1.)
        fDir = -1.;
    m_pControlRateDir1->slotSet(fDir);
    m_pControlRateDir2->slotSet(fDir);

    connect(ComboBoxRateDir,   SIGNAL(activated(int)), this, SLOT(slotSetRateDir(int)));

    // Set default range as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateRange")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateRange"),ConfigValue(1));

    slotSetRateRange(m_pConfig->getValueString(ConfigKey("[Controls]","RateRange")).toInt());
    connect(ComboBoxRateRange, SIGNAL(activated(int)), this, SLOT(slotSetRateRange(int)));

    //
    // Rate buttons configuration
    //
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateTempLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateTempLeft"),ConfigValue(50));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateTempRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateTempRight"),ConfigValue(10));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RatePermLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RatePermLeft"),ConfigValue(10));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RatePermRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RatePermRight"),ConfigValue(5));

    spinBoxTempRateLeft->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RateTempLeft")).toInt());
    spinBoxTempRateRight->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RateTempRight")).toInt());
    spinBoxPermRateLeft->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RatePermLeft")).toInt());
    spinBoxPermRateRight->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RatePermRight")).toInt());

    connect(spinBoxTempRateLeft, SIGNAL(valueChanged(int)), this, SLOT(slotSetRateTempLeft(int)));
    connect(spinBoxTempRateRight, SIGNAL(valueChanged(int)), this, SLOT(slotSetRateTempRight(int)));
    connect(spinBoxPermRateLeft, SIGNAL(valueChanged(int)), this, SLOT(slotSetRatePermLeft(int)));
    connect(spinBoxPermRateRight, SIGNAL(valueChanged(int)), this, SLOT(slotSetRatePermRight(int)));


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
#ifndef QT3_SUPPORT
    const QFileInfoList *list = dir.entryInfoList();
    if (list!=0)
    {
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                   // pointer for traversing
        int j=0;
        while ((fi=(*it)))
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
#else
    QList<QFileInfo> list = dir.entryInfoList(); 
    int j=0;
    for (int i=0; i<list.size(); ++i)
    {
        if (list.at(i).fileName()!="." && list.at(i).fileName()!="..")
        {
            ComboBoxSkinconf->insertItem(list.at(i).fileName());
            if (list.at(i).fileName() == pConfig->getValueString(ConfigKey("[Config]","Skin")))
                ComboBoxSkinconf->setCurrentItem(j);
            ++j;
        }
    }
#endif

    connect(ComboBoxSkinconf, SIGNAL(activated(int)), this, SLOT(slotSetSkin(int)));

    //
    // Scale BPM configuration
    //
    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","ScaleBpm")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","ScaleBpm"), ConfigValue(1));

    // Update combo box
    ComboBoxScaleBpm->setCurrentItem((m_pConfig->getValueString(ConfigKey("[Controls]","ScaleBpm")).toInt()+1)%2);

    connect(ComboBoxScaleBpm,   SIGNAL(activated(int)), this, SLOT(slotSetScaleBpm(int)));
    slotSetScaleBpm(0);


    //
    // Tooltip configuration
    //
    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue(1));

    // Update combo box
    ComboBoxTooltips->setCurrentItem((m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).toInt()+1)%2);

    connect(ComboBoxTooltips,   SIGNAL(activated(int)), this, SLOT(slotSetTooltips(int)));
    slotSetTooltips(0);
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

    float idx = 10.*m_pControlRateRange1->get();
    if (m_pControlRateRange1->get()==0.08)
        idx = 0.;
    
    ComboBoxRateRange->setCurrentItem((int)idx);

    ComboBoxRateDir->clear();
    ComboBoxRateDir->insertItem("Up increase speed");
    ComboBoxRateDir->insertItem("Down increase speed (Technics SL1210)");

    if (m_pControlRateDir1->get()==1)
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
    m_pControlRateRange1->slotSet(range);
    m_pControlRateRange2->slotSet(range);
    
    // Reset rate
    m_pControlRate1->slotSet(0.);
    m_pControlRate2->slotSet(0.);
}

void DlgPrefControls::slotSetRateDir(int)
{
    float dir = 1.;
    if (ComboBoxRateDir->currentItem()==1)
        dir = -1.;

    // Set rate dir
    m_pControlRateDir1->slotSet(dir);
    m_pControlRateDir2->slotSet(dir);
}

void DlgPrefControls::slotSetVisuals(int)
{
    m_pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(ComboBoxVisuals->currentItem()));
    textLabel->setText("Restart Mixxx before the change of visuals will take effect.");
}

void DlgPrefControls::slotSetScaleBpm(int)
{
    m_pConfig->set(ConfigKey("[Controls]","ScaleBpm"), ConfigValue((ComboBoxScaleBpm->currentItem()+1)%2));
    if (ComboBoxScaleBpm->currentItem()==0)
        WNumberBpm::setScaleBpm(true);
    else
        WNumberBpm::setScaleBpm(false);
}

void DlgPrefControls::slotSetTooltips(int)
{
// setGloballyEnabled currently not implemented in QT4
#ifndef QT3_SUPPORT
    m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue((ComboBoxTooltips->currentItem()+1)%2));
    if (ComboBoxTooltips->currentItem()==0)
        QToolTip::setGloballyEnabled(true);
    else
        QToolTip::setGloballyEnabled(false);
#endif
}

void DlgPrefControls::slotSetSkin(int)
{
    m_pConfig->set(ConfigKey("[Config]","Skin"), ComboBoxSkinconf->currentText());
    //textLabel->setText("Restart Mixxx before the new skin will be loaded.");
	m_mixxx->rebootMixxxView();
}

void DlgPrefControls::slotSetPositionDisplay(int)
{
    m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"), ConfigValue(ComboBoxPosition->currentItem()));

    if (ComboBoxPosition->currentItem()==1)
    {
        if (m_pView->m_pNumberPosCh1)
            m_pView->m_pNumberPosCh1->setRemain(true);
        if (m_pView->m_pNumberPosCh2)
            m_pView->m_pNumberPosCh2->setRemain(true);
    }
    else
    {
        if (m_pView->m_pNumberPosCh1)
            m_pView->m_pNumberPosCh1->setRemain(false);
        if (m_pView->m_pNumberPosCh2)
            m_pView->m_pNumberPosCh2->setRemain(false);
    }
}

void DlgPrefControls::slotSetRateTempLeft(int v)
{
    m_pConfig->set(ConfigKey("[Controls]","RateTempLeft"),ConfigValue(v));
    EngineBuffer::setTemp(v/1000.);
}

void DlgPrefControls::slotSetRateTempRight(int v)
{
    m_pConfig->set(ConfigKey("[Controls]","RateTempRight"),ConfigValue(v));
    EngineBuffer::setTempSmall(v/1000.);
}

void DlgPrefControls::slotSetRatePermLeft(int v)
{
    m_pConfig->set(ConfigKey("[Controls]","RatePermLeft"),ConfigValue(v));
    EngineBuffer::setPerm(v/1000.);
}

void DlgPrefControls::slotSetRatePermRight(int v)
{
    m_pConfig->set(ConfigKey("[Controls]","RatePermRight"),ConfigValue(v));
    EngineBuffer::setPermSmall(v/1000.);
}

void DlgPrefControls::slotApply()
{
    // Write rate range to config file
    float idx = 10.*m_pControlRateRange1->get();
    if (idx==0.8)
        idx = 0.;
    
    m_pConfig->set(ConfigKey("[Controls]","RateRange"), ConfigValue((int)idx));

    // Write rate direction to config file
    if (m_pControlRateDir1->get()==1)
        m_pConfig->set(ConfigKey("[Controls]","RateDir"), ConfigValue(0));
    else
        m_pConfig->set(ConfigKey("[Controls]","RateDir"), ConfigValue(1));
}







    
