#include "preferences/dialog/dlgprefautodj.h"

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
    RequeueIgnoreCheckBox->setChecked(
            (bool)m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), 0));
    connect(RequeueIgnoreCheckBox,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefAutoDJ::slotToggleRequeueIgnore);
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
    RandomQueueCheckBox->setChecked(
            (bool)m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    // 5-arbitrary
    RandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    // "[Auto DJ], Requeue" is set by 'Repeat Playlist' toggle in DlgAutoDj GUI.
    // If it's checked un-check 'Random Queue'
    slotConsiderRepeatPlaylistState(
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue")).toInt());
    slotToggleRandomQueue(
            m_pConfig->getValue<int>(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue")));
    // Be ready to enable and modify the minimum number and un/check the checkbox
    connect(RandomQueueCheckBox,
            &QCheckBox::stateChanged,
            this,
            &DlgPrefAutoDJ::slotToggleRandomQueue);
    connect(RandomQueueMinimumSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefAutoDJ::slotSetRandomQueueMin);
}

DlgPrefAutoDJ::~DlgPrefAutoDJ() {
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
                    ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), "0"));

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"), 5));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"), 0));
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
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), 0));

    RandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    RandomQueueCheckBox->setChecked(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), false));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    slotToggleRandomQueue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    slotToggleRandomQueue(
            m_pConfig->getValue<int>(ConfigKey("[Auto DJ]", "Requeue")));
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    // Re-queue tracks in AutoDJ
    MinimumAvailableSpinBox->setValue(20);

    RequeueIgnoreTimeEdit->setTime(QTime::fromString(
            "23:59", RequeueIgnoreTimeEdit->displayFormat()));
    RequeueIgnoreCheckBox->setChecked(false);
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"),QString("0"));
    RequeueIgnoreTimeEdit->setEnabled(false);

    RandomQueueMinimumSpinBox->setValue(5);
    RandomQueueCheckBox->setChecked(false);
    m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),QString("0"));
    RandomQueueMinimumSpinBox->setEnabled(false);
    RandomQueueCheckBox->setEnabled(true);
}

void DlgPrefAutoDJ::slotSetMinimumAvailable(int a_iValue) {
    QString str;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]","MinimumAvailableBuff"),str);
}

void DlgPrefAutoDJ::slotToggleRequeueIgnore(int a_iState) {
    bool bChecked = (a_iState == Qt::Checked);
    QString strChecked = (bChecked) ? "1" : "0";
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), strChecked);
    RequeueIgnoreTimeEdit->setEnabled(bChecked);
}

void DlgPrefAutoDJ::slotSetRequeueIgnoreTime(const QTime& a_rTime) {
    QString str = a_rTime.toString(RequeueIgnoreTimeEdit->displayFormat());
    m_pConfig->set(ConfigKey("[Auto DJ]", "IgnoreTimeBuff"),str);
}

void DlgPrefAutoDJ::slotSetRandomQueueMin(int a_iValue) {
    QString str;
    //qDebug() << "min allowed " << a_iValue;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"), str);
}

void DlgPrefAutoDJ::slotConsiderRepeatPlaylistState(int a_iValue) {
    if (a_iValue == 1) {
        // Requeue is enabled
        RandomQueueCheckBox->setChecked(false);
        // ToDo(ronso0): Redundant? If programmatic checkbox change is signaled
        // to slotToggleRandomQueue
        RandomQueueMinimumSpinBox->setEnabled(false);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                ConfigValue(0));
    } else {
        RandomQueueMinimumSpinBox->setEnabled(
                m_pConfig->getValue(
                        ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"), false));
    }
}

void DlgPrefAutoDJ::slotToggleRandomQueue(int a_iValue) {
    // Toggle the option to select minimum tracks
    if (a_iValue == 0) {
        RandomQueueMinimumSpinBox->setEnabled(false);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                ConfigValue(0));
    } else {
        RandomQueueMinimumSpinBox->setEnabled(true);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                ConfigValue(1));
    }
}
