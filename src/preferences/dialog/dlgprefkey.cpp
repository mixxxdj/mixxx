/***************************************************************************
                          dlgprefkey.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2012
    copyright            : (C) 2012 by Keith Salisbury
    email                : keithsalisbury@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "preferences/dialog/dlgprefkey.h"

#include <QLineEdit>
#include <QMessageBox>

#include "analyzer/analyzerkey.h"
#include "control/controlproxy.h"
#include "util/compatibility.h"
#include "util/xml.h"

DlgPrefKey::DlgPrefKey(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          Ui::DlgPrefKeyDlg(),
          m_keySettings(pConfig),
          m_bAnalyzerEnabled(m_keySettings.getKeyDetectionEnabledDefault()),
          m_bFastAnalysisEnabled(m_keySettings.getFastAnalysisDefault()),
          m_bReanalyzeEnabled(m_keySettings.getReanalyzeWhenSettingsChangeDefault()) {
    setupUi(this);

    m_keyLineEdits.insert(mixxx::track::io::key::C_MAJOR, c_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::D_FLAT_MAJOR, d_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::D_MAJOR, d_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_FLAT_MAJOR, e_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_MAJOR, e_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_MAJOR, f_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_SHARP_MAJOR, f_sharp_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::G_MAJOR, g_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::A_FLAT_MAJOR, a_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::A_MAJOR, a_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_FLAT_MAJOR, b_flat_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_MAJOR, b_major_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::C_MINOR, c_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::C_SHARP_MINOR, c_sharp_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::D_MINOR, d_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_FLAT_MINOR, e_flat_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::E_MINOR, e_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_MINOR, f_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::F_SHARP_MINOR, f_sharp_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::G_MINOR, g_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::G_SHARP_MINOR, g_sharp_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::A_MINOR, a_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_FLAT_MINOR, b_flat_minor_edit);
    m_keyLineEdits.insert(mixxx::track::io::key::B_MINOR, b_minor_edit);

    m_availablePlugins = AnalyzerKey::availablePlugins();
    for (const auto& info : qAsConst(m_availablePlugins)) {
        plugincombo->addItem(info.name, info.id);
    }

    m_pKeyNotation = new ControlProxy(ConfigKey("[Library]", "key_notation"), this);

    loadSettings();

    // Connections
    connect(plugincombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DlgPrefKey::pluginSelected);
    connect(banalyzerenabled, &QCheckBox::stateChanged,
            this, &DlgPrefKey::analyzerEnabled);
    connect(bfastAnalysisEnabled, &QCheckBox::stateChanged,
            this, &DlgPrefKey::fastAnalysisEnabled);
    connect(breanalyzeEnabled, &QCheckBox::stateChanged,
            this, &DlgPrefKey::reanalyzeEnabled);

    connect(radioNotationOpenKey, &QRadioButton::toggled,
            this, &DlgPrefKey::setNotationOpenKey);
    connect(radioNotationOpenKeyAndTraditional, &QRadioButton::toggled,
            this, &DlgPrefKey::setNotationOpenKeyAndTraditional);
    connect(radioNotationLancelot, &QRadioButton::toggled,
            this, &DlgPrefKey::setNotationLancelot);
    connect(radioNotationLancelotAndTraditional, &QRadioButton::toggled,
            this, &DlgPrefKey::setNotationLancelotAndTraditional);
    connect(radioNotationTraditional, &QRadioButton::toggled,
            this, &DlgPrefKey::setNotationTraditional);
    connect(radioNotationCustom, &QRadioButton::toggled,
            this, &DlgPrefKey::setNotationCustom);
}

DlgPrefKey::~DlgPrefKey() {
}

QUrl DlgPrefKey::helpUrl() const {
    return QUrl(MIXXX_MANUAL_KEY_URL);
}

void DlgPrefKey::loadSettings() {
    m_selectedAnalyzerId = m_keySettings.getKeyPluginId();
    qDebug() << "Key plugin ID:" << m_selectedAnalyzerId;

    m_bAnalyzerEnabled = m_keySettings.getKeyDetectionEnabled();
    m_bFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    m_bReanalyzeEnabled = m_keySettings.getReanalyzeWhenSettingsChange();

    QString notation_name = m_keySettings.getKeyNotation();
    KeyUtils::KeyNotation notation_type;
    QMap<mixxx::track::io::key::ChromaticKey, QString> notation;
    if (notation_name == KEY_NOTATION_CUSTOM) {
        radioNotationCustom->setChecked(true);
        for (auto it = m_keyLineEdits.constBegin();
                it != m_keyLineEdits.constEnd(); ++it) {
            it.value()->setText(m_keySettings.getCustomKeyNotation(it.key()));
            notation[it.key()] = it.value()->text();
        }
        setNotationCustom(true);
        notation_type = KeyUtils::KeyNotation::Custom;
    } else {
        if (notation_name == KEY_NOTATION_LANCELOT) {
            radioNotationLancelot->setChecked(true);
            notation_type = KeyUtils::KeyNotation::Lancelot;
        } else if (notation_name == KEY_NOTATION_LANCELOT_AND_TRADITIONAL) {
            radioNotationLancelotAndTraditional->setChecked(true);
            notation_type = KeyUtils::KeyNotation::LancelotAndTraditional;
        } else if (notation_name == KEY_NOTATION_TRADITIONAL) {
            radioNotationTraditional->setChecked(true);
            notation_type = KeyUtils::KeyNotation::Traditional;
        } else if (notation_name == KEY_NOTATION_OPEN_KEY_AND_TRADITIONAL) {
            radioNotationOpenKeyAndTraditional->setChecked(true);
            notation_type = KeyUtils::KeyNotation::OpenKeyAndTraditional;
        } else { // KEY_NOTATION_OPEN_KEY and unknown names
            radioNotationOpenKey->setChecked(true);
            notation_type = KeyUtils::KeyNotation::OpenKey;
        }

        // This is just a handy way to iterate the keys. We don't use the
        // QLineEdits.
        for (auto it = m_keyLineEdits.constBegin(); it != m_keyLineEdits.constEnd(); ++it) {
            notation[it.key()] = KeyUtils::keyToString(it.key(), notation_type);
        }
    }

    setNotation(notation_type);
    KeyUtils::setNotation(notation);
    m_pKeyNotation->set(static_cast<double>(notation_type));

    slotUpdate();
}

void DlgPrefKey::slotResetToDefaults() {
    // NOTE(rryan): Do not hard-code defaults here! Put them in
    // KeyDetectionSettings.
    m_bAnalyzerEnabled = m_keySettings.getKeyDetectionEnabledDefault();
    m_bFastAnalysisEnabled = m_keySettings.getFastAnalysisDefault();
    m_bReanalyzeEnabled = m_keySettings.getReanalyzeWhenSettingsChangeDefault();
    if (m_availablePlugins.size() > 0) {
        m_selectedAnalyzerId = m_availablePlugins[0].id;
    }

    KeyUtils::KeyNotation notation_type;
    QString defaultNotation = m_keySettings.getKeyNotationDefault();
    if (defaultNotation == KEY_NOTATION_LANCELOT) {
        radioNotationLancelot->setChecked(true);
        notation_type = KeyUtils::KeyNotation::Lancelot;
    } else if (defaultNotation == KEY_NOTATION_TRADITIONAL) {
        radioNotationTraditional->setChecked(true);
        notation_type = KeyUtils::KeyNotation::Traditional;
    } else if (defaultNotation == KEY_NOTATION_CUSTOM) {
        radioNotationCustom->setChecked(true);
        notation_type = KeyUtils::KeyNotation::Custom;
    } else { // KEY_NOTATION_OPEN_KEY
        radioNotationOpenKey->setChecked(true);
        notation_type = KeyUtils::KeyNotation::OpenKey;
    }
    setNotation(notation_type);

    slotUpdate();
}

void DlgPrefKey::pluginSelected(int i) {
    if (i == -1) {
        return;
    }
    m_selectedAnalyzerId = m_availablePlugins[i].id;
    slotUpdate();
}

void DlgPrefKey::analyzerEnabled(int i) {
    m_bAnalyzerEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::fastAnalysisEnabled(int i) {
    m_bFastAnalysisEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::reanalyzeEnabled(int i){
    m_bReanalyzeEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefKey::slotApply() {
    m_keySettings.setKeyPluginId(m_selectedAnalyzerId);
    m_keySettings.setKeyDetectionEnabled(m_bAnalyzerEnabled);
    m_keySettings.setFastAnalysis(m_bFastAnalysisEnabled);
    m_keySettings.setReanalyzeWhenSettingsChange(m_bReanalyzeEnabled);

    QString notation_name;
    KeyUtils::KeyNotation notation_type;
    QMap<mixxx::track::io::key::ChromaticKey, QString> notation;
    if (radioNotationCustom->isChecked()) {
        notation_name = KEY_NOTATION_CUSTOM;
        notation_type = KeyUtils::KeyNotation::Custom;
        for (auto it = m_keyLineEdits.constBegin();
                it != m_keyLineEdits.constEnd(); ++it) {
            notation[it.key()] = it.value()->text();
            m_keySettings.setCustomKeyNotation(it.key(), it.value()->text());
        }
    } else {
        if (radioNotationOpenKey->isChecked()) {
            notation_name = KEY_NOTATION_OPEN_KEY;
            notation_type = KeyUtils::KeyNotation::OpenKey;
        } else if (radioNotationOpenKeyAndTraditional->isChecked()) {
            notation_name = KEY_NOTATION_OPEN_KEY_AND_TRADITIONAL;
            notation_type = KeyUtils::KeyNotation::OpenKeyAndTraditional;
        } else if (radioNotationLancelotAndTraditional->isChecked()) {
            notation_name = KEY_NOTATION_LANCELOT_AND_TRADITIONAL;
            notation_type = KeyUtils::KeyNotation::LancelotAndTraditional;
        } else if (radioNotationTraditional->isChecked()) {
            notation_name = KEY_NOTATION_TRADITIONAL;
            notation_type = KeyUtils::KeyNotation::Traditional;
        } else {
            // Either Lancelot was chosen or somehow no radio button was chosen.
            notation_name = KEY_NOTATION_LANCELOT;
            notation_type = KeyUtils::KeyNotation::Lancelot;
        }

        // This is just a handy way to iterate the keys. We don't use the
        // QLineEdits.
        for (auto it = m_keyLineEdits.constBegin(); it != m_keyLineEdits.constEnd(); ++it) {
            notation[it.key()] = KeyUtils::keyToString(it.key(), notation_type);
        }
    }

    m_keySettings.setKeyNotation(notation_name);
    KeyUtils::setNotation(notation);
    m_pKeyNotation->set(static_cast<double>(notation_type));
}

void DlgPrefKey::slotUpdate() {
    plugincombo->setEnabled(m_bAnalyzerEnabled);
    banalyzerenabled->setChecked(m_bAnalyzerEnabled);
    bfastAnalysisEnabled->setChecked(m_bFastAnalysisEnabled);
    bfastAnalysisEnabled->setEnabled(m_bAnalyzerEnabled);
    breanalyzeEnabled->setChecked(m_bReanalyzeEnabled);
    breanalyzeEnabled->setEnabled(m_bAnalyzerEnabled);

    if (!m_bAnalyzerEnabled) {
        return;
    }

    if (m_availablePlugins.size() > 0) {
        bool found = false;
        for (int i = 0; i < m_availablePlugins.size(); ++i) {
            const auto& info = m_availablePlugins.at(i);
            if (info.id == m_selectedAnalyzerId) {
                plugincombo->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        if (!found) {
            plugincombo->setCurrentIndex(0);
            m_selectedAnalyzerId = m_availablePlugins[0].id;
        }
    }
}

void DlgPrefKey::setNotationCustom(bool active) {
    if (!active) {
        return;
    }

    for (auto it = m_keyLineEdits.constBegin();
            it != m_keyLineEdits.constEnd(); ++it) {
        it.value()->setEnabled(true);
    }
    slotUpdate();
}

void DlgPrefKey::setNotation(KeyUtils::KeyNotation notation) {
    for (auto it = m_keyLineEdits.constBegin();
            it != m_keyLineEdits.constEnd(); ++it) {
        it.value()->setText(KeyUtils::keyToString(it.key(), notation));
        it.value()->setEnabled(false);
    }
    slotUpdate();
}

void DlgPrefKey::setNotationTraditional(bool active) {
    if (active) {
        setNotation(KeyUtils::KeyNotation::Traditional);
    }
}

void DlgPrefKey::setNotationOpenKey(bool active) {
    if (active) {
        setNotation(KeyUtils::KeyNotation::OpenKey);
    }
}

void DlgPrefKey::setNotationOpenKeyAndTraditional(bool active) {
    if (active) {
        setNotation(KeyUtils::KeyNotation::OpenKeyAndTraditional);
    }
}

void DlgPrefKey::setNotationLancelot(bool active) {
    if (active) {
        setNotation(KeyUtils::KeyNotation::Lancelot);
    }
}

void DlgPrefKey::setNotationLancelotAndTraditional(bool active) {
    if (active) {
        setNotation(KeyUtils::KeyNotation::LancelotAndTraditional);
    }
}
