#include "preferences/dialog/dlgprefrecord.h"

#include <QFileDialog>
#include <QRadioButton>
#include <QStandardPaths>

#include "control/controlproxy.h"
#include "encoder/encoder.h"
#include "encoder/encodermp3settings.h"
#include "moc_dlgprefrecord.cpp"
#include "recording/defs_recording.h"
#include "util/sandbox.h"

namespace {
constexpr bool kDefaultCueEnabled = true;
constexpr bool kDefaultCueFileAnnotationEnabled = false;

const ConfigKey kRecSampleRateCfgkey =
        ConfigKey(QStringLiteral(RECORDING_PREF_KEY), QStringLiteral("rec_samplerate"));

const ConfigKey kUseEngineSampleRateCfgkey =
        ConfigKey(QStringLiteral(RECORDING_PREF_KEY), QStringLiteral("use_engine_samplerate"));

} // anonymous namespace

DlgPrefRecord::DlgPrefRecord(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_selFormat({QString(), QString(), false, QString()}),
          m_recSampleRate(kRecSampleRateCfgkey),
          m_useEngineSampleRate(kUseEngineSampleRateCfgkey) {
    setupUi(this); // uses the .ui file to generate qt gui elements.

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

    // generate custom recording samplerates
    const QList<mixxx::audio::SampleRate>* customSampleRatesList = nullptr;
    if (prefformat == ENCODING_OPUS) {
        customSampleRatesList = &kRecSampleRatesOpus;
    } else if (prefformat == ENCODING_MP3) {
        customSampleRatesList = &kRecSampleRatesMP3;
    } else if (prefformat == ENCODING_OGG) {
        customSampleRatesList = &kRecSampleRatesOGG;
    } else {
        customSampleRatesList = &kRecSampleRates;
    }
    updateSampleRates(*customSampleRatesList);

    auto engineSampleRateProxy = PollingControlProxy(
            QStringLiteral("[App]"), QStringLiteral("samplerate"));
    double engineSampleRate = engineSampleRateProxy.get();
    RadioButtonUseDefaultSampleRate->setText(QString("Default (%1 Hz)").arg(engineSampleRate));

    m_defaultSampleRate = engineSampleRate;
    m_oldRecSampleRate = m_recSampleRate.get();
    qDebug() << "samplerate from previous session: " << m_oldRecSampleRate;

    // Restore previous sample-rate
    if (m_useEngineSampleRate.toBool()) {
        // engine sample rate was used in the previous session
        // select the default radiobutton
        RadioButtonUseDefaultSampleRate->setChecked(true);
    } else {
        // select the combo-box item
        int idx = recSampleRateComboBox->findData(QVariant::fromValue(m_oldRecSampleRate));
        qDebug() << idx;
        recSampleRateComboBox->setCurrentIndex(idx);
        RadioButtonUseCustomSampleRate->setChecked(true);
    }

    // Setting Metadata
    loadMetaData();

    // Setting miscellaneous
    CheckBoxRecordCueFile->setChecked(m_pConfig->getValue<bool>(
            ConfigKey(RECORDING_PREF_KEY, "CueEnabled"), kDefaultCueEnabled));

    CheckBoxUseCueFileAnnotation->setChecked(m_pConfig->getValue<bool>(
            ConfigKey(RECORDING_PREF_KEY, "cue_file_annotation_enabled"),
            kDefaultCueFileAnnotationEnabled));

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
    } else {
        //Use max RIFF size (4GB) as default index, since usually people don't want to split.
        comboBoxSplitting->setCurrentIndex(4);
        saveSplitSize();
    }

    setScrollSafeGuard(comboBoxSplitting);

    connect(RadioButtonUseDefaultSampleRate,
            &QRadioButton::toggled,
            this,
            [this](bool checked) {
                slotToggleCustomSampleRateIgnore(checked ? Qt::Checked : Qt::Unchecked);
            });

    connect(recSampleRateComboBox,
            &QComboBox::currentIndexChanged,
            this,
            &DlgPrefRecord::slotSampleRateChanged);

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

    connect(CheckBoxRecordCueFile,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefRecord::slotToggleCueEnabled);

    // add new element ex here, define the slot that is
    // called when a new value is chosen. arg0 of connect
    // must be used in the corresponding .ui file
}

