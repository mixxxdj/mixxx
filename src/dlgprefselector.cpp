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
    connect(horizontalSliderBpmRange, SIGNAL(stateChanged(int)),
        this, SLOT(filterBpmRange(int)));
    connect(checkBoxKey, SIGNAL(stateChanged(int)),
        this, SLOT(filterKeyEnabled(int)));
    connect(checkBoxKey4th, SIGNAL(stateChanged(int)),
        this, SLOT(filterKey4thEnabled(int)));
    connect(checkBoxKey5th, SIGNAL(stateChanged(int)),
        this, SLOT(filterKey5thEnabled(int)));
    connect(checkBoxKeyRelative, SIGNAL(stateChanged(int)),
        this, SLOT(filterKeyRelativeEnabled(int)));
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
    slotApply();
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


void DlgPrefSelector::loadSettings() {
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

    slotUpdate();
}
