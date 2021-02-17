#include "preferences/dialog/dlgprefrecord.h"

#include <QFileDialog>
#include <QStandardPaths>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "encoder/encoder.h"
#include "encoder/encodermp3settings.h"
#include "moc_dlgprefrecord.cpp"
#include "recording/defs_recording.h"
#include "util/sandbox.h"

namespace {
constexpr bool kDefaultCueEnabled = true;
} // anonymous namespace

DlgPrefRecord::DlgPrefRecord(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_selFormat("","",false)
{
    setupUi(this);

    // Setting recordings path.
    QString recordingsPath = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"));
    if (recordingsPath.isEmpty()) {
        // Initialize recordings path in config to old default path.
        // Do it here so we show current value in UI correctly.
        QString musicDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        QDir recordDir(musicDir + "/Mixxx/Recordings");
        recordingsPath = recordDir.absolutePath();
        m_pConfig->setValue(ConfigKey(RECORDING_PREF_KEY, "Directory"), recordingsPath);
    }
    LineEditRecordings->setText(recordingsPath);
    connect(PushButtonBrowseRecordings,
            &QAbstractButton::clicked,
            this,
            &DlgPrefRecord::slotBrowseRecordingsDir);

    // Setting Encoder
    bool found = false;
    QString prefformat = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));
    for (const Encoder::Format& format : EncoderFactory::getFactory().getFormats()) {
        QRadioButton* button = new QRadioButton(format.label, this);
        button->setObjectName(format.internalName);
        connect(button, &QAbstractButton::clicked, this, &DlgPrefRecord::slotFormatChanged);
        if (format.lossless) {
            LosslessEncLayout->addWidget(button);
        } else {
            LossyEncLayout->addWidget(button);
        }
        encodersgroup.addButton(button);

        if (prefformat == format.internalName) {
            m_selFormat = format;
            button->setChecked(true);
            found=true;
        }
        m_formatButtons.append(button);
    }
    if (!found) {
        // If no format was available, set to WAVE as default.
        if (!prefformat.isEmpty()) {
            qWarning() << prefformat <<" format was set in the configuration, but it is not recognized!";
        }
        m_selFormat = EncoderFactory::getFactory().getFormats().first();
        m_formatButtons.first()->setChecked(true);
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"),  ConfigValue(m_selFormat.internalName));
    }

    setupEncoderUI();

    // Setting Metadata
    loadMetaData();

    // Setting miscellaneous
    CheckBoxRecordCueFile->setChecked(m_pConfig->getValue<bool>(
            ConfigKey(RECORDING_PREF_KEY, "CueEnabled"), kDefaultCueEnabled));

    // Setting split
    comboBoxSplitting->addItem(SPLIT_650MB);
    comboBoxSplitting->addItem(SPLIT_700MB);
    comboBoxSplitting->addItem(SPLIT_1024MB);
    comboBoxSplitting->addItem(SPLIT_2048MB);
    comboBoxSplitting->addItem(SPLIT_4096MB);
    comboBoxSplitting->addItem(SPLIT_60MIN);
    comboBoxSplitting->addItem(SPLIT_74MIN);
    comboBoxSplitting->addItem(SPLIT_80MIN);
    comboBoxSplitting->addItem(SPLIT_120MIN);

    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
    int index = comboBoxSplitting->findText(fileSizeStr);
    if (index >= 0) {
        // Set file split size
        comboBoxSplitting->setCurrentIndex(index);
    }
    else {
        //Use max RIFF size (4GB) as default index, since usually people don't want to split.
        comboBoxSplitting->setCurrentIndex(4);
    }

    // Do the one-time connection of signals here.
    connect(SliderQuality, &QAbstractSlider::valueChanged, this, &DlgPrefRecord::slotSliderQuality);
    connect(SliderQuality, &QAbstractSlider::sliderMoved, this, &DlgPrefRecord::slotSliderQuality);
    connect(SliderQuality,
            &QAbstractSlider::sliderReleased,
            this,
            &DlgPrefRecord::slotSliderQuality);
    connect(SliderCompression,
            &QAbstractSlider::valueChanged,
            this,
            &DlgPrefRecord::slotSliderCompression);
    connect(SliderCompression,
            &QAbstractSlider::sliderMoved,
            this,
            &DlgPrefRecord::slotSliderCompression);
    connect(SliderCompression,
            &QAbstractSlider::sliderReleased,
            this,
            &DlgPrefRecord::slotSliderCompression);
}

