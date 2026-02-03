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
          m_useEngineSampleRate(kUseEngineSampleRateCfgkey),
          m_defaultSampleRate(PollingControlProxy(
                  QStringLiteral("[App]"), QStringLiteral("samplerate"))
                          .get()),
          m_oldRecSampleRate(m_recSampleRate.get()) {
    setupUi(this); // uses the .ui file to generate qt gui elements.

    // This is a hack to set the correct positioning.
    // Values found by trial-and-error
    RadioButtonUseCustomSampleRate->setMaximumWidth(80);
    RadioButtonUseDefaultSampleRate->setMaximumWidth(234);

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

    qDebug() << "Default samplerate = engine samplerate: " << m_defaultSampleRate;
    RadioButtonUseDefaultSampleRate->setText(tr("Default (%1 Hz)").arg(m_defaultSampleRate));

    // generate list of custom recording samplerates
    [[maybe_unused]] const auto& customSampleRatesList = createSampleRateGUIForFormat(prefformat);

    // Restore previous sample-rate
    if (m_useEngineSampleRate.toBool()) {
        // engine sample rate was used in the previous session
        qDebug() << "Engine Samplerate was used in previous session: " << m_oldRecSampleRate;
        // select the default radiobutton
        RadioButtonUseDefaultSampleRate->setChecked(true);
    } else {
        // select the combo-box item
        qDebug() << "Custom Samplerate was used in the old session: " << m_oldRecSampleRate;
        int idx = comboBoxRecSampleRate->findData(QVariant::fromValue(m_oldRecSampleRate));
        comboBoxRecSampleRate->setCurrentIndex(idx);
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

    connect(comboBoxRecSampleRate,
            &QComboBox::activated,
            this,
            &DlgPrefRecord::slotComboBoxItemClicked);

    connect(comboBoxRecSampleRate,
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

// only called on a user-triggered index change.
void DlgPrefRecord::slotComboBoxItemClicked([[maybe_unused]] int index) {
    RadioButtonUseCustomSampleRate->setChecked(true);
}

// Called when:
// - Default samplerate radio button is selected.
// - Custom samplerate radio button is selected.
void DlgPrefRecord::slotToggleCustomSampleRateIgnore([[maybe_unused]] int buttonState) {
    // comboBoxRecSampleRate->setEnabled(buttonState != Qt::Checked);

    // if custom samplerate enabled: set false
    if (RadioButtonUseCustomSampleRate->isChecked()) {
        m_useEngineSampleRate.set(false);
        return;
    }

    m_useEngineSampleRate.set(true);
}

// m_pConfig is accessible from the enginerecord instance.
// only when a custom recording samplerate is being used.
void DlgPrefRecord::saveRecSampleRate() {
    double recSampleRate{};
    if (!m_useEngineSampleRate.toBool()) {
        // samplerate values stored as double in additem()
        recSampleRate = comboBoxRecSampleRate->currentData().value<double>();
        qDebug() << "Recording sample rate set to" << recSampleRate << "(custom)";
    } else {
        recSampleRate = m_defaultSampleRate;
        qDebug() << "Recording sample rate set to" << recSampleRate << "(engine sample rate)";
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
    int index = comboBoxRecSampleRate->findData(QVariant::fromValue(48000.0));
    if (index != -1) {
        comboBoxRecSampleRate->setCurrentIndex(index);
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
    // Recreate the available custom samplerates GUI according to the
    // newly chosen format
    const auto& customSampleRatesList = createSampleRateGUIForFormat(m_selFormat.internalName);

    // It is possible that the current default samplerate
    // i.e. engine sample rate is not supported by certain
    // formats (ex. OPUS does not support 96khz, 44.1khz). In this case,
    // we must disable the default radio button, and select the
    // custom radiobutton.
    const auto defaultRate = mixxx::audio::SampleRate(static_cast<unsigned>(m_defaultSampleRate));
    bool isDefaultRateValid = customSampleRatesList.contains(defaultRate);
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
    int recSampleRateIdx = comboBoxRecSampleRate->findData(
            QVariant::fromValue(m_oldRecSampleRate));
    if (!m_useEngineSampleRate.toBool() && (recSampleRateIdx != -1)) { // found in the combobox
        RadioButtonUseDefaultSampleRate->setChecked(false);       // calls slot ignore
        comboBoxRecSampleRate->setCurrentIndex(recSampleRateIdx); // calls slot sampleratechanged
        RadioButtonUseCustomSampleRate->setChecked(true);
    } else {
        // If the updated GUI list is missing the old rec samplerate,
        // that samplerate is the new default, i.e. engine samplerate.
        // Select the Default radio button instead.
        // However it could also be the case that even the default samplerate
        // is not supported. Need to check the value of default samplerate.
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
            qWarning() << "The current recording encoder does not support the "
                          "previous recording samplerate of"
                       << m_oldRecSampleRate << "Hz";
        }
    }
    setupEncoderUI();
}

// Update the set of custom sample rates offered in the recording combobox.
// Called on 2 occasions during a Mixxx run:
// 1. From a slot, whenever the engine samplerate is updated.
// 2. From a slot, whenever the recording format is changed.
void DlgPrefRecord::updateSampleRates(
        const QList<mixxx::audio::SampleRate>& sampleRates) {
    comboBoxRecSampleRate->blockSignals(true);

    comboBoxRecSampleRate->clear();
    for (const auto& sampleRate : sampleRates) {
        if (sampleRate.isValid()) {
            comboBoxRecSampleRate->addItem(tr("%1 Hz").arg(sampleRate.value()),
                    QVariant::fromValue(sampleRate.value()));
        }
    }
    comboBoxRecSampleRate->blockSignals(false);
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
        return;
    }
    // addItem() is storing samplerate double values.
    auto recSampleRateNew = comboBoxRecSampleRate->itemData(newRateIdx).value<double>();

    qDebug() << "Rec samplerate change triggered, new rate: " << recSampleRateNew;
    m_useEngineSampleRate.set(false);
}

const QList<mixxx::audio::SampleRate>&
DlgPrefRecord::createSampleRateGUIForFormat(const QString& recFormat) {
    const QList<mixxx::audio::SampleRate>& validSampleRates =
            (recFormat == ENCODING_OPUS)  ? kRecSampleRatesOpus
            : (recFormat == ENCODING_MP3) ? kRecSampleRatesMP3
            : (recFormat == ENCODING_OGG) ? kRecSampleRatesOGG
                                          : kRecSampleRates; // default
    updateSampleRates(validSampleRates);
    return validSampleRates;
}

void DlgPrefRecord::slotDefaultSampleRateUpdated(mixxx::audio::SampleRate newRate) {
    RadioButtonUseDefaultSampleRate->setText(QString("Default (%1 Hz)").arg(newRate.toDouble()));
    double rate = newRate.toDouble();
    m_defaultSampleRate = rate;

    bool enableRadioButtonDefault = true;
    double oldSampleRate = m_oldRecSampleRate;

    // recreate the available custom samplerates in the GUI
    const auto& customSampleRatesList = createSampleRateGUIForFormat(m_selFormat.internalName);

    // If the current format allows the value of
    // new engine samplerate (i.e. default rec samplerate),
    // allow the user to choose it.
    enableRadioButtonDefault = customSampleRatesList.contains(newRate);
    RadioButtonUseDefaultSampleRate->setEnabled(enableRadioButtonDefault);

    // Ensure that the previous recording samplerate remains unchanged
    // even if the engine samplerate has changed.
    int recSampleRateIdx = comboBoxRecSampleRate->findData(
            QVariant::fromValue(oldSampleRate));
    if (recSampleRateIdx != -1) { // found in the combobox
        RadioButtonUseDefaultSampleRate->setChecked(false); // calls slot ignore
        comboBoxRecSampleRate->setCurrentIndex(recSampleRateIdx);
    } else {
        // If the updated GUI list is missing the old rec samplerate,
        // that samplerate is the new default, i.e. engine samplerate.
        // select the default radio button instead.
        // However it could also be the case that even the default samplerate
        // is not supported. Need to check the value of default samplerate.
        DEBUG_ASSERT(m_defaultSampleRate == newRate.value());

        auto defaultRate = mixxx::audio::SampleRate(static_cast<unsigned>(m_defaultSampleRate));
        bool isDefaultSampleRateSupported = customSampleRatesList.contains(defaultRate);

        RadioButtonUseCustomSampleRate->setChecked(!isDefaultSampleRateSupported);
        // calls slottogglecustomsamplerateignore
        RadioButtonUseDefaultSampleRate->setChecked(isDefaultSampleRateSupported);
    }
}
