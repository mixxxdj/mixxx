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
#include <QLocale>

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
#include "waveform/waveformwidgetfactory.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "playermanager.h"
#include "controlobject.h"
#include "mixxx.h"

DlgPrefControls::DlgPrefControls(QWidget * parent, MixxxApp * mixxx,
                                 SkinLoader* pSkinLoader,
                                 PlayerManager* pPlayerManager,
                                 ConfigObject<ConfigValue> * pConfig)
        :  QWidget(parent) {
    m_pConfig = pConfig;
    m_timer = -1;
    m_mixxx = mixxx;
    m_pSkinLoader = pSkinLoader;
    m_pPlayerManager = pPlayerManager;

    setupUi(this);

    for (unsigned int i = 0; i < m_pPlayerManager->numDecks(); ++i) {
        QString group = QString("[Channel%1]").arg(i+1);
        m_rateControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "rate"))));
        m_rateRangeControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "rateRange"))));
        m_rateDirControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "rate_dir"))));
        m_cueControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "cue_mode"))));
    }

    for (unsigned int i = 0; i < m_pPlayerManager->numSamplers(); ++i) {
        QString group = QString("[Sampler%1]").arg(i+1);
        m_rateControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "rate"))));
        m_rateRangeControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "rateRange"))));
        m_rateDirControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "rate_dir"))));
        m_cueControls.push_back(new ControlObjectThreadMain(
            ControlObject::getControl(ConfigKey(group, "cue_mode"))));
    }


    // Track time display configuration
    m_pControlTrackTimeDisplay = new ControlObject(ConfigKey("[Controls]", "ShowTrackTimeRemaining"));
    connect(m_pControlTrackTimeDisplay, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackTimeDisplay(double)));

    // Upgrade from 1.10.x and below.
    if (m_pConfig->exists(ConfigKey("[Controls]","PositionDisplay"))) {
        if (m_pConfig->getValueString(ConfigKey("[Controls]","PositionDisplay")).toInt() == 1) {
            m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"),ConfigValue("Remaining"));
        }
        else {
            m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"),ConfigValue("Elapsed"));
        }
        // Remove the now-obsolete configuration entry
        m_pConfig->remove(ConfigKey("[Controls]","PositionDisplay"));
    } // End upgrade
    
    // If not present in the config, set the default value
    if (!m_pConfig->exists(ConfigKey("[Controls]","TrackTimeDisplay")))
        m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"),ConfigValue("Remaining"));
    
    if (m_pConfig->getValueString(ConfigKey("[Controls]","TrackTimeDisplay")) == "Remaining")
    {
        radioButtonRemaining->setChecked(true);
        m_pControlTrackTimeDisplay->set(1.0f);
    }
    else
    {
        radioButtonElapsed->setChecked(true);
        m_pControlTrackTimeDisplay->set(0.0f);
    }
    connect(buttonGroupTrackTime, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotSetTrackTimeDisplay(QAbstractButton *)));


    // Set default direction as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateDir")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateDir"),ConfigValue(0));

    slotSetRateDir(m_pConfig->getValueString(ConfigKey("[Controls]","RateDir")).toInt()==1);
    connect(checkBoxInvertPitchSlider, SIGNAL(toggled(bool)), this, SLOT(slotSetRateDir(bool)));

    // Set default range as stored in config file
    if (m_pConfig->getValueString(ConfigKey("[Controls]","RateRange")).length() == 0)
        m_pConfig->set(ConfigKey("[Controls]","RateRange"),ConfigValue(2));

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
    // Override Playing Track on Track Load
    //
    checkBoxDontLoadToPlayingDecks->setChecked(
        m_pConfig->getValueString(ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck")).toInt()==0);
    connect(checkBoxDontLoadToPlayingDecks, SIGNAL(toggled(bool)),
            this, SLOT(slotSetAllowTrackLoadToPlayingDeck(bool)));

    //
    // Locale setting
    //

    // Iterate through the available locales and add them to the combobox
    // Borrowed following snippet from http://qt-project.org/wiki/How_to_create_a_multi_language_application
    QString translationsFolder = m_pConfig->getResourcePath() + "translations/";
    QString currentLocale = pConfig->getValueString(ConfigKey("[Config]","Locale"));

    QDir translationsDir(translationsFolder);
    QStringList fileNames = translationsDir.entryList(QStringList("mixxx_*.qm"));

    ComboBoxLocale->addItem("System", ""); // System default locale
    ComboBoxLocale->setCurrentIndex(0);

    for (int i = 0; i < fileNames.size(); ++i) {
        // Extract locale from filename
        QString locale = fileNames[i];
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);

        QString lang = QLocale::languageToString(QLocale(locale).language());
        if (lang == "C") { // Ugly hack to remove the non-resolving locales
            continue;
        }

        ComboBoxLocale->addItem(lang, locale); // locale as userdata (for storing to config)
        if (locale == currentLocale) { // Set the currently selected locale
            ComboBoxLocale->setCurrentIndex(ComboBoxLocale->count() - 1);
        }
    }
    connect(ComboBoxLocale, SIGNAL(activated(int)),
            this, SLOT(slotSetLocale(int)));

    //
    // Default Cue Behavior
    //

    // Set default value in config file and control objects, if not present
    QString cueDefault = m_pConfig->getValueString(ConfigKey("[Controls]","CueDefault"));
    if(cueDefault.length() == 0) {
        m_pConfig->set(ConfigKey("[Controls]","CueDefault"), ConfigValue(0));
        cueDefault = "0";
    }
    int cueDefaultValue = cueDefault.toInt();

    buttonGroupCueBehavior->setId(radioButtonCueCdj, 0);
    buttonGroupCueBehavior->setId(radioButtonCueSimple, 1);
    if (cueDefaultValue == 0)
        radioButtonCueCdj->setChecked(true);
    else if (cueDefaultValue == 1)
        radioButtonCueSimple->setChecked(true);

    slotSetCueDefault(cueDefaultValue);
    connect(buttonGroupCueBehavior, SIGNAL(buttonClicked(int)), this, SLOT(slotSetCueDefault(int)));

    //Cue recall
    //NOTE: for CueRecall, 0 means ON....
    connect(checkBoxJumpToCueOnLoad, SIGNAL(toggled(bool)),
            this, SLOT(slotSetCueRecall(bool)));
    checkBoxJumpToCueOnLoad->setChecked(m_pConfig->getValueString(ConfigKey("[Controls]", "CueRecall")).toInt()==0);

    //
    // Skin configurations
    //
    QString warningString = "<img src=\":/images/preferences/ic_preferences_warning.png\") width=16 height=16 />"
        + tr("The selected skin is bigger than your screen resolution.");
    warningLabel->setText(warningString);

    ComboBoxSkinconf->clear();

    QDir dir(m_pConfig->getResourcePath() + "skins/");
    dir.setFilter(QDir::Dirs);

    QString configuredSkinPath = m_pSkinLoader->getConfiguredSkinPath();

    QList<QFileInfo> list = dir.entryInfoList();
    int j=0;
    for (int i=0; i<list.size(); ++i)
    {
        if (list.at(i).fileName()!="." && list.at(i).fileName()!="..")
        {
            checkSkinResolution(list.at(i).fileName())
                    ? ComboBoxSkinconf->insertItem(i, QIcon(":/trolltech/styles/commonstyle/images/standardbutton-apply-32.png"), list.at(i).fileName())
                    : ComboBoxSkinconf->insertItem(i, QIcon(":/images/preferences/ic_preferences_warning.png"), list.at(i).fileName());

            if (list.at(i).filePath() == configuredSkinPath) {
                ComboBoxSkinconf->setCurrentIndex(j);
            }
            ++j;
        }
    }

    connect(ComboBoxSkinconf, SIGNAL(activated(int)), this, SLOT(slotSetSkin(int)));
    connect(ComboBoxSchemeconf, SIGNAL(activated(int)), this, SLOT(slotSetScheme(int)));

    checkSkinResolution(ComboBoxSkinconf->currentText())
             ? warningLabel->hide() : warningLabel->show();
    slotUpdateSchemes();

    //
    // Tooltip configuration
    //
    // Set default value in config file if not present
    if (!m_pConfig->exists(ConfigKey("[Controls]","Tooltips")))
        m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue(0));

    // Initialize checkboxes to match config
    //0=ON, 1=ON (only in Library), 2=OFF
    switch (m_pConfig->getValueString(ConfigKey("[Controls]","Tooltips")).toInt()) {
        case 0:
            checkBoxTooltipsEnabled->setChecked(true);
            checkBoxTooltipsOnlyLibrary->setChecked(false);
            break;
        case 1:
            checkBoxTooltipsEnabled->setChecked(true);
            checkBoxTooltipsOnlyLibrary->setChecked(true);
            break;
        case 2:
            checkBoxTooltipsEnabled->setChecked(false);
            break;
    }

    slotSetTooltips();  // Update disabled status of "only library" checkbox
    connect(buttonGroupTooltips, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(slotSetTooltips()));

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

    slotUpdate();

    initWaveformControl();
}

