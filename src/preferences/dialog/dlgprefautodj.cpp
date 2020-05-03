#include "preferences/dialog/dlgprefautodj.h"

DlgPrefAutoDJ::DlgPrefAutoDJ(QWidget* pParent,
                             UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    // The minimum available for randomly-selected tracks
    autoDjMinimumAvailableSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "MinimumAvailable"), 20));
    connect(autoDjMinimumAvailableSpinBox, SIGNAL(valueChanged(int)), this,
            SLOT(slotSetAutoDjMinimumAvailable(int)));

    // The auto-DJ replay-age for randomly-selected tracks
    autoDjIgnoreTimeCheckBox->setChecked(
            (bool) m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), 0));
    connect(autoDjIgnoreTimeCheckBox, SIGNAL(stateChanged(int)), this,
            SLOT(slotSetAutoDjUseIgnoreTime(int)));
    autoDjIgnoreTimeEdit->setTime(
            QTime::fromString(
                    m_pConfig->getValue(
                            ConfigKey("[Auto DJ]", "IgnoreTime"), "23:59"),
                    autoDjIgnoreTimeEdit->displayFormat()));
    autoDjIgnoreTimeEdit->setEnabled(
            autoDjIgnoreTimeCheckBox->checkState() == Qt::Checked);
    connect(autoDjIgnoreTimeEdit, SIGNAL(timeChanged(const QTime &)), this,
            SLOT(slotSetAutoDjIgnoreTime(const QTime &)));

    // Auto DJ random enqueue
    ComboBoxAutoDjRandomQueue->addItem(tr("Off"));
    ComboBoxAutoDjRandomQueue->addItem(tr("On"));
    ComboBoxAutoDjRandomQueue->setCurrentIndex(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    // 5-arbitrary
    autoDJRandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    slotEnableAutoDJRandomQueueComboBox(
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue")).toInt());
    slotEnableAutoDJRandomQueue(
            m_pConfig->getValue<int>(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue")));
    // Be ready to enable and modify the minimum number and enable disable the spinbox
    connect(ComboBoxAutoDjRandomQueue, SIGNAL(activated(int)), this,
            SLOT(slotEnableAutoDJRandomQueue(int)));
    connect(autoDJRandomQueueMinimumSpinBox, SIGNAL(valueChanged(int)), this,
            SLOT(slotSetAutoDJRandomQueueMin(int)));

    ComboBoxAutoDjTransitionUnit->addItem(tr("Seconds"));
    ComboBoxAutoDjTransitionUnit->addItem(tr("Beats"));
    ComboBoxAutoDjTransitionUnit->setCurrentIndex(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "TransitionUnit"), 0));
    connect(ComboBoxAutoDjTransitionUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetAutoDJTransitionUnit(int)));
    connect(ComboBoxAutoDjTransitionUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEnableAutoDJBPMtoUseComboBox(int)));

    ComboBoxAutoDjBPMToUse->addItem(tr("Before track's end"));
    ComboBoxAutoDjBPMToUse->addItem(tr("From next track's start"));
    QString bpmToUseToolTip = tr(
            "If the tracks do not have the same BPM, this specifies\n"
            "before/from which point to calculate the transition period.");
    ComboBoxAutoDjBPMToUse->setToolTip(bpmToUseToolTip);
    ComboBoxAutoDjBPMToUse->setCurrentIndex(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "BPMToUse"), 0));
    connect(ComboBoxAutoDjBPMToUse, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetAutoDJBPMtoUse(int)));
    slotEnableAutoDJBPMtoUseComboBox(
            m_pConfig->getValue(ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUse"), 0));
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

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "TransitionUnit"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "TransitionUnitBuff"), 0));

    m_pConfig->setValue(ConfigKey("[Auto DJ]", "BPMToUse"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "BPMToUseBuff"), 0));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUse"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUseBuff"), 0));
}

