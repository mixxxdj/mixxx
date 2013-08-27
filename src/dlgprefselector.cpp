#include <qwidget.h>
#include <QtDebug>

#include "dlgprefselector.h"
#include "ui_dlgprefselectordlg.h"
#include "library/selector/selector_preferences.h"

DlgPrefSelector::DlgPrefSelector(QWidget *parent,
                                 ConfigObject<ConfigValue> *pConfig)
    : QWidget(parent),
      Ui::DlgPrefSelectorDlg(),
      m_pConfig(pConfig) {
    setupUi(this);
    loadSettings();

    connect(checkBoxGenre, SIGNAL(stateChanged(int)),
            this, SLOT(filterGenreEnabled(int)));
    connect(checkBoxBpm, SIGNAL(stateChanged(int)),
            this, SLOT(filterBpmEnabled(int)));
    connect(horizontalSliderBpmRange, SIGNAL(valueChanged(int)),
            this, SLOT(filterBpmRange(int)));
    connect(checkBoxKey, SIGNAL(stateChanged(int)),
            this, SLOT(filterKeyEnabled(int)));
    connect(checkBoxKey4th, SIGNAL(stateChanged(int)),
            this, SLOT(filterKey4thEnabled(int)));
    connect(checkBoxKey5th, SIGNAL(stateChanged(int)),
            this, SLOT(filterKey5thEnabled(int)));
    connect(checkBoxKeyRelative, SIGNAL(stateChanged(int)),
            this, SLOT(filterKeyRelativeEnabled(int)));
    connect(horizontalSliderTimbre, SIGNAL(sliderPressed()),
            this, SLOT(displayTimbreDescription()));
    connect(horizontalSliderTimbre, SIGNAL(valueChanged(int)),
            this, SLOT(setTimbreCoefficient(int)));
    connect(horizontalSliderRhythm, SIGNAL(sliderPressed()),
            this, SLOT(displayRhythmDescription()));
    connect(horizontalSliderRhythm, SIGNAL(valueChanged(int)),
            this, SLOT(setRhythmCoefficient(int)));
    connect(horizontalSliderLastFm, SIGNAL(sliderPressed()),
            this, SLOT(displayLastFmDescription()));
    connect(horizontalSliderLastFm, SIGNAL(valueChanged(int)),
            this, SLOT(setLastFmCoefficient(int)));
}

DlgPrefSelector::~DlgPrefSelector() {
}

void DlgPrefSelector::slotApply() {
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_GENRE),
        ConfigValue(m_bFilterGenre ? 1 : 0));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM),
        ConfigValue(m_bFilterBpm ? 1 : 0));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM_RANGE),
        ConfigValue(m_iFilterBpmRange));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY),
        ConfigValue(m_bFilterKey ? 1 : 0));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_4TH),
        ConfigValue(m_bFilterKey4th ? 1 : 0));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_5TH),
        ConfigValue(m_bFilterKey5th ? 1 : 0));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_RELATIVE),
        ConfigValue(m_bFilterKeyRelative ? 1 : 0));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, TIMBRE_COEFFICIENT),
        ConfigValue(m_iTimbreCoefficient));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, RHYTHM_COEFFICIENT),
        ConfigValue(m_iRhythmCoefficient));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, LASTFM_COEFFICIENT),
        ConfigValue(m_iLastFmCoefficient));
    m_pConfig->Save();
}

void DlgPrefSelector::slotUpdate() {
    checkBoxGenre->setChecked(m_bFilterGenre);
    checkBoxBpm->setChecked(m_bFilterBpm);
    horizontalSliderBpmRange->setValue(m_iFilterBpmRange);
    checkBoxKey->setChecked(m_bFilterKey);
    checkBoxKey4th->setChecked(m_bFilterKey4th);
    checkBoxKey5th->setChecked(m_bFilterKey5th);
    checkBoxKeyRelative->setChecked(m_bFilterKeyRelative);
    horizontalSliderTimbre->setValue(m_iTimbreCoefficient);
    horizontalSliderRhythm->setValue(m_iRhythmCoefficient);
    horizontalSliderLastFm->setValue(m_iLastFmCoefficient);
    slotApply();
}

void DlgPrefSelector::setDefaults() {
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, HAS_RUN),
        ConfigValue(1));
    m_bFilterGenre = false;
    m_bFilterBpm = false;
    m_iFilterBpmRange = 5;
    m_bFilterKey = false;
    m_bFilterKey4th = false;
    m_bFilterKey5th = false;
    m_bFilterKeyRelative = false;
    m_iTimbreCoefficient = 50;
    m_iRhythmCoefficient = 50;
    m_iLastFmCoefficient = 0;
}

void DlgPrefSelector::filterGenreEnabled(int value) {
    m_bFilterGenre = static_cast<bool>(value);
}

void DlgPrefSelector::filterBpmEnabled(int value) {
    m_bFilterBpm = static_cast<bool>(value);
}

void DlgPrefSelector::filterBpmRange(int value) {
    m_iFilterBpmRange = value;
}

void DlgPrefSelector::filterKeyEnabled(int value) {
    m_bFilterKey = static_cast<bool>(value);
}

void DlgPrefSelector::filterKey4thEnabled(int value) {
    m_bFilterKey4th = static_cast<bool>(value);
}

void DlgPrefSelector::filterKey5thEnabled(int value) {
    m_bFilterKey5th = static_cast<bool>(value);
}

void DlgPrefSelector::filterKeyRelativeEnabled(int value) {
    m_bFilterKeyRelative = static_cast<bool>(value);
}

void DlgPrefSelector::setTimbreCoefficient(int value) {
    m_iTimbreCoefficient = value;
}

void DlgPrefSelector::setRhythmCoefficient(int value) {
    m_iRhythmCoefficient = value;
}

void DlgPrefSelector::setLastFmCoefficient(int value) {
    m_iLastFmCoefficient = value;
}

void DlgPrefSelector::displayTimbreDescription() {
    descriptionBox->setText(tr("Compare track timbres."));
}

void DlgPrefSelector::displayRhythmDescription() {
    descriptionBox->setText(tr("Compare track rhythms "
                               "(cosine distance between beat spectra)."));
}

void DlgPrefSelector::displayLastFmDescription() {
    descriptionBox->setText(tr("Compare tracks using Last.fm tags."));
}

void DlgPrefSelector::loadSettings() {
    if (m_pConfig->
            getValueString(ConfigKey(SELECTOR_CONFIG_KEY, HAS_RUN)).toInt()
            == 0) {
        setDefaults();
        slotApply();
        return;
    }

    m_bFilterGenre = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_GENRE)).toInt());
    m_bFilterBpm = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM)).toInt());
    m_iFilterBpmRange = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM_RANGE)).toInt();
    m_bFilterKey = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY)).toInt());
    m_bFilterKey4th = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_4TH)).toInt());
    m_bFilterKey5th = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_5TH)).toInt());
    m_bFilterKeyRelative = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_RELATIVE)).toInt());
    m_iTimbreCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, TIMBRE_COEFFICIENT)).toInt();
    m_iRhythmCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, RHYTHM_COEFFICIENT)).toInt();
    m_iLastFmCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, LASTFM_COEFFICIENT)).toInt();

    slotUpdate();
}
