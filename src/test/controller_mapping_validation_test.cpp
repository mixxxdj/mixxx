#include "test/controller_mapping_validation_test.h"

#include <QRegularExpression>
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

bool LegacyControllerMappingValidationTest::testLoadMapping(
        const MappingInfo& mapping, const QDir& systemMappingsPath) {
    std::shared_ptr<LegacyControllerMapping> pMapping =
            LegacyControllerMappingFileHandler::loadMapping(
                    QFileInfo(mapping.getPath()), systemMappingsPath);
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

std::shared_ptr<LegacyControllerMappingList> createMappingEnumerator() {
    return std::make_shared<LegacyControllerMappingList>();
}

class PrintMappingName {
  public:
    template<class ParamType>
    std::string operator()(const ::testing::TestParamInfo<ParamType>& info) const {
        // ParamType is MappingInfo
        QString nameCopy = info.param.getName();
        nameCopy.replace(QRegularExpression("[^\\w\\s]"), "_");
        std::string testName = nameCopy.replace(" ", "_").toStdString();
        return testName;
    }
};

auto pEnumerator = createMappingEnumerator();

INSTANTIATE_TEST_SUITE_P(MidiMappingsValid,
        LegacyControllerMappingValidationTest,
        ::testing::ValuesIn(
                pEnumerator->getMappingEnumerator()->getMappingsByExtension(
                        MIDI_MAPPING_EXTENSION)),
        PrintMappingName());

INSTANTIATE_TEST_SUITE_P(HidMappingsValid,
        LegacyControllerMappingValidationTest,
        ::testing::ValuesIn(
                pEnumerator->getMappingEnumerator()->getMappingsByExtension(
                        HID_MAPPING_EXTENSION)),
        PrintMappingName());

INSTANTIATE_TEST_SUITE_P(BulkMappingsValid,
        LegacyControllerMappingValidationTest,
        ::testing::ValuesIn(
                pEnumerator->getMappingEnumerator()->getMappingsByExtension(
                        BULK_MAPPING_EXTENSION)),
        PrintMappingName());

TEST_P(LegacyControllerMappingValidationTest, ValidateMappingXML) {
    const MappingInfo& mapping = GetParam();
    qDebug() << "ValidateMappingXML " << mapping.getPath();
    std::string errorDescription = "Error while validating XML file " +
            mapping.getPath().toStdString();
    EXPECT_TRUE(mapping.isValid()) << errorDescription;
    EXPECT_TRUE(lintMappingInfo(mapping)) << errorDescription;
}

TEST_P(LegacyControllerMappingValidationTest, LoadMapping) {
    const MappingInfo& mapping = GetParam();
    qDebug() << "LoadMapping " << mapping.getPath();
    std::string errorDescription = "Error while loading " + mapping.getPath().toStdString();
    EXPECT_TRUE(mapping.isValid()) << errorDescription;
    EXPECT_TRUE(testLoadMapping(mapping, pEnumerator->getMappingPath())) << errorDescription;
}
