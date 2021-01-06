#include "test/controller_mapping_validation_test.h"

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
        : m_bMidiMapping(false),
          m_bHidMapping(false) {
    startEngine();
    getScriptEngine()->setTesting(true);
}

FakeController::~FakeController() {
}

LegacyControllerMappingPointer FakeController::getMapping() const {
    if (m_bHidMapping) {
        LegacyHidControllerMapping* pClone = new LegacyHidControllerMapping();
        *pClone = m_hidMapping;
        return LegacyControllerMappingPointer(pClone);
    } else {
        LegacyMidiControllerMapping* pClone = new LegacyMidiControllerMapping();
        *pClone = m_midiMapping;
        return LegacyControllerMappingPointer(pClone);
    }
}

bool FakeController::isMappable() const {
    if (m_bMidiMapping) {
        return m_midiMapping.isMappable();
    } else if (m_bHidMapping) {
        return m_hidMapping.isMappable();
    }
    return false;
}

LegacyControllerMapping* FakeController::mapping() {
    if (m_bHidMapping) {
        return &m_hidMapping;
    } else {
        // Default to MIDI.
        return &m_midiMapping;
    }
}

void LegacyControllerMappingValidationTest::SetUp() {
    m_mappingPath = QDir::current();
    m_mappingPath.cd("res/controllers");
    m_pEnumerator.reset(new MappingInfoEnumerator(QList<QString>{m_mappingPath.absolutePath()}));
}

bool LegacyControllerMappingValidationTest::testLoadMapping(const MappingInfo& mapping) {
    LegacyControllerMappingPointer pMapping =
            LegacyControllerMappingFileHandler::loadMapping(mapping.getPath(), m_mappingPath);
    if (pMapping.isNull()) {
        return false;
    }

    FakeController controller;
    controller.setDeviceName("Test Controller");
    controller.setMapping(*pMapping);
    bool result = controller.applyMapping();
    controller.stopEngine();
    return result;
}

bool checkUrl(const QString& url) {
    return QUrl(url).isValid();
}

bool lintMappingInfo(const MappingInfo& mapping) {
    bool result = true;
    if (mapping.getName().trimmed().isEmpty()) {
        qWarning() << "LINT:" << mapping.getPath() << "has no name.";
        result = false;
    }

    if (mapping.getAuthor().trimmed().isEmpty()) {
        qWarning() << "LINT:" << mapping.getPath() << "has no author.";
    }

    if (mapping.getDescription().trimmed().isEmpty()) {
        qWarning() << "LINT:" << mapping.getPath() << "has no description.";
    }

    if (mapping.getForumLink().trimmed().isEmpty()) {
        qWarning() << "LINT:" << mapping.getPath() << "has no forum link.";
    } else if (!checkUrl(mapping.getForumLink())) {
        qWarning() << "LINT:" << mapping.getPath() << "has invalid forum link";
        result = false;
    }

    if (mapping.getWikiLink().trimmed().isEmpty()) {
        qWarning() << "LINT:" << mapping.getPath() << "has no wiki link.";
    } else if (!checkUrl(mapping.getWikiLink())) {
        qWarning() << "LINT:" << mapping.getPath() << "has invalid wiki link";
        result = false;
    }
    return result;
}

TEST_F(LegacyControllerMappingValidationTest, MidiMappingsValid) {
    foreach (const MappingInfo& mapping,
            m_pEnumerator->getMappingsByExtension(MIDI_MAPPING_EXTENSION)) {
        qDebug() << "Validating " << mapping.getPath();
        std::string errorDescription = "Error while validating " + mapping.getPath().toStdString();
        EXPECT_TRUE(mapping.isValid()) << errorDescription;
        EXPECT_TRUE(lintMappingInfo(mapping)) << errorDescription;
        EXPECT_TRUE(testLoadMapping(mapping)) << errorDescription;
    }
}

TEST_F(LegacyControllerMappingValidationTest, HidMappingsValid) {
    foreach (const MappingInfo& mapping,
            m_pEnumerator->getMappingsByExtension(HID_MAPPING_EXTENSION)) {
        qDebug() << "Validating" << mapping.getPath();
        std::string errorDescription = "Error while validating " + mapping.getPath().toStdString();
        EXPECT_TRUE(mapping.isValid()) << errorDescription;
        EXPECT_TRUE(lintMappingInfo(mapping)) << errorDescription;
        EXPECT_TRUE(testLoadMapping(mapping)) << errorDescription;
    }
}

TEST_F(LegacyControllerMappingValidationTest, BulkMappingsValid) {
    foreach (const MappingInfo& mapping,
            m_pEnumerator->getMappingsByExtension(BULK_MAPPING_EXTENSION)) {
        qDebug() << "Validating" << mapping.getPath();
        std::string errorDescription = "Error while validating " + mapping.getPath().toStdString();
        EXPECT_TRUE(mapping.isValid()) << errorDescription;
        EXPECT_TRUE(lintMappingInfo(mapping)) << errorDescription;
        EXPECT_TRUE(testLoadMapping(mapping)) << errorDescription;
    }
}
