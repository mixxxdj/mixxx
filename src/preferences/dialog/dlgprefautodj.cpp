#include "preferences/dialog/dlgprefautodj.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#include <QTimeZone>
#endif

#include "moc_dlgprefautodj.cpp"

DlgPrefAutoDJ::DlgPrefAutoDJ(QWidget* pParent,
                             UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    // The minimum available for randomly-selected tracks
    MinimumAvailableSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "MinimumAvailable"), 20));
    connect(MinimumAvailableSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefAutoDJ::slotSetMinimumAvailable);

    // The auto-DJ replay-age for randomly-selected tracks
    RequeueIgnoreCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "UseIgnoreTime"), false));
    connect(RequeueIgnoreCheckBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefAutoDJ::slotToggleRequeueIgnore);
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
    connect(RequeueIgnoreTimeEdit,
            &QTimeEdit::timeChanged,
            this,
            &DlgPrefAutoDJ::slotSetRequeueIgnoreTime);

    // Auto DJ random enqueue
    RandomQueueCheckBox->setChecked(m_pConfig->getValue(
            ConfigKey("[Auto DJ]", "EnableRandomQueue"), false));
    // 5-arbitrary
    RandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    // "[Auto DJ], Requeue" is set by 'Repeat Playlist' toggle in DlgAutoDj GUI.
    // If it's checked un-check 'Random Queue'
    slotConsiderRepeatPlaylistState(
            m_pConfig->getValue<bool>(ConfigKey("[Auto DJ]", "Requeue")));
    slotToggleRandomQueue(
            m_pConfig->getValue<bool>(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"))
                    ? Qt::Checked
                    : Qt::Unchecked);
    // Be ready to enable and modify the minimum number and un/check the checkbox
    connect(RandomQueueCheckBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefAutoDJ::slotToggleRandomQueue);
    connect(RandomQueueMinimumSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefAutoDJ::slotSetRandomQueueMin);

    setScrollSafeGuardForAllInputWidgets(this);
}

void DlgPrefAutoDJ::slotUpdate() {
}

void DlgPrefAutoDJ::slotApply() {
    //Copy from Buffer to actual values
    m_pConfig->setValue(ConfigKey("[Auto DJ]","MinimumAvailable"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "MinimumAvailableBuff"), 20));

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "IgnoreTime"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "IgnoreTimeBuff"), "23:59"));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTime"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), false));

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"), 5));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"), false));
}

void DlgPrefAutoDJ::slotCancel() {
    // Load actual values and reset Buffer Values where ever needed
    MinimumAvailableSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "MinimumAvailable"), 20));

    RequeueIgnoreTimeEdit->setTime(
            QTime::fromString(
                    m_pConfig->getValue(
                            ConfigKey("[Auto DJ]", "IgnoreTime"), "23:59"),
                    RequeueIgnoreTimeEdit->displayFormat()));
    RequeueIgnoreCheckBox->setChecked(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), false));
    RequeueIgnoreTimeEdit->setEnabled(
            RequeueIgnoreCheckBox->checkState() == Qt::Checked);
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), false));

    RandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    RandomQueueCheckBox->setChecked(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), false));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), false));
    slotToggleRandomQueue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), false)
                    ? Qt::Checked
                    : Qt::Unchecked);
    slotToggleRandomQueue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "Requeue"), false)
                    ? Qt::Checked
                    : Qt::Unchecked);
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    // Re-queue tracks in AutoDJ
    MinimumAvailableSpinBox->setValue(20);

    RequeueIgnoreTimeEdit->setTime(QTime::fromString(
            "23:59", RequeueIgnoreTimeEdit->displayFormat()));
    RequeueIgnoreCheckBox->setChecked(false);
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), false);
    RequeueIgnoreTimeEdit->setEnabled(false);

    RandomQueueMinimumSpinBox->setValue(5);
    RandomQueueCheckBox->setChecked(false);
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"), false);
    RandomQueueMinimumSpinBox->setEnabled(false);
    RandomQueueCheckBox->setEnabled(true);
}

void DlgPrefAutoDJ::slotSetMinimumAvailable(int a_iValue) {
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "MinimumAvailableBuff"), a_iValue);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefAutoDJ::slotToggleRequeueIgnore(Qt::CheckState buttonState) {
#else
void DlgPrefAutoDJ::slotToggleRequeueIgnore(int buttonState) {
#endif
    bool checked = buttonState == Qt::Checked;
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), checked);
    RequeueIgnoreTimeEdit->setEnabled(checked);
}

void DlgPrefAutoDJ::slotSetRequeueIgnoreTime(const QTime& a_rTime) {
    QString str = a_rTime.toString(RequeueIgnoreTimeEdit->displayFormat());
    m_pConfig->set(ConfigKey("[Auto DJ]", "IgnoreTimeBuff"), str);
}

void DlgPrefAutoDJ::slotSetRandomQueueMin(int a_iValue) {
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"), a_iValue);
}

void DlgPrefAutoDJ::slotConsiderRepeatPlaylistState(bool enable) {
    if (enable) {
        // Requeue is enabled
        RandomQueueCheckBox->setChecked(false);
        // ToDo(ronso0): Redundant? If programmatic checkbox change is signaled
        // to slotToggleRandomQueue
        RandomQueueMinimumSpinBox->setEnabled(false);
        m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                false);
    } else {
        RandomQueueMinimumSpinBox->setEnabled(
                m_pConfig->getValue(
                        ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"), false));
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefAutoDJ::slotToggleRandomQueue(Qt::CheckState buttonState) {
#else
void DlgPrefAutoDJ::slotToggleRandomQueue(int buttonState) {
#endif
    bool enable = buttonState == Qt::Checked;
    // Toggle the option to select minimum tracks
    RandomQueueMinimumSpinBox->setEnabled(enable);
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
            enable);
}
