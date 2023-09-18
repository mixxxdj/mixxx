#include "preferences/dialog/dlgprefrhythm.h"

#include "control/controlobject.h"
#include "defs_urls.h"

DlgPrefRhythm::DlgPrefRhythm(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent) {
    setupUi(this);

    loadSettings();

    // TODO(Cristiano) Create remaining connections..
    connect(bBeatsAndTempoEnabled, SIGNAL(stateChanged(int)), this, SLOT(analyzerEnabled(int)));
    connect(txtMinBpm, SIGNAL(valueChanged(int)), this, SLOT(minBpmRangeChanged(int)));
    connect(txtMaxBpm, SIGNAL(valueChanged(int)), this, SLOT(maxBpmRangeChanged(int)));
}

DlgPrefRhythm::~DlgPrefRhythm() {
}

QUrl DlgPrefRhythm::helpUrl() const {
    return QUrl(MIXXX_MANUAL_BEATS_URL);
}

void DlgPrefRhythm::loadSettings() {
    slotUpdate();
}

void DlgPrefRhythm::slotResetToDefaults() {
    slotUpdate();
}

void DlgPrefRhythm::analyzerEnabled(int i) {
    m_banalyzerEnabled = static_cast<bool>(i);
    slotUpdate();
}

void DlgPrefRhythm::minBpmRangeChanged(int value) {
    m_minBpm = value;
    slotUpdate();
}

void DlgPrefRhythm::maxBpmRangeChanged(int value) {
    m_maxBpm = value;
    slotUpdate();
}

void DlgPrefRhythm::slotUpdate() {
    // TODO (Cristiano) Save and use these preferences
    ;
}

void DlgPrefRhythm::slotApply() {
    ;
}
