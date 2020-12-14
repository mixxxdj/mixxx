#include "test/controller_preset_validation_test.h"

#include <QUrl>

#include "controllers/defs_controllers.h"

FakeControllerJSProxy::FakeControllerJSProxy()
        : ControllerJSProxy(nullptr) {
}

void FakeControllerJSProxy::send(const QList<int>& data, unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);
}

void FakeControllerJSProxy::sendSysexMsg(const QList<int>& data, unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);
}

void FakeControllerJSProxy::sendShortMsg(unsigned char status,
        unsigned char byte1,
        unsigned char byte2) {
    Q_UNUSED(status);
    Q_UNUSED(byte1);
    Q_UNUSED(byte2);
}

FakeController::FakeController()
        : m_bMidiPreset(false),
          m_bHidPreset(false) {
    startEngine();
    getEngine()->setTesting(true);
}

FakeController::~FakeController() {
}

ControllerPresetPointer FakeController::getPreset() const {
    if (m_bHidPreset) {
        HidControllerPreset* pClone = new HidControllerPreset();
        *pClone = m_hidPreset;
        return ControllerPresetPointer(pClone);
    } else {
        MidiControllerPreset* pClone = new MidiControllerPreset();
        *pClone = m_midiPreset;
        return ControllerPresetPointer(pClone);
    }
}

bool FakeController::isMappable() const {
    if (m_bMidiPreset) {
        return m_midiPreset.isMappable();
    } else if (m_bHidPreset) {
        return m_hidPreset.isMappable();
    }
    return false;
}

ControllerPreset* FakeController::preset() {
    if (m_bHidPreset) {
        return &m_hidPreset;
    } else {
        // Default to MIDI.
        return &m_midiPreset;
    }
}

void ControllerPresetValidationTest::SetUp() {
    m_presetPath = QDir::current();
    m_presetPath.cd("res/controllers");
    m_pEnumerator.reset(new PresetInfoEnumerator(QList<QString>{m_presetPath.absolutePath()}));
}

bool ControllerPresetValidationTest::testLoadPreset(const PresetInfo& preset) {
    ControllerPresetPointer pPreset =
            ControllerPresetFileHandler::loadPreset(preset.getPath(), m_presetPath);
    if (pPreset.isNull()) {
        return false;
    }

    FakeController controller;
    controller.setDeviceName("Test Controller");
    controller.setPreset(*pPreset);
    // Do not initialize the scripts.
    bool result = controller.applyPreset(false);
    controller.stopEngine();
    return result;
}

bool checkUrl(const QString& url) {
    return QUrl(url).isValid();
}

bool lintPresetInfo(const PresetInfo& preset) {
    bool result = true;
    if (preset.getName().trimmed().isEmpty()) {
        qWarning() << "LINT:" << preset.getPath() << "has no name.";
        result = false;
    }

    if (preset.getAuthor().trimmed().isEmpty()) {
        qWarning() << "LINT:" << preset.getPath() << "has no author.";
    }

    if (preset.getDescription().trimmed().isEmpty()) {
        qWarning() << "LINT:" << preset.getPath() << "has no description.";
    }

    if (preset.getForumLink().trimmed().isEmpty()) {
        qWarning() << "LINT:" << preset.getPath() << "has no forum link.";
    } else if (!checkUrl(preset.getForumLink())) {
        qWarning() << "LINT:" << preset.getPath() << "has invalid forum link";
        result = false;
    }

    if (preset.getWikiLink().trimmed().isEmpty()) {
        qWarning() << "LINT:" << preset.getPath() << "has no wiki link.";
    } else if (!checkUrl(preset.getWikiLink())) {
        qWarning() << "LINT:" << preset.getPath() << "has invalid wiki link";
        result = false;
    }
    return result;
}

TEST_F(ControllerPresetValidationTest, MidiPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresetsByExtension(MIDI_PRESET_EXTENSION)) {
        qDebug() << "Validating " << preset.getPath();
        std::string errorDescription = "Error while validating " + preset.getPath().toStdString();
        EXPECT_TRUE(preset.isValid()) << errorDescription;
        EXPECT_TRUE(lintPresetInfo(preset)) << errorDescription;
        EXPECT_TRUE(testLoadPreset(preset)) << errorDescription;
    }
}

TEST_F(ControllerPresetValidationTest, HidPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresetsByExtension(HID_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        std::string errorDescription = "Error while validating " + preset.getPath().toStdString();
        EXPECT_TRUE(preset.isValid()) << errorDescription;
        EXPECT_TRUE(lintPresetInfo(preset)) << errorDescription;
        EXPECT_TRUE(testLoadPreset(preset)) << errorDescription;
    }
}

TEST_F(ControllerPresetValidationTest, BulkPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresetsByExtension(BULK_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        std::string errorDescription = "Error while validating " + preset.getPath().toStdString();
        EXPECT_TRUE(preset.isValid()) << errorDescription;
        EXPECT_TRUE(lintPresetInfo(preset)) << errorDescription;
        EXPECT_TRUE(testLoadPreset(preset)) << errorDescription;
    }
}