DlgPrefRecord::~DlgPrefRecord()
{
    // Note: I don't disconnect signals, since that's supposedly done automatically
    // when the object is deleted
    for (QRadioButton* button : qAsConst(m_formatButtons)) {
        if (LosslessEncLayout->indexOf(button) != -1) {
            LosslessEncLayout->removeWidget(button);
        } else {
            LossyEncLayout->removeWidget(button);
        }
        button->deleteLater();
    }
    for (QAbstractButton* widget : qAsConst(m_optionWidgets)) {
        OptionGroupsLayout->removeWidget(widget);
        widget->deleteLater();
    }
    m_optionWidgets.clear();
}

void DlgPrefRecord::slotApply()
{
    saveRecordingFolder();
    saveMetaData();
    saveEncoding();
    saveUseCueFile();
    saveSplitSize();
}

// This function updates/refreshes the contents of this dialog.
void DlgPrefRecord::slotUpdate()
{
    // Find out the max width of the labels. This is needed to keep the
    // UI fixed in size when hiding or showing elements.
    // It is not perfect, but it didn't get better than this.
    int max=0;
    if (LabelQuality->size().width()> max) {
        max=LabelQuality->size().width();
    }
    LabelLossless->setMaximumWidth(max);
    LabelLossy->setMaximumWidth(max);
    LabelLossless->setMinimumWidth(max);
    LabelLossy->setMinimumWidth(max);

    QString recordingsPath = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"));
    LineEditRecordings->setText(recordingsPath);

    QString prefformat = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));
    for (const Encoder::Format& format : EncoderFactory::getFactory().getFormats()) {
        if (prefformat == format.internalName) {
            m_selFormat = format;
            break;
        }
    }
    setupEncoderUI();

    loadMetaData();

     // Setting miscellaneous
    CheckBoxRecordCueFile->setChecked(m_pConfig->getValue<bool>(
            ConfigKey(RECORDING_PREF_KEY, "CueEnabled"), kDefaultCueEnabled));

    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
    int index = comboBoxSplitting->findText(fileSizeStr);
    if (index >= 0) {
        comboBoxSplitting->setCurrentIndex(index);
    }
}

void DlgPrefRecord::slotResetToDefaults()
{
    m_formatButtons.first()->setChecked(true);
    m_selFormat = EncoderFactory::getFactory().getFormatFor(
            m_formatButtons.first()->objectName());
    setupEncoderUI();
    // TODO (XXX): It would be better that a defaultSettings() method is added
    // to the EncoderSettings interface so that we know which option to set
    m_optionWidgets.first()->setChecked(true);

    LineEditTitle->setText("");
    LineEditAlbum->setText("");
    LineEditAuthor->setText("");

    // 4GB splitting is the default
    comboBoxSplitting->setCurrentIndex(4);
    CheckBoxRecordCueFile->setChecked(kDefaultCueEnabled);
}


void DlgPrefRecord::slotBrowseRecordingsDir()
{
    QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose recordings directory"),
            m_pConfig->getValueString(
                    ConfigKey(RECORDING_PREF_KEY,"Directory")));

    if (fd != "") {
        // The user has picked a new directory via a file dialog. This means the
        // system sandboxer (if we are sandboxed) has granted us permission to
        // this folder. Create a security bookmark while we have permission so
        // that we can access the folder on future runs. We need to canonicalize
        // the path so we first wrap the directory string with a QDir.
        QDir directory(fd);
        Sandbox::createSecurityToken(directory);
        LineEditRecordings->setText(fd);
    }
}

void DlgPrefRecord::slotFormatChanged()
{
    QObject *senderObj = sender();
    m_selFormat = EncoderFactory::getFactory().getFormatFor(senderObj->objectName());
    setupEncoderUI();
}