DlgPrefControls::~DlgPrefControls()
{
    foreach (ControlObjectThreadMain* pControl, m_rateControls) {
        delete pControl;
    }
    foreach (ControlObjectThreadMain* pControl, m_rateDirControls) {
        delete pControl;
    }
    foreach (ControlObjectThreadMain* pControl, m_cueControls) {
        delete pControl;
    }
    foreach (ControlObjectThreadMain* pControl, m_rateRangeControls) {
        delete pControl;
    }
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
    ComboBoxRateRange->addItem(tr("6%"));
    ComboBoxRateRange->addItem(tr("8% (Technics SL-1210)"));
    ComboBoxRateRange->addItem(tr("10%"));
    ComboBoxRateRange->addItem(tr("20%"));
    ComboBoxRateRange->addItem(tr("30%"));
    ComboBoxRateRange->addItem(tr("40%"));
    ComboBoxRateRange->addItem(tr("50%"));
    ComboBoxRateRange->addItem(tr("60%"));
    ComboBoxRateRange->addItem(tr("70%"));
    ComboBoxRateRange->addItem(tr("80%"));
    ComboBoxRateRange->addItem(tr("90%"));

    double deck1RateRange = m_rateRangeControls[0]->get();
    double deck1RateDir = m_rateDirControls[0]->get();

    double idx = (10. * deck1RateRange) + 1;
    if (deck1RateRange <= 0.07)
        idx = 0.;
    else if (deck1RateRange <= 0.09)
        idx = 1.;

    ComboBoxRateRange->setCurrentIndex((int)idx);

    if (deck1RateDir == 1)
        checkBoxInvertPitchSlider->setChecked(false);
    else
        checkBoxInvertPitchSlider->setChecked(true);
}

