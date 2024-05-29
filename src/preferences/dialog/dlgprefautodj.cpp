#include "preferences/dialog/dlgprefautodj.h"

#include "moc_dlgprefautodj.cpp"

DlgPrefAutoDJ::DlgPrefAutoDJ(QWidget* pParent,
                             UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    // The auto-DJ replay-age for randomly-selected tracks
    connect(RequeueIgnoreCheckBox,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefAutoDJ::slotToggleRequeueIgnore);

    // Auto DJ random enqueue
    connect(RandomQueueCheckBox,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefAutoDJ::slotToggleRandomQueue);

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
}

void DlgPrefAutoDJ::slotApply() {
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "MinimumAvailable"),
            MinimumAvailableSpinBox->value());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTime"),
            RequeueIgnoreCheckBox->isChecked());
    const QString ignTimeStr =
            RequeueIgnoreTimeEdit->time().toString(RequeueIgnoreTimeEdit->displayFormat());
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "IgnoreTime"), ignTimeStr);

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
            RandomQueueCheckBox->isChecked());
    m_pConfig->setValue(
            ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),
            RandomQueueMinimumSpinBox->value());

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "center_xfader_when_disabling"),
            CenterXfaderCheckBox->isChecked());
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    MinimumAvailableSpinBox->setValue(20);

    RequeueIgnoreCheckBox->setChecked(false);
    RequeueIgnoreTimeEdit->setEnabled(false);
    RequeueIgnoreTimeEdit->setTime(QTime::fromString(
            "23:59", RequeueIgnoreTimeEdit->displayFormat()));

    RandomQueueCheckBox->setChecked(false);
    RandomQueueCheckBox->setEnabled(true);
    RandomQueueMinimumSpinBox->setEnabled(false);
    RandomQueueMinimumSpinBox->setValue(5);

    CenterXfaderCheckBox->setChecked(false);
}

void DlgPrefAutoDJ::slotToggleRequeueIgnore(int buttonState) {
    RequeueIgnoreTimeEdit->setEnabled(buttonState == Qt::Checked);
}

void DlgPrefAutoDJ::considerRepeatPlaylistState(bool enable) {
    RandomQueueMinimumSpinBox->setEnabled(enable);
}

void DlgPrefAutoDJ::slotToggleRandomQueue(int buttonState) {
    RandomQueueMinimumSpinBox->setEnabled(buttonState == Qt::Checked);
}