void DlgPrefAutoDJ::slotCancel() {
    // Load actual values and reset Buffer Values where ever needed
    autoDjMinimumAvailableSpinBox->setValue(
            m_pConfig->getValue(
                      ConfigKey("[Auto DJ]", "MinimumAvailable"), 20));

    autoDjIgnoreTimeEdit->setTime(
            QTime::fromString(
            m_pConfig->getValue(
                      ConfigKey("[Auto DJ]", "IgnoreTime"), "23:59"),
                                autoDjIgnoreTimeEdit->displayFormat()));
    autoDjIgnoreTimeCheckBox->setChecked(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), false));
    autoDjIgnoreTimeEdit->setEnabled(
            autoDjIgnoreTimeCheckBox->checkState() == Qt::Checked);
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), 0));

    autoDJRandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), 5));
    ComboBoxAutoDjRandomQueue->setCurrentIndex(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    slotEnableAutoDJRandomQueue(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), 0));
    slotEnableAutoDJRandomQueueComboBox(
            m_pConfig->getValue<int>(ConfigKey("[Auto DJ]", "Requeue")));

    ComboBoxAutoDjTransitionUnit->setCurrentIndex(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "TransitionUnit"), 0));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "TransitionUnitBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "TransitionUnit"), 0));

    ComboBoxAutoDjBPMToUse->setCurrentIndex(
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "BPMToUseBuff"), 0));
    m_pConfig->setValue(ConfigKey("[Auto DJ]", "BPMToUseBuff"),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "BPMToUse"), 0));
    slotEnableAutoDJBPMtoUseComboBox(
            m_pConfig->getValue(ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUse"), 0));
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    // Re-queue tracks in AutoDJ
    autoDjMinimumAvailableSpinBox->setValue(20);

    autoDjIgnoreTimeEdit->setTime(QTime::fromString(
            "23:59", autoDjIgnoreTimeEdit->displayFormat()));
    autoDjIgnoreTimeCheckBox->setChecked(false);
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"),QString("0"));
    autoDjIgnoreTimeEdit->setEnabled(false);

    autoDJRandomQueueMinimumSpinBox->setValue(5);
    ComboBoxAutoDjRandomQueue->setCurrentIndex(0);
    m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),QString("0"));
    autoDJRandomQueueMinimumSpinBox->setEnabled(false);
    ComboBoxAutoDjRandomQueue->setEnabled(true);

    ComboBoxAutoDjTransitionUnit->setCurrentIndex(0);
    m_pConfig->set(ConfigKey("[Auto DJ]", "TransitionUnitBuff"), ConfigValue(0));
    ComboBoxAutoDjBPMToUse->setCurrentIndex(0);
    m_pConfig->set(ConfigKey("[Auto DJ]", "BPMToUseBuff"), ConfigValue(0));
    m_pConfig->set(ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUse"), ConfigValue(0));
    ComboBoxAutoDjBPMToUse->setEnabled(false);
}

void DlgPrefAutoDJ::slotSetAutoDjMinimumAvailable(int a_iValue) {
    QString str;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]","MinimumAvailableBuff"),str);
}

void DlgPrefAutoDJ::slotSetAutoDjUseIgnoreTime(int a_iState) {
    bool bChecked = (a_iState == Qt::Checked);
    QString strChecked = (bChecked) ? "1" : "0";
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), strChecked);
    autoDjIgnoreTimeEdit->setEnabled(bChecked);
}

void DlgPrefAutoDJ::slotSetAutoDjIgnoreTime(const QTime &a_rTime) {
    QString str = a_rTime.toString(autoDjIgnoreTimeEdit->displayFormat());
    m_pConfig->set(ConfigKey("[Auto DJ]", "IgnoreTimeBuff"),str);
}

void DlgPrefAutoDJ::slotSetAutoDJRandomQueueMin(int a_iValue) {
    QString str;
    //qDebug() << "min allowed " << a_iValue;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"), str);
}

void DlgPrefAutoDJ::slotEnableAutoDJRandomQueueComboBox(int a_iValue) {
    if (a_iValue == 1) {
        // Requeue is enabled
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                ConfigValue(0));
        ComboBoxAutoDjRandomQueue->setCurrentIndex(0);
        ComboBoxAutoDjRandomQueue->setEnabled(false);
        autoDJRandomQueueMinimumSpinBox->setEnabled(false);
    } else {
        ComboBoxAutoDjRandomQueue->setEnabled(true);
        autoDJRandomQueueMinimumSpinBox->setEnabled(
                m_pConfig->getValue(
                        ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"), false));
    }
}

void DlgPrefAutoDJ::slotEnableAutoDJRandomQueue(int a_iValue) {
    // Disable enable the option to select minimum tracks
    if (a_iValue == 0) {
        autoDJRandomQueueMinimumSpinBox->setEnabled(false);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                ConfigValue(0));
    } else {
        autoDJRandomQueueMinimumSpinBox->setEnabled(true);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
                ConfigValue(1));
    }
}

void DlgPrefAutoDJ::slotSetAutoDJTransitionUnit(int a_iValue) {
    if (a_iValue == 0) {
        // Use seconds
        m_pConfig->set(ConfigKey("[Auto DJ]", "TransitionUnitBuff"),
                ConfigValue(0));
    } else {
        // Use beats
        m_pConfig->set(ConfigKey("[Auto DJ]", "TransitionUnitBuff"),
                ConfigValue(1));
    }
}

void DlgPrefAutoDJ::slotEnableAutoDJBPMtoUseComboBox(int a_iValue) {
    if (a_iValue == 0) {
        // Using seconds, disable the combo box
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUseBuff"),
                ConfigValue(0));
        ComboBoxAutoDjBPMToUse->setEnabled(false);
    } else {
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableAutoDJBPMToUseBuff"),
                ConfigValue(1));
        ComboBoxAutoDjBPMToUse->setEnabled(true);
    }
}

void DlgPrefAutoDJ::slotSetAutoDJBPMtoUse(int a_iValue) {
    if (a_iValue == 0) {
        // Before track's end
        m_pConfig->setValue(ConfigKey("[Auto DJ]", "BPMToUseBuff"), 0);
    } else {
        // From next track's start
        m_pConfig->setValue(ConfigKey("[Auto DJ]", "BPMToUseBuff"), 1);
    }
}
