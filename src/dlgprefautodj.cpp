#include "dlgprefautodj.h"

DlgPrefAutoDJ::DlgPrefAutoDJ(QWidget* pParent,
                             ConfigObject<ConfigValue>* pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    // Re-queue tracks in Auto DJ
    ComboBoxAutoDjRequeue->addItem(tr("Off"));
    ComboBoxAutoDjRequeue->addItem(tr("On"));
    ComboBoxAutoDjRequeue->setCurrentIndex(m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue")).toInt());
    connect(ComboBoxAutoDjRequeue, SIGNAL(activated(int)),
            this, SLOT(slotSetAutoDjRequeue(int)));

#ifdef __AUTODJCRATES__

    // The minimum available for randomly-selected tracks
    autoDjMinimumAvailableSpinBox->setValue(
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "MinimumAvailable"), "20").toInt());
    connect(autoDjMinimumAvailableSpinBox, SIGNAL(valueChanged(int)), this,
            SLOT(slotSetAutoDjMinimumAvailable(int)));

    // The auto-DJ replay-age for randomly-selected tracks
    autoDjIgnoreTimeCheckBox->setChecked(
            (bool) m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), "0").toInt());
    connect(autoDjIgnoreTimeCheckBox, SIGNAL(stateChanged(int)), this,
            SLOT(slotSetAutoDjUseIgnoreTime(int)));
    autoDjIgnoreTimeEdit->setTime(
            QTime::fromString(
                    m_pConfig->getValueString(
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
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"),"0").toInt());
    // 5-arbitrary
    autoDJRandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),"5").toInt());
    slotEnableAutoDJRandomQueueComboBox(
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue")).toInt());
    slotEnableAutoDJRandomQueue(
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt());
    // Be ready to enable disable the random enque as reque is modified
    connect(ComboBoxAutoDjRequeue, SIGNAL(activated(int)), this,
            SLOT(slotEnableAutoDJRandomQueueComboBox(int)));
    // Be ready to enable and modify the minimum number and enable disable the spinbox
    connect(ComboBoxAutoDjRandomQueue, SIGNAL(activated(int)), this,
            SLOT(slotEnableAutoDJRandomQueue(int)));
    connect(autoDJRandomQueueMinimumSpinBox, SIGNAL(valueChanged(int)), this,
            SLOT(slotSetAutoDJRandomQueueMin(int)));

#else // __AUTODJCRATES__

    // Remove the preferences.
    autoDjMinimumAvailableLabel->setVisible(false);
    GridLayout1->removeWidget(autoDjMinimumAvailableLabel);
    autoDjMinimumAvailableSpinBox->setVisible(false);
    GridLayout1->removeWidget(autoDjMinimumAvailableSpinBox);
    autoDjIgnoreTimeCheckBox->setVisible(false);
    GridLayout1->removeWidget(autoDjIgnoreTimeCheckBox);
    autoDjIgnoreTimeEdit->setVisible(false);
    GridLayout1->removeWidget(autoDjIgnoreTimeEdit);
#endif // __AUTODJCRATES__
    // Connect the global cancell signal
       connect(this,SIGNAL(cancelPreferences()),this,
               SLOT(slotCancel()));
}

DlgPrefAutoDJ::~DlgPrefAutoDJ() {
}

void DlgPrefAutoDJ::slotUpdate() {
}

void DlgPrefAutoDJ::slotApply() {
    //Copy from Buffer to actual values
    m_pConfig->set(ConfigKey("[Auto DJ]", "Requeue"),
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "RequeueBuff"),"0"));
#ifdef __AUTODJCRATES__
    m_pConfig->set(ConfigKey("[Auto DJ]","MinimumAvailable"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "MinimumAvailableBuff"), "20"));

    m_pConfig->set(ConfigKey("[Auto DJ]", "IgnoreTime"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "IgnoreTimeBuff"), "23:59"));
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTime"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), "0"));

    m_pConfig->set(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"),"5"));
    m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),"0"));
#endif //__AUTODJCRATES__
}

void DlgPrefAutoDJ::slotCancel() {
    // Load actual values and reset Buffer Values where ever needed
    ComboBoxAutoDjRequeue->setCurrentIndex(
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue"),"0").toInt());
    m_pConfig->set(ConfigKey("[Auto DJ]", "RequeueBuff"),
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue"),"0"));
#ifdef __AUTODJCRATES__
    autoDjMinimumAvailableSpinBox->setValue(
            m_pConfig->getValueString(
                      ConfigKey("[Auto DJ]", "MinimumAvailable"), "20").toInt());

    autoDjIgnoreTimeEdit->setTime(
            QTime::fromString(
            m_pConfig->getValueString(
                      ConfigKey("[Auto DJ]", "IgnoreTime"), "23:59"),
                                autoDjIgnoreTimeEdit->displayFormat()));
    autoDjIgnoreTimeCheckBox->setChecked(
            (bool) m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), "0").toInt());
    autoDjIgnoreTimeEdit->setEnabled(
            autoDjIgnoreTimeCheckBox->checkState() == Qt::Checked);
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "UseIgnoreTime"), "0"));

    autoDJRandomQueueMinimumSpinBox->setValue(
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"),"5").toInt());
    ComboBoxAutoDjRandomQueue->setCurrentIndex(
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"),"0").toInt());
    m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue"), "0"));
    slotEnableAutoDJRandomQueue(
            m_pConfig->getValueString(
                    ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt());
    slotEnableAutoDJRandomQueueComboBox(
            m_pConfig->getValueString(ConfigKey("[Auto DJ]", "Requeue")).toInt());
#endif //__AUTODJCRATES__
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    // Re-queue tracks in AutoDJ
    ComboBoxAutoDjRequeue->setCurrentIndex(0);
    m_pConfig->set(ConfigKey("[Auto DJ]", "RequeueBuff"),ConfigValue(0));
#ifdef __AUTODJCRATES__
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
#endif
}

void DlgPrefAutoDJ::slotSetAutoDjRequeue(int) {
    m_pConfig->set(ConfigKey("[Auto DJ]", "RequeueBuff"),
            ConfigValue(ComboBoxAutoDjRequeue->currentIndex()));
}

void DlgPrefAutoDJ::slotSetAutoDjMinimumAvailable(int a_iValue) {
#ifdef __AUTODJCRATES__
    QString str;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]","MinimumAvailableBuff"),str);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotSetAutoDjUseIgnoreTime(int a_iState) {
#ifdef __AUTODJCRATES__
    bool bChecked = (a_iState == Qt::Checked);
    QString strChecked = (bChecked) ? "1" : "0";
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTimeBuff"), strChecked);
    autoDjIgnoreTimeEdit->setEnabled(bChecked);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotSetAutoDjIgnoreTime(const QTime &a_rTime) {
#ifdef __AUTODJCRATES__
    QString str = a_rTime.toString(autoDjIgnoreTimeEdit->displayFormat());
    m_pConfig->set(ConfigKey("[Auto DJ]", "IgnoreTimeBuff"),str);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotSetAutoDJRandomQueueMin(int a_iValue) {
#ifdef __AUTODJCRATES__
    QString str;
    //qDebug() << "min allowed " << a_iValue;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowedBuff"), str);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotEnableAutoDJRandomQueueComboBox(int a_iValue) {
#ifdef __AUTODJCRATES__
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
                m_pConfig->getValueString(
                        ConfigKey("[Auto DJ]", "EnableRandomQueueBuff"),"0").toInt());
    }
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotEnableAutoDJRandomQueue(int a_iValue) {
#ifdef __AUTODJCRATES__
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
#endif // __AUTODJCRATES__
}

