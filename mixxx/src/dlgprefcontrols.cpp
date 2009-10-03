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

#include <QList>
#include <QDir>
#include <QToolTip>
#include <QDoubleSpinBox>
#include <QWidget>

#include "dlgprefcontrols.h"
#include "qcombobox.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "mixxxview.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberbpm.h"
#include "engine/enginebuffer.h"
#include "engine/ratecontrol.h"

DlgPrefControls::DlgPrefControls(QWidget * parent, MixxxView * pView, MixxxApp * mixxx, ConfigObject<ConfigValue> * pConfig) :  QWidget(parent), Ui::DlgPrefControlsDlg()
{
    m_pView = pView;
    m_pConfig = pConfig;
    m_mixxx = mixxx;

    setupUi(this);

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
    ComboBoxPosition->addItem("Position");
    ComboBoxPosition->addItem("Remaining");
    if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"),ConfigValue(0));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).toInt() == 1)
    {
        if (pView->m_pNumberPosCh1)
            pView->m_pNumberPosCh1->setRemain(true);
        if (pView->m_pNumberPosCh2)
            pView->m_pNumberPosCh2->setRemain(true);
        ComboBoxPosition->setCurrentIndex(1);
    }
    else
    {
        if (pView->m_pNumberPosCh1)
            pView->m_pNumberPosCh1->setRemain(false);
        if (pView->m_pNumberPosCh2)
            pView->m_pNumberPosCh2->setRemain(false);
        ComboBoxPosition->setCurrentIndex(0);
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
        m_pConfig->set(ConfigKey("[Controls]","RateTempLeft"),ConfigValue(QString("4.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateTempRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateTempRight"),ConfigValue(QString("2.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RatePermLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RatePermLeft"),ConfigValue(QString("1.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RatePermRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RatePermRight"),ConfigValue(QString("1.0")));

    connect(spinBoxTempRateLeft, SIGNAL(valueChanged(double)), this, SLOT(slotSetRateTempLeft(double)));
    connect(spinBoxTempRateRight, SIGNAL(valueChanged(double)), this, SLOT(slotSetRateTempRight(double)));
    connect(spinBoxPermRateLeft, SIGNAL(valueChanged(double)), this, SLOT(slotSetRatePermLeft(double)));
    connect(spinBoxPermRateRight, SIGNAL(valueChanged(double)), this, SLOT(slotSetRatePermRight(double)));

    spinBoxTempRateLeft->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RateTempLeft")).toDouble());
    spinBoxTempRateRight->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RateTempRight")).toDouble());
    spinBoxPermRateLeft->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RatePermLeft")).toDouble());
    spinBoxPermRateRight->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RatePermRight")).toDouble());


    //
    // Visuals
    //

    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","Visuals")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(0));

    // Update combo box
    ComboBoxVisuals->addItem("Waveform");
    ComboBoxVisuals->addItem("Simple");
    ComboBoxVisuals->setCurrentIndex(m_pConfig->getValueString(ConfigKey("[Controls]","Visuals")).toInt());

    connect(ComboBoxVisuals,   SIGNAL(activated(int)), this, SLOT(slotSetVisuals(int)));

    //
    // Skin configurations
    //
    ComboBoxSkinconf->clear();

    QString qSkinPath(pConfig->getValueString(ConfigKey("[Config]","Path")));
    QDir dir(qSkinPath.append("skins/"));
    dir.setFilter(QDir::Dirs);
    
    //
    // Default Cue Behavior
    //
    
    m_pControlCueDefault1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","cue_mode")));
    m_pControlCueDefault2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","cue_mode")));
    
    // Set default value in config file and control objects, if not present
    QString cueDefault = m_pConfig->getValueString(ConfigKey("[Controls]","CueDefault"));
    if(cueDefault.length() == 0) {
        m_pConfig->set(ConfigKey("[Controls]","CueDefault"), ConfigValue(0));
        cueDefault = "0";
    }
    int cueDefaultValue = cueDefault.toInt();

    m_pControlCueDefault1->slotSet(cueDefaultValue);
    m_pControlCueDefault2->slotSet(cueDefaultValue);
    
    // Update combo box
    ComboBoxCueDefault->addItem("CDJ Mode");
    ComboBoxCueDefault->addItem("Simple");
    ComboBoxCueDefault->setCurrentIndex(m_pConfig->getValueString(ConfigKey("[Controls]","CueDefault")).toInt());
   
    connect(ComboBoxCueDefault,   SIGNAL(activated(int)), this, SLOT(slotSetCueDefault(int)));

    //Cue recall
    ComboBoxCueRecall->addItem(tr("On"));
    ComboBoxCueRecall->addItem(tr("Off"));
    ComboBoxCueRecall->setCurrentIndex(m_pConfig->getValueString(ConfigKey("[Controls]", "CueRecall")).toInt());
    //NOTE: for CueRecall, 0 means ON....
    connect(ComboBoxCueRecall, SIGNAL(activated(int)), this, SLOT(slotSetCueRecall(int)));


// #ifndef QT3_SUPPORT
//     const QFileInfoList * list = dir.entryInfoList();
//     if (list!=0)
//     {
//         QFileInfoListIterator it(* list);        // create list iterator
//         QFileInfo * fi;                   // pointer for traversing
//         int j=0;
//         while ((fi=(*it)))
//         {
//             if (fi->fileName()!="." && fi->fileName()!="..")
//             {
//                 ComboBoxSkinconf->addItem(fi->fileName());
//                 if (fi->fileName() == pConfig->getValueString(ConfigKey("[Config]","Skin")))
//                     ComboBoxSkinconf->setCurrentIndex(j);
//                 ++j;
//             }
//             ++it;
//         }
//     }
// #else


    QList<QFileInfo> list = dir.entryInfoList();
    int j=0;
    for (int i=0; i<list.size(); ++i)
    {
        if (list.at(i).fileName()!="." && list.at(i).fileName()!="..")
        {
            ComboBoxSkinconf->addItem(list.at(i).fileName());
            if (list.at(i).fileName() == pConfig->getValueString(ConfigKey("[Config]","Skin")))
                ComboBoxSkinconf->setCurrentIndex(j);
            ++j;
        }
    }
// #endif

   // Detect small display and prompt user to use small skin.
   if (QApplication::desktop()->width() >= 800 && QApplication::desktop()->height() == 480 && pConfig->getValueString(ConfigKey("[Config]","Skin"))!= "outlineMini") {
      int ret = QMessageBox::warning(this, tr("Mixxx Detected a WVGA Screen"), tr("Mixxx has detected that your screen has a resolution of ") +
                   QString::number(QApplication::desktop()->width()) + " x " + QString::number(QApplication::desktop()->height()) + ".  " +
                   tr("The only skin compatiable with this size display is OutlineMini (800x480).  Would you like to use that skin?"),
                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
      if (ret == QMessageBox::Yes) {
         pConfig->set(ConfigKey("[Config]","Skin"), ConfigValue("outlineMini"));
         pConfig->Save();
         ComboBoxSkinconf->setCurrentIndex(ComboBoxSkinconf->findText(pConfig->getValueString(ConfigKey("[Config]","Skin"))));
         qDebug() << "Retrieved skin:" << pConfig->getValueString(ConfigKey("[Config]","Skin")) << "ComboBoxSkinconf:" << ComboBoxSkinconf->currentText();
         slotSetSkin(1);
      }
    }

    connect(ComboBoxSkinconf, SIGNAL(activated(int)), this, SLOT(slotSetSkin(int)));

    slotUpdateSchemes();

    connect(ComboBoxSchemeconf, SIGNAL(activated(int)), this, SLOT(slotSetScheme(int)));
    //
    // Scale BPM configuration
    //
    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","ScaleBpm")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","ScaleBpm"), ConfigValue(1));

    // Update combo box
    ComboBoxScaleBpm->setCurrentIndex((m_pConfig->getValueString(ConfigKey("[Controls]","ScaleBpm")).toInt()+1)%2);

    connect(ComboBoxScaleBpm,   SIGNAL(activated(int)), this, SLOT(slotSetScaleBpm(int)));
    slotSetScaleBpm(0);


    //
    // Tooltip configuration
    //
    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue(1));

    // Update combo box
    ComboBoxTooltips->setCurrentIndex((m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).toInt()+1)%2);

    connect(ComboBoxTooltips,   SIGNAL(activated(int)), this, SLOT(slotSetTooltips(int)));

    slotUpdateSchemes();
    slotUpdate();
}

DlgPrefControls::~DlgPrefControls()
{
}

void DlgPrefControls::slotUpdateSchemes()
{
    // Since this involves opening a file we won't do this as part of regular slotUpdate
    QList<QString> schlist = MixxxView::getSchemeList(m_mixxx->getSkinPath());

    ComboBoxSchemeconf->clear();

    if (schlist.size() == 0) {
        ComboBoxSchemeconf->setEnabled(false);
        ComboBoxSchemeconf->addItem("This skin does not support schemes", 0);
        ComboBoxSchemeconf->setCurrentIndex(0);
    } else {
        ComboBoxSchemeconf->setEnabled(true);
        for (int i = 0; i < schlist.size(); i++) {
            ComboBoxSchemeconf->addItem(schlist[i]);
            
            if (schlist[i] == m_pConfig->getValueString(ConfigKey("[Config]","Scheme"))) {
                ComboBoxSchemeconf->setCurrentIndex(i);
            }
        }
    }
}

void DlgPrefControls::slotUpdate()
{
    ComboBoxRateRange->clear();
    ComboBoxRateRange->addItem(" 8% (Technics SL1210)");
    ComboBoxRateRange->addItem("10% ");
    ComboBoxRateRange->addItem("20%");
    ComboBoxRateRange->addItem("30%");
    ComboBoxRateRange->addItem("40%");
    ComboBoxRateRange->addItem("50%");
    ComboBoxRateRange->addItem("60%");
    ComboBoxRateRange->addItem("70%");
    ComboBoxRateRange->addItem("80%");
    ComboBoxRateRange->addItem("90%");

    float idx = 10.*m_pControlRateRange1->get();
    if (m_pControlRateRange1->get()==0.08)
        idx = 0.;

    ComboBoxRateRange->setCurrentIndex((int)idx);

    ComboBoxRateDir->clear();
    ComboBoxRateDir->addItem("Up increases speed");
    ComboBoxRateDir->addItem("Down increases speed (Technics SL1210)");

    if (m_pControlRateDir1->get()==1)
        ComboBoxRateDir->setCurrentIndex(0);
    else
        ComboBoxRateDir->setCurrentIndex(1);
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
    if (ComboBoxRateDir->currentIndex()==1)
        dir = -1.;

    // Set rate dir
    m_pControlRateDir1->slotSet(dir);
    m_pControlRateDir2->slotSet(dir);
}

void DlgPrefControls::slotSetVisuals(int)
{
    m_pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(ComboBoxVisuals->currentIndex()));
    QMessageBox::information(this, tr("Information"), //make the fact that you have to restart mixxx more obvious
    //textLabel->setText(
      tr("Restart Mixxx before the change of visuals will take effect.")
      );
}

