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
    FakeController();
    virtual ~FakeController();

    virtual QString presetExtension() {
        // Doesn't affect anything at the moment.
        return ".test.xml";
    }

    virtual ControllerPresetPointer getPreset() const {
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

    virtual bool savePreset(const QString fileName) const {
        Q_UNUSED(fileName);
        return true;
    }

    virtual void visit(const MidiControllerPreset* preset) {
        m_bMidiPreset = true;
        m_bHidPreset = false;
        m_midiPreset = *preset;
        m_hidPreset = HidControllerPreset();
    }
    virtual void visit(const HidControllerPreset* preset) {
        m_bMidiPreset = false;
        m_bHidPreset = true;
        m_midiPreset = MidiControllerPreset();
        m_hidPreset = *preset;
    }

    virtual void accept(ControllerVisitor* visitor) {
        // Do nothing since we aren't a normal controller.
        Q_UNUSED(visitor);
    }

    virtual bool isMappable() const {
        if (m_bMidiPreset) {
            return m_midiPreset.isMappable();
        } else if (m_bHidPreset) {
            return m_hidPreset.isMappable();
        }
        return false;
    }

    virtual bool matchPreset(const PresetInfo& preset) {
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
    int open() {
        return 0;
    }
    int close() {
        return 0;
    }

  private:
    virtual void send(QByteArray data) {
        Q_UNUSED(data);
    }
    virtual void send(QByteArray data, unsigned int reportID) {
        Q_UNUSED(data);
        Q_UNUSED(reportID);
    }
    virtual bool isPolling() const {
        return false;
    }
    virtual ControllerPreset* preset() {
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

FakeController::FakeController()
        : m_bMidiPreset(false),
          m_bHidPreset(false) {
}

FakeController::~FakeController() {
}

class ControllerPresetValidationTest : public MixxxTest {
  protected:
    virtual void SetUp() {
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
             m_pEnumerator->getPresets(MIDI_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}

TEST_F(ControllerPresetValidationTest, HidPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresets(HID_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}

TEST_F(ControllerPresetValidationTest, BulkPresetsValid) {
    foreach (const PresetInfo& preset,
             m_pEnumerator->getPresets(BULK_PRESET_EXTENSION)) {
        qDebug() << "Validating" << preset.getPath();
        EXPECT_TRUE(preset.isValid());
        EXPECT_TRUE(lintPresetInfo(preset));
        EXPECT_TRUE(testLoadPreset(preset));
    }
}
