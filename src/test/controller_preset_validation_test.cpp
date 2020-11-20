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
#include "controllers/defs_controllers.h"
#include "test/mixxxtest.h"

class FakeController : public Controller {
  public:
    explicit FakeController(UserSettingsPointer pConfig);
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
        } else {
            MidiControllerPreset* pClone = new MidiControllerPreset();
            *pClone = m_midiPreset;
            return ControllerPresetPointer(pClone);
        }
    }

    void visit(const MidiControllerPreset* preset) override {
        m_bMidiPreset = true;
        m_bHidPreset = false;
        m_midiPreset = *preset;
        m_hidPreset = HidControllerPreset();
    }
    void visit(const HidControllerPreset* preset) override {
        m_bMidiPreset = false;
        m_bHidPreset = true;
        m_midiPreset = MidiControllerPreset();
        m_hidPreset = *preset;
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
        }
        return false;
    }

    bool matchPreset(const PresetInfo& preset) override {
        // We're not testing product info matching in this test.
        Q_UNUSED(preset);
        return false;
    }

  protected:
    Q_INVOKABLE void send(const QList<int>& data, unsigned int length, unsigned int reportID) {
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
    void send(const QByteArray& data) override {
        Q_UNUSED(data);
    }
    virtual void send(const QByteArray& data, unsigned int reportID) {
        Q_UNUSED(data);
        Q_UNUSED(reportID);
    }

    ControllerPreset* preset() override {
        if (m_bHidPreset) {
            return &m_hidPreset;
        } else {
            // Default to MIDI.
            return &m_midiPreset;
        }
    }

    bool m_bMidiPreset;
    bool m_bHidPreset;
    MidiControllerPreset m_midiPreset;
    HidControllerPreset m_hidPreset;
};

FakeController::FakeController(UserSettingsPointer pConfig)
        : Controller(pConfig), m_bMidiPreset(false), m_bHidPreset(false) {
}

FakeController::~FakeController() {
}

class ControllerPresetValidationTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_presetPath = QDir::current();
        m_presetPath.cd("res/controllers");
        m_pEnumerator.reset(new PresetInfoEnumerator(QList<QString>{m_presetPath.absolutePath()}));
    }

    bool testLoadPreset(const PresetInfo& preset) {
        ControllerPresetPointer pPreset =
                ControllerPresetFileHandler::loadPreset(
                        preset.getPath(),
                        m_presetPath);
        if (pPreset.isNull()) {
            return false;
        }

        FakeController controller(config());
        controller.setDeviceName("Test Controller");
        controller.startEngine();
        controller.setPreset(*pPreset);
        // Do not initialize the scripts.
        bool result = controller.applyPreset(false);
        controller.stopEngine();
        return result;
    }

    QDir m_presetPath;
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