void DlgPrefControls::slotSetCueDefault(int)
{
    m_pConfig->set(ConfigKey("[Controls]","CueDefault"), ConfigValue(ComboBoxCueDefault->currentIndex()));
    m_pControlCueDefault1->slotSet(ComboBoxCueDefault->currentIndex());
    m_pControlCueDefault2->slotSet(ComboBoxCueDefault->currentIndex());
}

void DlgPrefControls::slotSetCueRecall(int)
{
    m_pConfig->set(ConfigKey("[Controls]","CueRecall"), ConfigValue(ComboBoxCueRecall->currentIndex()));
}

void DlgPrefControls::slotSetScaleBpm(int)
{
    m_pConfig->set(ConfigKey("[Controls]","ScaleBpm"), ConfigValue((ComboBoxScaleBpm->currentIndex()+1)%2));
    if (ComboBoxScaleBpm->currentIndex()==0)
        WNumberBpm::setScaleBpm(true);
    else
        WNumberBpm::setScaleBpm(false);
}

void DlgPrefControls::slotSetTooltips(int)
{
// setGloballyEnabled currently not implemented in QT4
//#ifndef QT3_SUPPORT
    m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue((ComboBoxTooltips->currentIndex()+1)%2));

    //This is somewhat confusing, but to disable tooltips in QT4, you need to install an eventFilter
    //on the QApplication object. That object is located in MixxxApp (mixxx.cpp/h), so that's where
    //the eventFilter is. The value of the ConfigObject is cached at startup because it's too slow
    //to refresh it during each Tooltip event (I think), which is why we require a restart.

    
    QMessageBox::information(this, tr("Information"), //make the fact that you have to restart mixxx more obvious
    //textLabel->setText(
      tr("Mixxx must be restarted before the changes will take effect."));
    