void DlgPrefRecord::setupEncoderUI() {
    EncoderRecordingSettingsPointer settings =
            EncoderFactory::getFactory().getEncoderRecordingSettings(
                    m_selFormat, m_pConfig);
    if (settings->usesQualitySlider()) {
        LabelQuality->setVisible(true);
        SliderQuality->setVisible(true);
        TextQuality->setVisible(true);
        SliderQuality->setMinimum(0);
        SliderQuality->setMaximum(settings->getQualityValues().size()-1);
        SliderQuality->setValue(settings->getQualityIndex());
        updateTextQuality();
    } else {
        LabelQuality->setVisible(false);
        SliderQuality->setVisible(false);
        TextQuality->setVisible(false);
    }
    if (settings->usesCompressionSlider()) {
        LabelCompression->setVisible(true);
        SliderCompression->setVisible(true);
        TextCompression->setVisible(true);
        SliderCompression->setMinimum(0);
        SliderCompression->setMaximum(settings->getCompressionValues().size()-1);
        SliderCompression->setValue(settings->getCompression());
        updateTextCompression();
    } else {
        LabelCompression->setVisible(false);
        SliderCompression->setVisible(false);
        TextCompression->setVisible(false);
    }

    for (QAbstractButton* widget : qAsConst(m_optionWidgets)) {
        optionsgroup.removeButton(widget);
        OptionGroupsLayout->removeWidget(widget);
        disconnect(widget, &QAbstractButton::clicked, this, &DlgPrefRecord::slotGroupChanged);
        widget->deleteLater();
    }
    m_optionWidgets.clear();
    if (!settings->getOptionGroups().isEmpty()) {
        labelOptionGroup->setVisible(true);
        // TODO (XXX): Right now i am supporting just one optiongroup.
        // The concept is already there for multiple groups
        // It will require to generate the buttongroup dynamically like:
        // >> buttongroup = new QButtonGroup(this);
        // >> buttongroup->addButton(radioButtonNoFFT);
        // >> buttongroup->addButton(radioButtonFFT);

        EncoderSettings::OptionsGroup group = settings->getOptionGroups().first();
        labelOptionGroup->setText(group.groupName);
        int controlIdx = settings->getSelectedOption(group.groupCode);
        for (const QString& name : qAsConst(group.controlNames)) {
            QAbstractButton* widget;
            if (group.controlNames.size() == 1) {
                QCheckBox* button = new QCheckBox(name, this);
                widget = button;
            } else {
                QRadioButton* button = new QRadioButton(name, this);
                widget = button;
            }
            connect(widget, &QAbstractButton::clicked, this, &DlgPrefRecord::slotGroupChanged);
            widget->setObjectName(group.groupCode);
            OptionGroupsLayout->addWidget(widget);
            optionsgroup.addButton(widget);
            m_optionWidgets.append(widget);
            if (controlIdx == 0 ) {
                widget->setChecked(true);
            }
            controlIdx--;
        }
    } else {
        labelOptionGroup->setVisible(false);
    }
        // small hack for VBR
    if (m_selFormat.internalName == ENCODING_MP3) {
        updateTextQuality();
    }
}

void DlgPrefRecord::slotSliderQuality()
{
    updateTextQuality();
    // Settings are only stored when doing an apply so that "cancel" can actually cancel.
}

void DlgPrefRecord::updateTextQuality() {
    EncoderRecordingSettingsPointer settings =
            EncoderFactory::getFactory().getEncoderRecordingSettings(
                    m_selFormat, m_pConfig);
    int quality;
    // This should be handled somehow by the EncoderSettings classes, but currently
    // I don't have a clean way to do it
    bool isVbr = false;
    if (m_selFormat.internalName == ENCODING_MP3) {
        EncoderSettings::OptionsGroup group = settings->getOptionGroups().first();
        int i=0;
        for (const QAbstractButton* widget : qAsConst(m_optionWidgets)) {
            if (widget->objectName() == group.groupCode) {
                if (widget->isChecked() != Qt::Unchecked && widget->text() == "VBR") {
                    isVbr = true;
                }
                i++;
            }
        }
    }
    if (isVbr) {
        EncoderMp3Settings* settingsmp3 = static_cast<EncoderMp3Settings*>(&(*settings));
        quality = settingsmp3->getVBRQualityValues().at(SliderQuality->value());
        TextQuality->setText(QString(QString::number(quality) + " kbit/s"));
    } else {
        quality = settings->getQualityValues().at(SliderQuality->value());
        TextQuality->setText(QString(QString::number(quality) + " kbit/s"));
    }
}