DlgPrefRecord::~DlgPrefRecord() {
    // Note: I don't disconnect signals, since that's supposedly done automatically
    // when the object is deleted
    for (QRadioButton* button : std::as_const(m_formatButtons)) {
        if (LosslessEncLayout->indexOf(button) != -1) {
            LosslessEncLayout->removeWidget(button);
        } else {
            LossyEncLayout->removeWidget(button);
        }
        button->deleteLater();
    }
    for (QAbstractButton* widget : std::as_const(m_optionWidgets)) {
        OptionGroupsLayout->removeWidget(widget);
        widget->deleteLater();
    }
    m_optionWidgets.clear();
}

void DlgPrefRecord::slotApply() {
    saveRecordingFolder();
    saveMetaData();
    saveEncoding();
    saveUseCueFile();
    saveUseCueFileAnnotation();
    saveSplitSize();
    saveRecSampleRate();
}

// Called when:
// - Default samplerate radio button is selected.
// - Custom samplerate radio button is selected.
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefRecord::slotToggleCustomSampleRateIgnore(Qt::CheckState buttonState) {
#else
void DlgPrefRecord::slotToggleCustomSampleRateIgnore(int buttonState) {
#endif
    qDebug() << "defaultradiobutton state: " << buttonState;
    recSampleRateComboBox->setEnabled(buttonState != Qt::Checked);

    // if custom samplerate enabled: set false
    if (RadioButtonUseCustomSampleRate->isChecked()) {
        m_useEngineSampleRate.set(false);
        return;
    }

    m_useEngineSampleRate.set(true);
    qDebug() << "Using default (engine samplerate) as recording samplerate " << m_defaultSampleRate;
}

// m_pConfig is accessible from the enginerecord instance.
// only when a custom recording samplerate is being used.
void DlgPrefRecord::saveRecSampleRate() {
    double recSampleRate{};
    if (!m_useEngineSampleRate.toBool()) {
        // samplerate values stored as double in additem()
        recSampleRate = recSampleRateComboBox->currentData().value<double>();
        qDebug() << "Apply custom: recording samplerate changed to: " << recSampleRate << " Hz";
    } else {
        recSampleRate = m_defaultSampleRate;
        qDebug() << "Apply default: recording samplerate changed to: " << recSampleRate << " Hz";
    }
    m_recSampleRate.set(recSampleRate);

    // when a samplerate is applied, it becomes the base
    // for future changes.
    m_oldRecSampleRate = recSampleRate;

    m_pConfig->set(kRecSampleRateCfgkey, ConfigValue(m_recSampleRate.get()));
}
// This function updates/refreshes the contents of this dialog.
void DlgPrefRecord::slotUpdate() {
    // Find out the max width of the labels. This is needed to keep the
    // UI fixed in size when hiding or showing elements.
    // It is not perfect, but it didn't get better than this.
    int max=0;
    if (LabelQuality->size().width()> max) {
        max = LabelQuality->size().width() + 10; // quick fix for gui bug
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

    slotToggleCueEnabled();

    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
    int index = comboBoxSplitting->findText(fileSizeStr);
    if (index >= 0) {
        comboBoxSplitting->setCurrentIndex(index);
    }
}

void DlgPrefRecord::slotResetToDefaults() {
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

    // Sets 'Create a CUE file' checkbox value
    CheckBoxRecordCueFile->setChecked(kDefaultCueEnabled);

    // Sets 'Enable File Annotation in CUE file' checkbox value
    CheckBoxUseCueFileAnnotation->setChecked(kDefaultCueFileAnnotationEnabled);

    // Let 48000 Hz be the "default" non-engine sample rate.
    int index = recSampleRateComboBox->findData(QVariant::fromValue(48000.0));
    if (index != -1) {
        recSampleRateComboBox->setCurrentIndex(index);
    }
}

void DlgPrefRecord::slotBrowseRecordingsDir() {
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
        Sandbox::createSecurityTokenForDir(directory);
        LineEditRecordings->setText(fd);
    }
}

void DlgPrefRecord::slotFormatChanged() {
    QObject *senderObj = sender();
    m_selFormat = EncoderFactory::getFactory().getFormatFor(senderObj->objectName());
    const auto& formatName = m_selFormat.internalName;
    qDebug() << senderObj->objectName() << ": Format updated to " << formatName;
    qDebug() << "Samplerate before changing format, needs to persist: " << m_oldRecSampleRate;

    // Recreate the available custom samplerates GUI according to the
    // newly chosen format
    const QList<mixxx::audio::SampleRate>* customSampleRatesList = nullptr;
    if (m_selFormat.internalName == ENCODING_MP3) {
        customSampleRatesList = &kRecSampleRatesMP3;
    } else if (m_selFormat.internalName == ENCODING_OPUS) {
        customSampleRatesList = &kRecSampleRatesOpus;
    } else if (m_selFormat.internalName == ENCODING_OGG) {
        customSampleRatesList = &kRecSampleRatesOGG;
    } else {
        customSampleRatesList = &kRecSampleRates;
    }
    updateSampleRates(*customSampleRatesList);

    // It is possible that the current default samplerate
    // i.e. engine sample rate is not supported by certain
    // formats (ex. OPUS does not support 96khz, 44.1khz). In this case,
    // we must disable the default radio button, and select the
    // custom radiobutton.
    bool isDefaultRateValid = true;
    auto defaultRate = mixxx::audio::SampleRate(static_cast<unsigned>(m_defaultSampleRate));
    qDebug() << "Current default samplerate: " << m_defaultSampleRate;
    isDefaultRateValid = customSampleRatesList->contains(defaultRate);
    qDebug() << "Is the current default samplerate a valid rec samplerate for "
                "this format? "
             << isDefaultRateValid;
    RadioButtonUseDefaultSampleRate->setEnabled(isDefaultRateValid);

    // Now we inspect the user-selected value
    // It is possible that the old recording samplerate
    // is also available in the new format. In this case, we
    // have two choices: If the old sample rate equals the current default
    // (engine) samplerate, we check the usedefault radio button. If not,
    // we select the correct index from the combo-box. If the combo-box
    // does not have the value either, we must conclude that this
    // samplerate value is not common to the old and new formats.
    // Warn that the recording samplerate will have to be
    // changed, and allow the user to select a samplerate
    // from the new custom list. This gui action triggers
    // slotsampleratechanged and all is good.
    int recSampleRateIdx = recSampleRateComboBox->findData(
            QVariant::fromValue(m_oldRecSampleRate));
    if (recSampleRateIdx != -1) { // found in the combobox
        qDebug() << "Samplerate " << m_oldRecSampleRate << " found in combo-box";
        RadioButtonUseDefaultSampleRate->setChecked(false);       // calls slot ignore
        recSampleRateComboBox->setCurrentIndex(recSampleRateIdx); // calls slot sampleratechanged
        RadioButtonUseCustomSampleRate->setChecked(true);
        qDebug() << "checked custom samplerate radiobutton";
    } else {
        // If the updated GUI list is missing the old rec samplerate,
        // that samplerate is the new default, i.e. engine samplerate.
        // select the default radio button instead.
        // However it could also be the case that even the default samplerate
        // is not supported. Need to check the value of default samplerate.
        qDebug() << "Default rate: " << m_defaultSampleRate << " old rate: " << m_oldRecSampleRate;
        if (isDefaultRateValid) {
            RadioButtonUseDefaultSampleRate->setEnabled(true);
            if (m_defaultSampleRate == m_oldRecSampleRate) {
                RadioButtonUseDefaultSampleRate->setChecked(true);
            } else {
                RadioButtonUseCustomSampleRate->setChecked(true);
            }
        } else {
            RadioButtonUseDefaultSampleRate->setEnabled(false);
            RadioButtonUseDefaultSampleRate->setChecked(false);
            RadioButtonUseCustomSampleRate->setChecked(true);
            qWarning() << "The current format does not support the original "
                          "recording samplerate ("
                       << m_oldRecSampleRate << "Hz)";
        }
    }
    setupEncoderUI();
}

// Update the set of custom sample rates offered in
// the recording combo-box.
// called on 2 occasions during a Mixxx run:
// 1. From a slot, whenever the engine samplerate is updated.
// 2. From a slot, whenever the recording format is changed.
void DlgPrefRecord::updateSampleRates(
        const QList<mixxx::audio::SampleRate>& sampleRates) {
    recSampleRateComboBox->blockSignals(true);

    recSampleRateComboBox->clear();
    for (const auto& sampleRate : sampleRates) {
        if (sampleRate.isValid()) {
            if (sampleRate.value() == m_defaultSampleRate) {
                continue; // do not add default samplerate to list.
            }
            recSampleRateComboBox->addItem(tr("%1 Hz").arg(sampleRate.value()),
                    QVariant::fromValue(sampleRate.value()));
        }
    }
    recSampleRateComboBox->blockSignals(false);
}

void DlgPrefRecord::setupEncoderUI() {
    EncoderRecordingSettingsPointer settings =
            EncoderFactory::getFactory().getEncoderRecordingSettings(
                    m_selFormat, m_pConfig);
    if (settings->usesQualitySlider()) {
        qualityGroup->setVisible(true);
        SliderQuality->setMinimum(0);
        SliderQuality->setMaximum(settings->getQualityValues().size()-1);
        SliderQuality->setValue(settings->getQualityIndex());
        updateTextQuality();
    } else {
        qualityGroup->setVisible(false);
    }
    if (settings->usesCompressionSlider()) {
        compressionGroup->setVisible(true);
        SliderCompression->setMinimum(0);
        SliderCompression->setMaximum(settings->getCompressionValues().size()-1);
        SliderCompression->setValue(settings->getCompression());
        updateTextCompression();
    } else {
        compressionGroup->setVisible(false);
    }

    for (QAbstractButton* widget : std::as_const(m_optionWidgets)) {
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
        for (const QString& name : std::as_const(group.controlNames)) {
            QAbstractButton* widget;
            if (group.controlNames.size() == 1) {
                QCheckBox* button = new QCheckBox(name, this);
                widget = button;
            } else {
                QRadioButton* button = new QRadioButton(name, this); // qradiobutton
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

void DlgPrefRecord::slotSliderQuality() {
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
        for (const QAbstractButton* widget : std::as_const(m_optionWidgets)) {
            if (widget->objectName() == group.groupCode) {
                if (widget->isChecked() != Qt::Unchecked && widget->text() == "VBR") {
                    isVbr = true;
                }
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

void DlgPrefRecord::slotSliderCompression() {
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

void DlgPrefRecord::saveRecordingFolder() {
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

void DlgPrefRecord::saveMetaData() {
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
        for (const QAbstractButton* widget : std::as_const(m_optionWidgets)) {
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

// Set 'Enable File Annotation in CUE file' checkbox value depending on 'Create
// a CUE file' checkbox value
void DlgPrefRecord::slotToggleCueEnabled() {
    CheckBoxUseCueFileAnnotation->setEnabled(CheckBoxRecordCueFile
                    ->isChecked());
}

void DlgPrefRecord::saveUseCueFile() {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "CueEnabled"),
                   ConfigValue(CheckBoxRecordCueFile->isChecked()));
}

void DlgPrefRecord::saveUseCueFileAnnotation() {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "cue_file_annotation_enabled"),
            ConfigValue(CheckBoxUseCueFileAnnotation->isChecked()));
}

void DlgPrefRecord::saveSplitSize() {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "FileSize"),
                   ConfigValue(comboBoxSplitting->currentText()));
}

// only when recsamplerate combobox is chosen, i.e.
// custom recording samplerate.
void DlgPrefRecord::slotSampleRateChanged(int newRateIdx) {
    if (m_useEngineSampleRate.toBool()) {
        qDebug() << "Rec samplerate set changed, but using default (engine) samplerate.";
        return;
    }
    // addItem() is storing samplerate double values.
    auto recSampleRateNew = recSampleRateComboBox->itemData(newRateIdx).value<double>();

    qDebug() << "rec samplerate change triggered, new rate: " << recSampleRateNew;
    recSampleRateComboBox->setCurrentIndex(newRateIdx);

    m_useEngineSampleRate.set(false);
}

void DlgPrefRecord::slotDefaultSampleRateUpdated(mixxx::audio::SampleRate newRate) {
    qDebug() << "Engine samplerate changed, rec dlg received sample rate update:" << newRate;
    RadioButtonUseDefaultSampleRate->setText(QString("Default (%1 Hz)").arg(newRate.toDouble()));
    double rate = newRate.toDouble();
    m_defaultSampleRate = rate;

    // recreate the available custom samplerates
    // in the GUI
    bool enableRadioButtonDefault = true;
    const QList<mixxx::audio::SampleRate>* customSampleRatesList = nullptr;
    double oldSampleRate = m_oldRecSampleRate;

    if (m_selFormat.internalName == ENCODING_MP3) {
        customSampleRatesList = &kRecSampleRatesMP3;
    } else if (m_selFormat.internalName == ENCODING_OPUS) {
        customSampleRatesList = &kRecSampleRatesOpus;
    } else if (m_selFormat.internalName == ENCODING_OGG) {
        customSampleRatesList = &kRecSampleRatesOGG;
    } else {
        customSampleRatesList = &kRecSampleRates;
    }

    // If the current format allows the value of
    // new engine samplerate (i.e. default rec samplerate),
    // allow the user to choose it.
    enableRadioButtonDefault = customSampleRatesList->contains(newRate);
    RadioButtonUseDefaultSampleRate->setEnabled(enableRadioButtonDefault);

    // Generate a GUI list of custom samplerates, removing the
    // new engine samplerate if it is supported by the
    // format.
    updateSampleRates(*customSampleRatesList);

    // Ensure that the previous recording samplerate remains unchanged
    // even if the engine samplerate has changed.
    qDebug() << "GUI-chosen (need not be applied) old samplerate: " << oldSampleRate;
    int recSampleRateIdx = recSampleRateComboBox->findData(
            QVariant::fromValue(oldSampleRate));
    if (recSampleRateIdx != -1) { // found in the combobox
        qDebug() << oldSampleRate << " found in combobox for format " << m_selFormat.internalName;
        RadioButtonUseDefaultSampleRate->setChecked(false); // calls slot ignore
        recSampleRateComboBox->setCurrentIndex(recSampleRateIdx);
    } else {
        // If the updated GUI list is missing the old rec samplerate,
        // that samplerate is the new default, i.e. engine samplerate.
        // select the default radio button instead.
        // However it could also be the case that even the default samplerate
        // is not supported. Need to check the value of default samplerate.
        qDebug() << "Default rate: " << m_defaultSampleRate << " New rate: " << newRate;
        DEBUG_ASSERT(m_defaultSampleRate == newRate.value());

        auto defaultRate = mixxx::audio::SampleRate(static_cast<unsigned>(m_defaultSampleRate));
        bool isDefaultSampleRateSupported = customSampleRatesList->contains(defaultRate);

        RadioButtonUseCustomSampleRate->setChecked(!isDefaultSampleRateSupported);
        // calls slottogglecustomsamplerateignore
        RadioButtonUseDefaultSampleRate->setChecked(isDefaultSampleRateSupported);
    }
}