//    if (ComboBoxTooltips->currentIndex()==0)
//        QToolTip::setGloballyEnabled(true);
//    else
//        QToolTip::setGloballyEnabled(false);
//#endif
}

void DlgPrefControls::slotSetScheme(int)
{
    m_pConfig->set(ConfigKey("[Config]", "Scheme"), ComboBoxSchemeconf->currentText());
    m_mixxx->rebootMixxxView();
}

void DlgPrefControls::slotSetSkin(int)
{
    m_pConfig->set(ConfigKey("[Config]","Skin"), ComboBoxSkinconf->currentText());
    m_mixxx->rebootMixxxView();
    slotUpdateSchemes();
}

void DlgPrefControls::slotSetPositionDisplay(int)
{
    m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"), ConfigValue(ComboBoxPosition->currentIndex()));

    if (ComboBoxPosition->currentIndex()==1)
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

void DlgPrefControls::slotSetRateTempLeft(double v)
{
    QString str;
    str = str.setNum(v, 'f', 1);
    m_pConfig->set(ConfigKey("[Controls]","RateTempLeft"),ConfigValue(str));
    RateControl::setTemp(v);
}

void DlgPrefControls::slotSetRateTempRight(double v)
{
    QString str;
    str = str.setNum(v, 'f', 1);
    m_pConfig->set(ConfigKey("[Controls]","RateTempRight"),ConfigValue(str));
    RateControl::setTempSmall(v);
}

void DlgPrefControls::slotSetRatePermLeft(double v)
{
    QString str;
    str = str.setNum(v, 'f', 1);
    m_pConfig->set(ConfigKey("[Controls]","RatePermLeft"),ConfigValue(str));
    RateControl::setPerm(v);
}

void DlgPrefControls::slotSetRatePermRight(double v)
{
    QString str;
    str = str.setNum(v, 'f', 1);
    m_pConfig->set(ConfigKey("[Controls]","RatePermRight"),ConfigValue(str));
    RateControl::setPermSmall(v);
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