void DlgPrefControls::slotSetLocale(int pos) {
    QString newLocale = ComboBoxLocale->itemData(pos).toString();
    m_pConfig->set(ConfigKey("[Config]","Locale"), ConfigValue(newLocale));
    notifyRebootNecessary();
}

void DlgPrefControls::slotSetRateRange(int pos)
{
    float range = (float)(pos-1)/10.;
    if (pos==0)
        range = 0.06f;
    if (pos==1)
        range = 0.08f;

    // Set rate range for every group
    foreach (ControlObjectThreadMain* pControl, m_rateRangeControls) {
        pControl->slotSet(range);
    }

    // Reset rate for every group
    foreach (ControlObjectThreadMain* pControl, m_rateControls) {
        pControl->slotSet(0);
    }
}

void DlgPrefControls::slotSetRateDir(bool invert)
{
    float dir = 1.;
    if (invert)
        dir = -1.;

    // Set rate direction for every group
    foreach (ControlObjectThreadMain* pControl, m_rateDirControls) {
        pControl->slotSet(dir);
    }
}

void DlgPrefControls::slotSetAllowTrackLoadToPlayingDeck(bool b)
{
    m_pConfig->set(ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck"),
                   ConfigValue(b?0:1));
}

void DlgPrefControls::slotSetCueDefault(int index)
{
    m_pConfig->set(ConfigKey("[Controls]","CueDefault"), ConfigValue(index));

    // Set cue behavior for every group
    foreach (ControlObjectThreadMain* pControl, m_cueControls) {
        pControl->slotSet(index);
    }
}