void DlgPrefRecord::slotSliderCompression()
{
    updateTextCompression();
    // Settings are only stored when doing an apply so that "cancel" can actually cancel.
}

void DlgPrefRecord::updateTextCompression() {
    EncoderRecordingSettingsPointer settings =
            EncoderFactory::getFactory().getEncoderRecordingSettings(
                    m_selFormat, m_pConfig);
    int quality = settings->getCompressionValues().at(SliderCompression->value());
    TextCompression->setText(QString::number(quality));
}

void DlgPrefRecord::slotGroupChanged()
{
    // On complex scenarios, one could want to enable or disable some controls when changing
    // these, but we don't have these needs now.
    EncoderRecordingSettingsPointer settings =
            EncoderFactory::getFactory().getEncoderRecordingSettings(
                    m_selFormat, m_pConfig);
    if (settings->usesQualitySlider()) {
        updateTextQuality();
    }
}

void DlgPrefRecord::loadMetaData() {
    LineEditTitle->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")));
    LineEditAuthor->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")));
    LineEditAlbum->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")));
}


void DlgPrefRecord::saveRecordingFolder()
{
    QString newPath = LineEditRecordings->text();
    if (newPath != m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"))) {
        QFileInfo fileInfo(newPath);
        if (!fileInfo.exists()) {
            QMessageBox::warning(
                    this,
                    tr("Recordings directory invalid"),
                    tr("Recordings directory must be set to an existing directory."));
            return;
        }
        if (!fileInfo.isDir()) {
            QMessageBox::warning(
                    this,
                    tr("Recordings directory invalid"),
                    tr("Recordings directory must be set to a directory."));
            return;
        }
        if (!fileInfo.isWritable()) {
            QMessageBox::warning(this,
                    tr("Recordings directory not writable"),
                    tr("You do not have write access to %1. Choose a "
                       "recordings directory you have write access to.")
                            .arg(newPath));
            return;
        }
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Directory"), newPath);
    }
}
void DlgPrefRecord::saveMetaData()
{
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Title"), ConfigValue(LineEditTitle->text()));
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Author"), ConfigValue(LineEditAuthor->text()));
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Album"), ConfigValue(LineEditAlbum->text()));
}

void DlgPrefRecord::saveEncoding() {
    EncoderRecordingSettingsPointer settings =
            EncoderFactory::getFactory().getEncoderRecordingSettings(
                    m_selFormat, m_pConfig);
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"),
        ConfigValue(m_selFormat.internalName));

    if (settings->usesQualitySlider()) {
        settings->setQualityByIndex(SliderQuality->value());
    }
    if (settings->usesCompressionSlider()) {
        QList<int> comps = settings->getCompressionValues();
        settings->setCompression(comps.at(SliderCompression->value()));
    }
    if (!settings->getOptionGroups().isEmpty()) {
        // TODO (XXX): Right now i am supporting just one optiongroup.
        // The concept is already there for multiple groups
        EncoderSettings::OptionsGroup group = settings->getOptionGroups().first();
        int i=0;
        for (const QAbstractButton* widget : qAsConst(m_optionWidgets)) {
            if (widget->objectName() == group.groupCode) {
                if (widget->isChecked() != Qt::Unchecked) {
                    settings->setGroupOption(group.groupCode, i);
                    break;
                }
                i++;
            }
        }
    }
}

void DlgPrefRecord::saveUseCueFile() {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "CueEnabled"),
                   ConfigValue(CheckBoxRecordCueFile->isChecked()));
}

void DlgPrefRecord::saveSplitSize() {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "FileSize"),
                   ConfigValue(comboBoxSplitting->currentText()));
}
