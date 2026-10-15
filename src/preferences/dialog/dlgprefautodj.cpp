#include "preferences/dialog/dlgprefautodj.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#include <QTimeZone>
#endif

#include "library/autodj/harmonicmixing.h"
#include "moc_dlgprefautodj.cpp"

DlgPrefAutoDJ::DlgPrefAutoDJ(QWidget* pParent,
                             UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    // The auto-DJ replay-age for randomly-selected tracks
    connect(RequeueIgnoreCheckBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefAutoDJ::slotToggleRequeueIgnore);

    // Auto DJ random enqueue
    connect(RandomQueueCheckBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefAutoDJ::slotToggleRandomQueue);

    // Auto DJ harmonic mixing
    connect(HarmonicMixingCheckBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefAutoDJ::slotToggleHarmonicMixing);

    setScrollSafeGuardForAllInputWidgets(this);
}

void DlgPrefAutoDJ::slotUpdate() {
    // The minimum available for randomly-selected tracks
    MinimumAvailableSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "MinimumAvailable"), 20));

    // The auto-DJ replay-age for randomly-selected tracks
    RequeueIgnoreCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "UseIgnoreTime"), false));
    /// TODO: Once we require at least Qt 6.7, remove this `setTimeZone` call
    /// and uncomment the corresponding declarations in the UI file instead.
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    RequeueIgnoreTimeEdit->setTimeZone(QTimeZone::LocalTime);
#else
    RequeueIgnoreTimeEdit->setTimeSpec(Qt::LocalTime);
#endif
    RequeueIgnoreTimeEdit->setTime(
            QTime::fromString(
                    m_pConfig->getValue(
                            ConfigKey("[Auto DJ]", "IgnoreTime"), "23:59"),
                    RequeueIgnoreTimeEdit->displayFormat()));
    RequeueIgnoreTimeEdit->setEnabled(
            RequeueIgnoreCheckBox->checkState() == Qt::Checked);

    // Auto DJ random enqueue
    RandomQueueCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "EnableRandomQueue"), false));

    RandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    // "[Auto DJ], Requeue" is set by 'Repeat Playlist' toggle in DlgAutoDj GUI.
    // If it's checked un-check 'Random Queue'
    // TODO Add 'Repeat' checkbox here, or add a hint why the checkbox may be disabled
    considerRepeatPlaylistState(
            m_pConfig->getValue<bool>(ConfigKey("[Auto DJ]", "Requeue")));
    slotToggleRandomQueue(
            m_pConfig->getValue<bool>(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"))
                    ? Qt::Checked
                    : Qt::Unchecked);

    // Re-center the crossfader instantly when AutoDJ is disabled
    CenterXfaderCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "center_xfader_when_disabling"), false));

    // Harmonic mixing
    const bool harmonicMixing = m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "HarmonicMixing"), false);
    HarmonicMixingCheckBox->setChecked(harmonicMixing);
    HarmonicBpmToleranceSpinBox->setValue(m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "HarmonicBpmTolerance"),
            static_cast<int>(mixxx::HarmonicMixing::kDefaultBpmTolerancePercent)));
    HarmonicBpmToleranceSpinBox->setEnabled(harmonicMixing);
}

void DlgPrefAutoDJ::slotApply() {
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "MinimumAvailable"),
            MinimumAvailableSpinBox->value());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTime"),
            RequeueIgnoreCheckBox->isChecked());
    const QString ignTimeStr =
            RequeueIgnoreTimeEdit->time().toString();
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "IgnoreTime"), ignTimeStr);

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
            RandomQueueCheckBox->isChecked());
    m_pConfig->setValue(
            ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),
            RandomQueueMinimumSpinBox->value());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "center_xfader_when_disabling"),
            CenterXfaderCheckBox->isChecked());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "HarmonicMixing"),
            HarmonicMixingCheckBox->isChecked());
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "HarmonicBpmTolerance"),
            HarmonicBpmToleranceSpinBox->value());
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    MinimumAvailableSpinBox->setValue(20);

    RequeueIgnoreCheckBox->setChecked(false);
    RequeueIgnoreTimeEdit->setEnabled(false);
    RequeueIgnoreTimeEdit->setTime(QTime::fromString("23:59"));

    RandomQueueCheckBox->setChecked(false);
    RandomQueueCheckBox->setEnabled(true);
    RandomQueueMinimumSpinBox->setEnabled(false);
    RandomQueueMinimumSpinBox->setValue(5);

    CenterXfaderCheckBox->setChecked(false);

    HarmonicMixingCheckBox->setChecked(false);
    HarmonicBpmToleranceSpinBox->setValue(
            static_cast<int>(mixxx::HarmonicMixing::kDefaultBpmTolerancePercent));
    HarmonicBpmToleranceSpinBox->setEnabled(false);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefAutoDJ::slotToggleRequeueIgnore(Qt::CheckState buttonState) {
#else
void DlgPrefAutoDJ::slotToggleRequeueIgnore(int buttonState) {
#endif
    RequeueIgnoreTimeEdit->setEnabled(buttonState == Qt::Checked);
}

void DlgPrefAutoDJ::considerRepeatPlaylistState(bool enable) {
    RandomQueueMinimumSpinBox->setEnabled(enable);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefAutoDJ::slotToggleRandomQueue(Qt::CheckState buttonState) {
#else
void DlgPrefAutoDJ::slotToggleRandomQueue(int buttonState) {
#endif
    RandomQueueMinimumSpinBox->setEnabled(buttonState == Qt::Checked);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefAutoDJ::slotToggleHarmonicMixing(Qt::CheckState buttonState) {
#else
void DlgPrefAutoDJ::slotToggleHarmonicMixing(int buttonState) {
#endif
    HarmonicBpmToleranceSpinBox->setEnabled(buttonState == Qt::Checked);
}
