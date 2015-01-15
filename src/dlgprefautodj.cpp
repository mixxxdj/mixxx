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
    // Assign a minimum value here with some logic , 5-arbitrary
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

}

DlgPrefAutoDJ::~DlgPrefAutoDJ() {
}

void DlgPrefAutoDJ::slotUpdate() {
}

void DlgPrefAutoDJ::slotApply() {
}

void DlgPrefAutoDJ::slotResetToDefaults() {
    // Re-queue tracks in AutoDJ
    ComboBoxAutoDjRequeue->setCurrentIndex(0);
#ifdef __AUTODJCRATES__
    // **delete** Should't the config objects values be updated
    autoDjMinimumAvailableSpinBox->setValue(20);
    autoDjIgnoreTimeEdit->setTime(QTime::fromString(
            "23:59", autoDjIgnoreTimeEdit->displayFormat()));
    autoDjIgnoreTimeCheckBox->setChecked(false);
    autoDJRandomQueueMinimumSpinBox->setValue(5);
    ComboBoxAutoDjRandomQueue->setCurrentIndex(0);
    autoDJRandomQueueMinimumSpinBox->setEnabled(false);
    ComboBoxAutoDjRandomQueue->setEnabled(true);

#endif
}

void DlgPrefAutoDJ::slotSetAutoDjRequeue(int) {
    m_pConfig->set(ConfigKey("[Auto DJ]", "Requeue"), ConfigValue(ComboBoxAutoDjRequeue->currentIndex()));
}

void DlgPrefAutoDJ::slotSetAutoDjMinimumAvailable(int a_iValue) {
#ifdef __AUTODJCRATES__
    QString str;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]","MinimumAvailable"),str);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotSetAutoDjUseIgnoreTime(int a_iState) {
#ifdef __AUTODJCRATES__
    bool bChecked = (a_iState == Qt::Checked);
    QString strChecked = (bChecked) ? "1" : "0";
    m_pConfig->set(ConfigKey("[Auto DJ]", "UseIgnoreTime"), strChecked);
    autoDjIgnoreTimeEdit->setEnabled(bChecked);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotSetAutoDjIgnoreTime(const QTime &a_rTime) {
#ifdef __AUTODJCRATES__
    QString str = a_rTime.toString(autoDjIgnoreTimeEdit->displayFormat());
    m_pConfig->set(ConfigKey("[Auto DJ]", "IgnoreTime"),str);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotSetAutoDJRandomQueueMin(int a_iValue) {
#ifdef __AUTODJCRATES__
    QString str;
    //qDebug() << "min allowed " << a_iValue;
    str.setNum(a_iValue);
    m_pConfig->set(ConfigKey("[Auto DJ]", "RandomQueueMinimumAllowed"), str);
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotEnableAutoDJRandomQueueComboBox(int a_iValue) {
#ifdef __AUTODJCRATES__
    if (a_iValue == 1) {
        // Requeue is enabled
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
                ConfigValue(0));
        ComboBoxAutoDjRandomQueue->setCurrentIndex(
                m_pConfig->getValueString(
                        ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt());
        ComboBoxAutoDjRandomQueue->setEnabled(false);
        autoDJRandomQueueMinimumSpinBox->setEnabled(false);
    } else {
        ComboBoxAutoDjRandomQueue->setEnabled(true);
        autoDJRandomQueueMinimumSpinBox->setEnabled(false);
    }
#endif // __AUTODJCRATES__
}

void DlgPrefAutoDJ::slotEnableAutoDJRandomQueue(int a_iValue) {
#ifdef __AUTODJCRATES__
    // Disable enable the option to select minimum tracks
    if (a_iValue == 0) {
        autoDJRandomQueueMinimumSpinBox->setEnabled(false);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
                ConfigValue(0));
    } else {
        autoDJRandomQueueMinimumSpinBox->setEnabled(true);
        m_pConfig->set(ConfigKey("[Auto DJ]", "EnableRandomQueue"),
                ConfigValue(1));
    }
#endif // __AUTODJCRATES__
}

