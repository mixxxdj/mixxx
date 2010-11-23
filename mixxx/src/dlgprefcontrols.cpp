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
#include "widget/wnumberpos.h"
#include "engine/enginebuffer.h"
#include "engine/ratecontrol.h"
#include "skin/skinloader.h"
#include "skin/legacyskinparser.h"

DlgPrefControls::DlgPrefControls(QWidget * parent, MixxxApp * mixxx,
                                 SkinLoader* pSkinLoader,
                                 ConfigObject<ConfigValue> * pConfig)
        :  QWidget(parent), Ui::DlgPrefControlsDlg() {
    m_pConfig = pConfig;
    m_mixxx = mixxx;
    m_pSkinLoader = pSkinLoader;

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
    m_pControlPositionDisplay = new ControlObject(ConfigKey("[Controls]", "ShowDurationRemaining"));
    ComboBoxPosition->addItem(tr("Position"));
    ComboBoxPosition->addItem(tr("Remaining"));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"),ConfigValue(0));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).toInt() == 1)
    {
        ComboBoxPosition->setCurrentIndex(1);
        m_pControlPositionDisplay->set(1.0f);
    }
    else
    {
        ComboBoxPosition->setCurrentIndex(0);
        m_pControlPositionDisplay->set(0.0f);
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
    //NOTE: THESE DEFAULTS ARE A LIE! You'll need to hack the same values into the static variables
    //      at the top of enginebuffer.cpp
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateTempLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateTempLeft"),ConfigValue(QString("4.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateTempRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateTempRight"),ConfigValue(QString("2.0")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RatePermLeft")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RatePermLeft"),ConfigValue(QString("0.50")));
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RatePermRight")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RatePermRight"),ConfigValue(QString("0.05")));

    connect(spinBoxTempRateLeft, SIGNAL(valueChanged(double)), this, SLOT(slotSetRateTempLeft(double)));
    connect(spinBoxTempRateRight, SIGNAL(valueChanged(double)), this, SLOT(slotSetRateTempRight(double)));
    connect(spinBoxPermRateLeft, SIGNAL(valueChanged(double)), this, SLOT(slotSetRatePermLeft(double)));
    connect(spinBoxPermRateRight, SIGNAL(valueChanged(double)), this, SLOT(slotSetRatePermRight(double)));

    spinBoxTempRateLeft->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RateTempLeft")).toDouble());
    spinBoxTempRateRight->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RateTempRight")).toDouble());
    spinBoxPermRateLeft->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RatePermLeft")).toDouble());
    spinBoxPermRateRight->setValue(m_pConfig->getValueString(ConfigKey("[Controls]","RatePermRight")).toDouble());

    SliderRateRampSensitivity->setEnabled(true);
    SpinBoxRateRampSensitivity->setEnabled(true);

    //
    // Visuals
    //

    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","Visuals")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(0));

    // Update combo box
    ComboBoxVisuals->addItem(tr("On"));
    ComboBoxVisuals->addItem(tr("Off"));
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
    ComboBoxCueDefault->addItem(tr("CDJ Mode"));
    ComboBoxCueDefault->addItem(tr("Simple"));
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
   if (QApplication::desktop()->width() >= 800 && QApplication::desktop()->height() == 480 && pConfig->getValueString(ConfigKey("[Config]","Skin"))!= "Outline800x480-WVGA") {
      int ret = QMessageBox::warning(this, tr("Mixxx Detected a WVGA Screen"), tr("Mixxx has detected that your screen has a resolution of ") +
                   QString::number(QApplication::desktop()->width()) + " x " + QString::number(QApplication::desktop()->height()) + ".  " +
                   tr("The only skin compatiable with this size display is Outline800x480-WVGA.  Would you like to use that skin?"),
                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
      if (ret == QMessageBox::Yes) {
         pConfig->set(ConfigKey("[Config]","Skin"), ConfigValue("Outline800x480-WVGA"));
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
    // Tooltip configuration
    //
    // Set default value in config file, if not present
    if (m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue(1));

    // Update combo box
    ComboBoxTooltips->setCurrentIndex((m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).toInt()+1)%2);

    //
    // Ramping Temporary Rate Change configuration
    //

    // Set Ramp Rate On or Off
    connect(groupBoxRateRamp, SIGNAL(toggled(bool)), this, SLOT(slotSetRateRamp(bool)));
    groupBoxRateRamp->setChecked((bool)
            m_pConfig->getValueString(ConfigKey("[Controls]","RateRamp")).toInt()
    );

    // Update Ramp Rate Sensitivity
    connect(SliderRateRampSensitivity, SIGNAL(valueChanged(int)), this, SLOT(slotSetRateRampSensitivity(int)));
    SliderRateRampSensitivity->setValue(
        m_pConfig->getValueString(ConfigKey("[Controls]","RateRampSensitivity")).toInt()
    );

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
    QList<QString> schlist = LegacySkinParser::getSchemeList(
        m_pSkinLoader->getConfiguredSkinPath());

    ComboBoxSchemeconf->clear();

    if (schlist.size() == 0) {
        ComboBoxSchemeconf->setEnabled(false);
        ComboBoxSchemeconf->addItem(tr("This skin does not support schemes", 0));
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
    ComboBoxRateDir->addItem(tr("Up increases speed"));
    ComboBoxRateDir->addItem(tr("Down increases speed (Technics SL1210)"));

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
    m_mixxx->rebootMixxxView();
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
    int positionDisplay = ComboBoxPosition->currentIndex();
    m_pConfig->set(ConfigKey("[Controls]","PositionDisplay"), ConfigValue(positionDisplay));
    m_pControlPositionDisplay->set(positionDisplay);
}

void DlgPrefControls::slotSetRateTempLeft(double v)
{
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]","RateTempLeft"),ConfigValue(str));
    RateControl::setTemp(v);
}

void DlgPrefControls::slotSetRateTempRight(double v)
{
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]","RateTempRight"),ConfigValue(str));
    RateControl::setTempSmall(v);
}

void DlgPrefControls::slotSetRatePermLeft(double v)
{
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]","RatePermLeft"),ConfigValue(str));
    RateControl::setPerm(v);
}

void DlgPrefControls::slotSetRatePermRight(double v)
{
    QString str;
    str = str.setNum(v, 'f');
    m_pConfig->set(ConfigKey("[Controls]","RatePermRight"),ConfigValue(str));
    RateControl::setPermSmall(v);
}

void DlgPrefControls::slotSetRateRampSensitivity(int sense)
{
    m_pConfig->set(ConfigKey("[Controls]","RateRampSensitivity"),
                        ConfigValue(SliderRateRampSensitivity->value()));
    RateControl::setRateRampSensitivity(sense);
}

void DlgPrefControls::slotSetRateRamp(bool mode)
{
    m_pConfig->set(ConfigKey("[Controls]", "RateRamp"),
                        ConfigValue(groupBoxRateRamp->isChecked()));
    RateControl::setRateRamp(mode);

    /*
    if ( mode )
    {
        SliderRateRampSensitivity->setEnabled(TRUE);
        SpinBoxRateRampSensitivity->setEnabled(TRUE);
    }
    else
    {
        SliderRateRampSensitivity->setEnabled(FALSE);
        SpinBoxRateRampSensitivity->setEnabled(FALSE);
    }*/
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

