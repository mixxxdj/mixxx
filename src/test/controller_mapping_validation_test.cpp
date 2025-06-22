#include "test/controller_mapping_validation_test.h"

#include <QUrl>

#include "controllers/defs_controllers.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "moc_controller_mapping_validation_test.cpp"

FakeMidiControllerJSProxy::FakeMidiControllerJSProxy()
        : ControllerJSProxy(nullptr) {
}

void FakeMidiControllerJSProxy::send(const QList<int>& data, unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);
    qInfo() << "LINT: Prefer to use sendSysexMsg instead of the generic alias send!";
}

void FakeMidiControllerJSProxy::sendSysexMsg(const QList<int>& data, unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);
}

void FakeMidiControllerJSProxy::sendShortMsg(unsigned char status,
        unsigned char byte1,
        unsigned char byte2) {
    Q_UNUSED(status);
    Q_UNUSED(byte1);
    Q_UNUSED(byte2);
}

FakeHidControllerJSProxy::FakeHidControllerJSProxy()
        : ControllerJSProxy(nullptr) {
}

void FakeHidControllerJSProxy::send(const QList<int>& data, unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);

    qInfo() << "LINT: Prefer to use sendOutputReport instead of send!";
}

void FakeHidControllerJSProxy::send(const QList<int>& dataList,
        unsigned int length,
        quint8 reportID,
        bool useNonSkippingFIFO) {
    Q_UNUSED(dataList);
    Q_UNUSED(length);
    Q_UNUSED(reportID);
    Q_UNUSED(useNonSkippingFIFO);

    qInfo() << "LINT: Prefer to use sendOutputReport instead of send!";
}

void FakeHidControllerJSProxy::sendOutputReport(quint8 reportID,
        const QByteArray& dataArray,
        bool resendUnchangedReport) {
    Q_UNUSED(reportID);
    Q_UNUSED(dataArray);
    Q_UNUSED(resendUnchangedReport);
}

QByteArray FakeHidControllerJSProxy::getInputReport(
        quint8 reportID) {
    Q_UNUSED(reportID);
    return QByteArray();
}

void FakeHidControllerJSProxy::sendFeatureReport(
        quint8 reportID, const QByteArray& reportData) {
    Q_UNUSED(reportID);
    Q_UNUSED(reportData);
}

QByteArray FakeHidControllerJSProxy::getFeatureReport(
        quint8 reportID) {
    Q_UNUSED(reportID);
    return QByteArray();
}

FakeBulkControllerJSProxy::FakeBulkControllerJSProxy()
        : ControllerJSProxy(nullptr) {
}

void FakeBulkControllerJSProxy::send(const QList<int>& data, unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);
}

FakeController::FakeController()
        : Controller("Test Controller"),
          m_bMidiMapping(false),
          m_bHidMapping(false) {
    startEngine();
    getScriptEngine()->setTesting(true);
}

FakeController::~FakeController() {
}

bool FakeController::isMappable() const {
    if (m_bMidiMapping) {
        return m_pMidiMapping->isMappable();
    } else if (m_bHidMapping) {
        return m_pHidMapping->isMappable();
    }
    return false;
}

void LegacyControllerMappingValidationTest::SetUp() {
    m_mappingPath = getTestDir().filePath(QStringLiteral("../../res/controllers/"));
    m_pEnumerator.reset(new MappingInfoEnumerator(QList<QString>{m_mappingPath.absolutePath()}));
}

bool LegacyControllerMappingValidationTest::testLoadMapping(const MappingInfo& mapping) {
    std::shared_ptr<LegacyControllerMapping> pMapping =
            LegacyControllerMappingFileHandler::loadMapping(
                    QFileInfo(mapping.getPath()), m_mappingPath);
    if (!pMapping) {
        return false;
    }

    FakeController controller;
    controller.setMapping(pMapping);
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

#ifdef __HID__
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
#endif

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
