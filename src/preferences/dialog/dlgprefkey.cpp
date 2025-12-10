#include "preferences/dialog/dlgprefkey.h"

#include "analyzer/analyzerkey.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "library/library_prefs.h"
#include "moc_dlgprefkey.cpp"

DlgPrefKey::DlgPrefKey(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          Ui::DlgPrefKeyDlg(),
          m_keySettings(pConfig),
          m_bAnalyzerEnabled(m_keySettings.getKeyDetectionEnabledDefault()),
          m_bFastAnalysisEnabled(m_keySettings.getFastAnalysisDefault()),
          m_bReanalyzeEnabled(m_keySettings.getReanalyzeWhenSettingsChangeDefault()),
          m_bDetect432HzEnabled(m_keySettings.getDetect432HzDefault()),
          m_bDetectTuningEnabled(m_keySettings.getDetectTuningFrequencyDefault()),
          m_tuningMinFreq(m_keySettings.getTuningMinFrequencyDefault()),
          m_tuningMaxFreq(m_keySettings.getTuningMaxFrequencyDefault()),
          m_tuningStepFreq(m_keySettings.getTuningStepFrequencyDefault()),
          m_stemStrategy(KeyDetectionSettings::StemStrategy::Disabled) {
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
    for (const auto& info : std::as_const(m_availablePlugins)) {
        plugincombo->addItem(info.name(), info.id());
    }

    m_pKeyNotation = new ControlProxy(mixxx::library::prefs::kKeyNotationConfigKey, this);

    loadSettings();

    // TODO (#13466) Keeping the setting hidden for now
    comboBoxStemStrategy->hide();
    labelStemStrategy->hide();

    // Connections
    connect(plugincombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DlgPrefKey::pluginSelected);
    setScrollSafeGuard(plugincombo);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(banalyzerenabled, &QCheckBox::checkStateChanged,
#else
    connect(banalyzerenabled, &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefKey::analyzerEnabled);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(bfastAnalysisEnabled, &QCheckBox::checkStateChanged,
#else
    connect(bfastAnalysisEnabled, &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefKey::fastAnalysisEnabled);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(breanalyzeEnabled, &QCheckBox::checkStateChanged,
#else
    connect(breanalyzeEnabled, &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefKey::reanalyzeEnabled);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(bdetect432HzEnabled, &QCheckBox::checkStateChanged,
#else
    connect(bdetect432HzEnabled, &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefKey::detect432HzEnabled);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(bdetectTuningEnabled, &QCheckBox::checkStateChanged,
#else
    connect(bdetectTuningEnabled, &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefKey::detectTuningEnabled);

    connect(spinTuningMin,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefKey::tuningMinChanged);
    connect(spinTuningMax,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefKey::tuningMaxChanged);
    connect(spinTuningStep,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefKey::tuningStepChanged);
    setScrollSafeGuard(spinTuningMin);
    setScrollSafeGuard(spinTuningMax);
    setScrollSafeGuard(spinTuningStep);

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
    connect(comboBoxStemStrategy,
            &QComboBox::currentIndexChanged,
            this,
            &DlgPrefKey::slotStemStrategyChanged);
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
    m_bDetect432HzEnabled = m_keySettings.getDetect432Hz();
    m_bDetectTuningEnabled = m_keySettings.getDetectTuningFrequency();
    m_tuningMinFreq = m_keySettings.getTuningMinFrequency();
    m_tuningMaxFreq = m_keySettings.getTuningMaxFrequency();
    m_tuningStepFreq = m_keySettings.getTuningStepFrequency();

    KeyUtils::KeyNotation notation_type =
            KeyUtils::keyNotationFromString(m_keySettings.getKeyNotation());
    QMap<mixxx::track::io::key::ChromaticKey, QString> notation;
    if (notation_type == KeyUtils::KeyNotation::Custom) {
        radioNotationCustom->setChecked(true);
        // Read the custom notation from the config and store it in a temp QMap
        for (auto it = m_keyLineEdits.constBegin();
                it != m_keyLineEdits.constEnd(); ++it) {
            it.value()->setText(m_keySettings.getCustomKeyNotation(it.key()));
            notation[it.key()] = it.value()->text();
        }
    } else {
        if (notation_type == KeyUtils::KeyNotation::Lancelot) {
            radioNotationLancelot->setChecked(true);
        } else if (notation_type == KeyUtils::KeyNotation::LancelotAndTraditional) {
            radioNotationLancelotAndTraditional->setChecked(true);
        } else if (notation_type == KeyUtils::KeyNotation::Traditional) {
            radioNotationTraditional->setChecked(true);
        } else if (notation_type == KeyUtils::KeyNotation::OpenKeyAndTraditional) {
            radioNotationOpenKeyAndTraditional->setChecked(true);
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

    // Store notation map for later recall...
    KeyUtils::setNotation(notation);
    // ... BEFORE invoking setNotation() which populates the QLineEdits from
    // the map retrieved from KeyUtils.
    setNotation(notation_type);
    m_pKeyNotation->set(static_cast<double>(notation_type));

    slotUpdate();
    m_stemStrategy = m_keySettings.getStemStrategy();
}

void DlgPrefKey::slotResetToDefaults() {
    // NOTE(rryan): Do not hard-code defaults here! Put them in
    // KeyDetectionSettings.
    m_bAnalyzerEnabled = m_keySettings.getKeyDetectionEnabledDefault();
    m_bFastAnalysisEnabled = m_keySettings.getFastAnalysisDefault();
    m_bReanalyzeEnabled = m_keySettings.getReanalyzeWhenSettingsChangeDefault();
    m_bDetect432HzEnabled = m_keySettings.getDetect432HzDefault();
    m_bDetectTuningEnabled = m_keySettings.getDetectTuningFrequencyDefault();
    m_tuningMinFreq = m_keySettings.getTuningMinFrequencyDefault();
    m_tuningMaxFreq = m_keySettings.getTuningMaxFrequencyDefault();
    m_tuningStepFreq = m_keySettings.getTuningStepFrequencyDefault();
    if (m_availablePlugins.size() > 0) {
        m_selectedAnalyzerId = m_availablePlugins[0].id();
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
    setNotation(notation_type); // calls slotUpdate()
    m_stemStrategy = m_keySettings.getStemStrategyDefault();
}

void DlgPrefKey::pluginSelected(int i) {
    if (i == -1) {
        return;
    }
    m_selectedAnalyzerId = m_availablePlugins[i].id();
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefKey::analyzerEnabled(Qt::CheckState state) {
    m_bAnalyzerEnabled = (state == Qt::Checked);
#else
void DlgPrefKey::analyzerEnabled(int i) {
    m_bAnalyzerEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefKey::fastAnalysisEnabled(Qt::CheckState state) {
    m_bFastAnalysisEnabled = (state == Qt::Checked);
#else
void DlgPrefKey::fastAnalysisEnabled(int i) {
    m_bFastAnalysisEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefKey::reanalyzeEnabled(Qt::CheckState state) {
    m_bReanalyzeEnabled = (state == Qt::Checked);
#else
void DlgPrefKey::reanalyzeEnabled(int i) {
    m_bReanalyzeEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefKey::detect432HzEnabled(Qt::CheckState state) {
    m_bDetect432HzEnabled = (state == Qt::Checked);
#else
void DlgPrefKey::detect432HzEnabled(int i) {
    m_bDetect432HzEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefKey::detectTuningEnabled(Qt::CheckState state) {
    m_bDetectTuningEnabled = (state == Qt::Checked);
#else
void DlgPrefKey::detectTuningEnabled(int i) {
    m_bDetectTuningEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

void DlgPrefKey::tuningMinChanged(int value) {
    m_tuningMinFreq = value;
    // Ensure max >= min
    if (m_tuningMaxFreq < m_tuningMinFreq) {
        m_tuningMaxFreq = m_tuningMinFreq;
    }
    slotUpdate();
}

void DlgPrefKey::tuningMaxChanged(int value) {
    m_tuningMaxFreq = value;
    // Ensure min <= max
    if (m_tuningMinFreq > m_tuningMaxFreq) {
        m_tuningMinFreq = m_tuningMaxFreq;
    }
    slotUpdate();
}

void DlgPrefKey::tuningStepChanged(int value) {
    m_tuningStepFreq = value;
    slotUpdate();
}

void DlgPrefKey::slotStemStrategyChanged(int index) {
    switch (index) {
    case 1:
        m_stemStrategy = KeyDetectionSettings::StemStrategy::Enforced;
        break;
    default:
        m_stemStrategy = KeyDetectionSettings::StemStrategy::Disabled;
        break;
    }
    slotUpdate();
}

void DlgPrefKey::slotApply() {
    m_keySettings.setKeyPluginId(m_selectedAnalyzerId);
    m_keySettings.setKeyDetectionEnabled(m_bAnalyzerEnabled);
    m_keySettings.setFastAnalysis(m_bFastAnalysisEnabled);
    m_keySettings.setReanalyzeWhenSettingsChange(m_bReanalyzeEnabled);
    m_keySettings.setDetect432Hz(m_bDetect432HzEnabled);
    m_keySettings.setDetectTuningFrequency(m_bDetectTuningEnabled);
    m_keySettings.setTuningMinFrequency(m_tuningMinFreq);
    m_keySettings.setTuningMaxFrequency(m_tuningMaxFreq);
    m_keySettings.setTuningStepFrequency(m_tuningStepFreq);

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
    m_keySettings.setStemStrategy(m_stemStrategy);
}

void DlgPrefKey::slotUpdate() {
    plugincombo->setEnabled(m_bAnalyzerEnabled);
    banalyzerenabled->setChecked(m_bAnalyzerEnabled);
    bfastAnalysisEnabled->setChecked(m_bFastAnalysisEnabled);
    bfastAnalysisEnabled->setEnabled(m_bAnalyzerEnabled);
    breanalyzeEnabled->setChecked(m_bReanalyzeEnabled);
    breanalyzeEnabled->setEnabled(m_bAnalyzerEnabled);
    bdetect432HzEnabled->setChecked(m_bDetect432HzEnabled);
    bdetect432HzEnabled->setEnabled(m_bAnalyzerEnabled);

    // Tuning detection controls
    bdetectTuningEnabled->setChecked(m_bDetectTuningEnabled);
    bdetectTuningEnabled->setEnabled(m_bAnalyzerEnabled);
    spinTuningMin->setValue(m_tuningMinFreq);
    spinTuningMax->setValue(m_tuningMaxFreq);
    spinTuningStep->setValue(m_tuningStepFreq);
    // Enable tuning range controls only when analyzer and tuning detection are enabled
    bool tuningControlsEnabled = m_bAnalyzerEnabled && m_bDetectTuningEnabled;
    spinTuningMin->setEnabled(tuningControlsEnabled);
    spinTuningMax->setEnabled(tuningControlsEnabled);
    spinTuningStep->setEnabled(tuningControlsEnabled);
    labelTuningRange->setEnabled(tuningControlsEnabled);
    labelTuningTo->setEnabled(tuningControlsEnabled);
    labelTuningStep->setEnabled(tuningControlsEnabled);

    if (!m_bAnalyzerEnabled) {
        return;
    }

    if (m_availablePlugins.size() > 0) {
        bool found = false;
        for (int i = 0; i < m_availablePlugins.size(); ++i) {
            const auto& info = m_availablePlugins.at(i);
            if (info.id() == m_selectedAnalyzerId) {
                plugincombo->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        if (!found) {
            plugincombo->setCurrentIndex(0);
            m_selectedAnalyzerId = m_availablePlugins[0].id();
        }
    }
    comboBoxStemStrategy->setCurrentIndex(
            m_stemStrategy == KeyDetectionSettings::StemStrategy::Enforced ? 1
                                                                           : 0);
}

void DlgPrefKey::setNotation(KeyUtils::KeyNotation notation) {
    for (auto it = m_keyLineEdits.constBegin();
            it != m_keyLineEdits.constEnd(); ++it) {
        it.value()->setText(KeyUtils::keyToString(it.key(), notation));
        // QLineEdits are only enabled for Custom notation.
        it.value()->setEnabled(notation == KeyUtils::KeyNotation::Custom);
    }
    slotUpdate();
}

void DlgPrefKey::setNotationCustom(bool active) {
    if (active) {
        setNotation(KeyUtils::KeyNotation::Custom);
    }
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
