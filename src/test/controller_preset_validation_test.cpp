#include <gtest/gtest.h>

#include <QDir>
#include <QUrl>
#include <QScopedPointer>
#include <QtDebug>

#include "controllers/controller.h"
#include "controllers/controllerpresetfilehandler.h"
#include "controllers/controllerpresetinfoenumerator.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/hid/hidcontrollerpreset.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "controllers/defs_controllers.h"
#include "test/mixxxtest.h"

class FakeController : public Controller {
  public:
    FakeController();
    ~FakeController() override;

    QString presetExtension() override {
        // Doesn't affect anything at the moment.
        return ".test.xml";
    }

    ControllerPresetPointer getPreset() const override {
        if (m_bHidPreset) {
            HidControllerPreset* pClone = new HidControllerPreset();
            *pClone = m_hidPreset;
            return ControllerPresetPointer(pClone);
        } else if (m_bKbdPreset) {
            KeyboardControllerPreset* pClone = new KeyboardControllerPreset();
            *pClone = m_kbdPreset;
            return ControllerPresetPointer(pClone);
        } else {
            // Default to Midi
            MidiControllerPreset* pClone = new MidiControllerPreset();
            *pClone = m_midiPreset;
            return ControllerPresetPointer(pClone);
        }
    }

    bool savePreset(const QString fileName) const override {
        Q_UNUSED(fileName);
        return true;
    }

    void visitMidi(const MidiControllerPreset* preset) override {
        m_bMidiPreset = true;
        m_bHidPreset = false;
        m_bKbdPreset = false;
        m_midiPreset = *preset;
        m_hidPreset = HidControllerPreset();
        m_kbdPreset = KeyboardControllerPreset();
    }

    void visitHid(const HidControllerPreset* preset) override {
        m_bMidiPreset = false;
        m_bHidPreset = true;
        m_bKbdPreset = false;
        m_midiPreset = MidiControllerPreset();
        m_hidPreset = *preset;
        m_kbdPreset = KeyboardControllerPreset();
    }

    void visitKeyboard(const KeyboardControllerPreset *preset) override {
        m_bMidiPreset = false;
        m_bHidPreset = false;
        m_bKbdPreset = true;
        m_midiPreset = MidiControllerPreset();
        m_hidPreset = HidControllerPreset();
        m_kbdPreset = *preset;
    }

    void accept(ControllerVisitor* visitor) override {
        // Do nothing since we aren't a normal controller.
        Q_UNUSED(visitor);
    }

    bool isMappable() const override {
        if (m_bMidiPreset) {
            return m_midiPreset.isMappable();
        } else if (m_bHidPreset) {
            return m_hidPreset.isMappable();
        } else if (m_bKbdPreset) {
            return m_kbdPreset.isMappable();
        }
        return false;
    }

    bool matchPreset(const PresetInfo& preset) override {
        // We're not testing product info matching in this test.
        Q_UNUSED(preset);
        return false;
    }

  protected:
    Q_INVOKABLE void send(QList<int> data, unsigned int length, unsigned int reportID) {
        Q_UNUSED(data);
        Q_UNUSED(length);
        Q_UNUSED(reportID);
    }

  private slots:
    int open() override {
        return 0;
    }
    int close() override {
        return 0;
    }

  private:
    void send(QByteArray data) override {
        Q_UNUSED(data);
    }
    void send(QByteArray data, unsigned int reportID) {
        Q_UNUSED(data);
        Q_UNUSED(reportID);
    }

    ControllerPreset* preset() override {
        if (m_bHidPreset) {
            return &m_hidPreset;
        } else if (m_bKbdPreset) {
            return &m_kbdPreset;
        } else {
            // Default to MIDI.
            return &m_midiPreset;
        }
    }

    bool m_bMidiPreset;
    bool m_bHidPreset;
    bool m_bKbdPreset;
    MidiControllerPreset m_midiPreset;
    HidControllerPreset m_hidPreset;
    KeyboardControllerPreset m_kbdPreset;
};

FakeController::FakeController()
        : m_bMidiPreset(false),
          m_bHidPreset(false),
          m_bKbdPreset(false) {
}

FakeController::~FakeController() {
}

class ControllerPresetValidationTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_presetPaths << QDir::currentPath() + "/res/controllers";
        m_pEnumerator.reset(new PresetInfoEnumerator(m_presetPaths));
    }

    bool testLoadPreset(const PresetInfo& preset) {
        ControllerPresetPointer pPreset =
                ControllerPresetFileHandler::loadPreset(preset.getPath(),
                                                        m_presetPaths);
        if (pPreset.isNull()) {
            return false;
        }

        FakeController controller;
        controller.setDeviceName("Test Controller");
        controller.startEngine();
        controller.setPreset(*pPreset);
        // Do not initialize the scripts.
        bool result = controller.applyPreset(m_presetPaths, false);
        controller.stopEngine();
        return result;
    }


    QStringList m_presetPaths;
    QScopedPointer<PresetInfoEnumerator> m_pEnumerator;
};

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
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}

TEST_F(ControllerPresetValidationTest, HidPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresetsByExtension(HID_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}

TEST_F(ControllerPresetValidationTest, BulkPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresetsByExtension(BULK_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}

TEST_F(ControllerPresetValidationTest, KbdPresetsValid) {
    for (const PresetInfo &preset:
            m_pEnumerator->getPresetsByExtension(KEYBOARD_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}