void DlgPrefControls::slotSetCueRecall(bool b)
{
    m_pConfig->set(ConfigKey("[Controls]","CueRecall"), ConfigValue(b?0:1));
}

void DlgPrefControls::slotSetTooltips()
{
    //0=ON, 1=ON (only in Library), 2=OFF
    int valueToSet = 0;
    if (!checkBoxTooltipsEnabled->isChecked()) {
        checkBoxTooltipsOnlyLibrary->setDisabled(true);
        valueToSet = 2;
    }
    else {
        checkBoxTooltipsOnlyLibrary->setDisabled(false);
        if (checkBoxTooltipsOnlyLibrary->isChecked()) {
            valueToSet = 1;
        }
    }
    m_pConfig->set(ConfigKey("[Controls]","Tooltips"), ConfigValue(valueToSet));
    m_mixxx->setToolTips(valueToSet);
}

void DlgPrefControls::notifyRebootNecessary() {
    // make the fact that you have to restart mixxx more obvious
    QMessageBox::information(
        this, tr("Information"),
        tr("Mixxx must be restarted before the changes will take effect."));
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
    checkSkinResolution(ComboBoxSkinconf->currentText())
            ? warningLabel->hide() : warningLabel->show();
    slotUpdateSchemes();
}

void DlgPrefControls::slotSetTrackTimeDisplay(QAbstractButton* b)
{
    int timeDisplay = 0;
    if (b == radioButtonRemaining) {
        timeDisplay = 1;
        m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"), ConfigValue("Remaining"));
    }
    else {
        m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"), ConfigValue("Elapsed"));
    }
    m_pControlTrackTimeDisplay->set(timeDisplay);
}

void DlgPrefControls::slotSetTrackTimeDisplay(double v) {
    if (v > 0) {
        // Remaining
        radioButtonRemaining->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"), ConfigValue("Remaining"));
    } else {
        // Elapsed
        radioButtonElapsed->setChecked(true);
        m_pConfig->set(ConfigKey("[Controls]","TrackTimeDisplay"), ConfigValue("Elapsed"));
    }
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
    double deck1RateRange = m_rateRangeControls[0]->get();
    double deck1RateDir = m_rateDirControls[0]->get();

    // Write rate range to config file
    double idx = (10. * deck1RateRange) + 1;
    if (deck1RateRange <= 0.07)
        idx = 0.;
    else if (deck1RateRange <= 0.09)
        idx = 1.;

    m_pConfig->set(ConfigKey("[Controls]","RateRange"), ConfigValue((int)idx));

    // Write rate direction to config file
    if (deck1RateDir == 1)
        m_pConfig->set(ConfigKey("[Controls]","RateDir"), ConfigValue(0));
    else
        m_pConfig->set(ConfigKey("[Controls]","RateDir"), ConfigValue(1));

}

void DlgPrefControls::slotSetFrameRate(int frameRate) {
    WaveformWidgetFactory::instance()->setFrameRate(frameRate);
}

void DlgPrefControls::slotSetWaveformType(int index) {
    if (WaveformWidgetFactory::instance()->setWidgetTypeFromHandle(index)) {
        // It was changed to a valid type. Previously we rebooted the Mixxx GUI
        // here but now we can update the waveforms on the fly.
    }
}

void DlgPrefControls::slotSetDefaultZoom(int index) {
    WaveformWidgetFactory::instance()->setDefaultZoom( index + 1);
}

void DlgPrefControls::slotSetZoomSynchronization(bool checked) {
    WaveformWidgetFactory::instance()->setZoomSync(checked);
}

void DlgPrefControls::slotSetVisualGainAll(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::All,gain);
}

