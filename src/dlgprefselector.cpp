#include <qwidget.h>
#include <QtDebug>

#include "dlgprefselector.h"
#include "ui_dlgprefselectordlg.h"
#include "library/selector/selector_preferences.h"

DlgPrefSelector::DlgPrefSelector(QWidget *parent,
                                 ConfigObject<ConfigValue> *pConfig)
        :  DlgPreferencePage(parent),
        Ui::DlgPrefSelectorDlg(),
        m_pConfig(pConfig) {
    setupUi(this);
    m_similaritySliders.insert("timbre", horizontalSliderTimbre);
    m_similaritySliders.insert("rhythm", horizontalSliderRhythm);
    m_similaritySliders.insert("lastfm", horizontalSliderLastFm);

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
    connect(horizontalSliderTimbre, SIGNAL(sliderMoved(int)),
            this, SLOT(setTimbreContribution(int)));
    connect(horizontalSliderRhythm, SIGNAL(sliderPressed()),
            this, SLOT(displayRhythmDescription()));
    connect(horizontalSliderRhythm, SIGNAL(sliderMoved(int)),
            this, SLOT(setRhythmContribution(int)));
    connect(horizontalSliderLastFm, SIGNAL(sliderPressed()),
            this, SLOT(displayLastFmDescription()));
    connect(horizontalSliderLastFm, SIGNAL(sliderMoved(int)),
            this, SLOT(setLastFmContribution(int)));
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
        ConfigValue(m_similarityContributions.value("timbre")));
    m_pConfig->set(
        ConfigKey(SELECTOR_CONFIG_KEY, RHYTHM_COEFFICIENT),
        ConfigValue(m_similarityContributions.value("rhythm")));
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

    foreach (QString key, m_similarityContributions.keys()) {
        int value = m_similarityContributions.value(key);
        QSlider* pSlider = m_similaritySliders.value(key);
        pSlider->setValue(value);
    }
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
    m_similarityContributions.insert("timbre", 50);
    m_similarityContributions.insert("rhythm", 49);
    m_similarityContributions.insert("lastfm", 1);
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

void DlgPrefSelector::setTimbreContribution(int value) {
    setContribution("timbre", value);
}

void DlgPrefSelector::setRhythmContribution(int value) {
    setContribution("rhythm", value);
}

void DlgPrefSelector::setLastFmContribution(int value) {
    setContribution("lastfm", value);
}

void DlgPrefSelector::setContribution(QString key, int value) {
    m_similarityContributions.insert(key, value);
    double scale = 0.0;
    foreach (int contribution, m_similarityContributions.values()) {
        scale += contribution;
    }
    scale = (100 - value) / (scale - value);

    foreach (QString otherKey, m_similarityContributions.keys()) {
        if (otherKey != key) {
            int otherValue = m_similarityContributions.value(otherKey);
            otherValue = (int) (otherValue * scale);
            if (otherValue < 1) otherValue = 1;
            m_similarityContributions.insert(otherKey, otherValue);

            QSlider* pSlider = m_similaritySliders.value(otherKey);
            pSlider->setValue(otherValue);
        }
    }
}

void DlgPrefSelector::displayTimbreDescription() {
    labelDescriptionText->setText(tr("Timbre: "
                                     "Do the tracks use similar instruments? "
                                     "How similar do the tracks sound?"));
}

void DlgPrefSelector::displayRhythmDescription() {
    labelDescriptionText->setText(tr("Rhythm: "
                                     "How similar are the rhythms "
                                     "of each track? "
                                     "Would the beats match well?"));
}

void DlgPrefSelector::displayLastFmDescription() {
    labelDescriptionText->setText(tr("Tags: "
                                     "Have people tagged these tracks in a "
                                     "similar way? Are they likely to be in "
                                     "the same genre?"));
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
    m_similarityContributions.insert("timbre", m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, TIMBRE_COEFFICIENT)).toInt());
    m_similarityContributions.insert("rhythm", m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, RHYTHM_COEFFICIENT)).toInt());

    slotUpdate();
}