void DlgPrefControls::slotSetVisualGainLow(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::Low,gain);
}

void DlgPrefControls::slotSetVisualGainMid(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::Mid,gain);
}

void DlgPrefControls::slotSetVisualGainHigh(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::High,gain);
}

void DlgPrefControls::slotSetNormalizeOverview( bool normalize) {
    WaveformWidgetFactory::instance()->setOverviewNormalized(normalize);
}

void DlgPrefControls::onShow() {
    m_timer = startTimer(100); //refresh actual frame rate every 100 ms
}

void DlgPrefControls::onHide() {
    if (m_timer != -1) {
        killTimer(m_timer);
    }
}

void DlgPrefControls::timerEvent(QTimerEvent * /*event*/) {
    //Just to refresh actual framrate any time the controller is modified
    frameRateAverage->setText(QString::number(
        WaveformWidgetFactory::instance()->getActualFrameRate()));
}

void DlgPrefControls::initWaveformControl()
{
    waveformTypeComboBox->clear();
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();

    if (factory->isOpenGLAvailable())
        openGlStatusIcon->setText(factory->getOpenGLVersion());
    else
        openGlStatusIcon->setText(tr("OpenGL not available"));

    WaveformWidgetType::Type currentType = factory->getType();
    int currentIndex = -1;

    std::vector<WaveformWidgetAbstractHandle> handles = factory->getAvailableTypes();
    for (unsigned int i = 0; i < handles.size(); i++) {
        waveformTypeComboBox->addItem(handles[i].getDisplayName());
        if (handles[i].getType() == currentType)
            currentIndex = i;
    }

    if (currentIndex != -1)
        waveformTypeComboBox->setCurrentIndex(currentIndex);

    frameRateSpinBox->setValue(factory->getFrameRate());

    synchronizeZoomCheckBox->setChecked( factory->isZoomSync());
    allVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::All));
    lowVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::Low));
    midVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::Mid));
    highVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::High));
    normalizeOverviewCheckBox->setChecked(factory->isOverviewNormalized());

    for( int i = WaveformWidgetRenderer::s_waveformMinZoom;
         i <= WaveformWidgetRenderer::s_waveformMaxZoom;
         i++) {
        defaultZoomComboBox->addItem(QString::number( 100/double(i),'f',1) + " %");
    }
    defaultZoomComboBox->setCurrentIndex( factory->getDefaultZoom() - 1);

    connect(frameRateSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(slotSetFrameRate(int)));
    connect(waveformTypeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSetWaveformType(int)));
    connect(defaultZoomComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSetDefaultZoom(int)));
    connect(synchronizeZoomCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(slotSetZoomSynchronization(bool)));
    connect(allVisualGain,SIGNAL(valueChanged(double)),
            this,SLOT(slotSetVisualGainAll(double)));
    connect(lowVisualGain,SIGNAL(valueChanged(double)),
            this,SLOT(slotSetVisualGainLow(double)));
    connect(midVisualGain,SIGNAL(valueChanged(double)),
            this,SLOT(slotSetVisualGainMid(double)));
    connect(highVisualGain,SIGNAL(valueChanged(double)),
            this,SLOT(slotSetVisualGainHigh(double)));
    connect(normalizeOverviewCheckBox,SIGNAL(toggled(bool)),
            this,SLOT(slotSetNormalizeOverview(bool)));

}

//Returns TRUE if skin fits to screen resolution, FALSE otherwise
bool DlgPrefControls::checkSkinResolution(QString skin)
{
    int screenWidth = QApplication::desktop()->width();
    int screenHeight = QApplication::desktop()->height();

    QString skinName = skin.left(skin.indexOf(QRegExp("\\d")));
    QString resName = skin.right(skin.count()-skinName.count());
    QString res = resName.left(resName.lastIndexOf(QRegExp("\\d"))+1);
    QString skinWidth = res.left(res.indexOf("x"));
    QString skinHeight = res.right(res.count()-skinWidth.count()-1);

    return !(skinWidth.toInt() > screenWidth || skinHeight.toInt() > screenHeight);
}
