#include "test/controller_mapping_validation_test.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QRegularExpression>
#include <QUrl>

#include "controllers/defs_controllers.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#ifdef MIXXX_USE_QML
#include "effects/effectsmanager.h"
#include "engine/channelhandle.h"
#include "engine/enginemixer.h"
#include "library/coverartcache.h"
#include "library/library.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "qml/qmlplayermanagerproxy.h"
#include "soundio/soundmanager.h"
#endif
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

#ifdef MIXXX_USE_QML
void deleteTrack(Track* pTrack) {
    // Delete track objects directly in unit tests with
    // no main event loop
    delete pTrack;
};
#endif

void LegacyControllerMappingValidationTest::SetUp() {
#ifdef MIXXX_USE_QML
    // This setup mirrors coreservices -- it would be nice if we could use coreservices instead
    // but it does a lot of local disk / settings setup.
    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();

#endif
}

void LegacyControllerMappingValidationTest::TearDown() {
#ifdef MIXXX_USE_QML

#endif
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
    bool result = controller.applyMapping(getTestDir().filePath(QStringLiteral("../../res")));
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

// Create a mapping enumerator for the test, which will be used to get the mappings
// for the test cases
std::shared_ptr<LegacyControllerMappingList> createMappingEnumerator() {
    return std::make_shared<LegacyControllerMappingList>();
}

// Inhibit the output of the mapping info to avoid spamming the console
std::ostream& operator<<(std::ostream& os, const MappingInfo& mapping) {
    Q_UNUSED(mapping);
    os << "<MappingInfo>";
    return os;
}

// Print the mapping name instead with underscores instead of spaces and special characters
class PrintMappingName {
  public:
    template<class ParamType>
    std::string operator()(const ::testing::TestParamInfo<ParamType>& info) const {
        // ParamType is MappingInfo
        QString nameCopy = info.param.getName();
        static const QRegularExpression regex("[^\\w\\s]");
        nameCopy.replace(regex, "_");
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

#ifdef __HID__
INSTANTIATE_TEST_SUITE_P(HidMappingsValid,
        LegacyControllerMappingValidationTest,
        ::testing::ValuesIn(
                pEnumerator->getMappingEnumerator()->getMappingsByExtension(
                        HID_MAPPING_EXTENSION)),
        PrintMappingName());
#endif

#ifdef __BULK__
INSTANTIATE_TEST_SUITE_P(BulkMappingsValid,
        LegacyControllerMappingValidationTest,
        ::testing::ValuesIn(
                pEnumerator->getMappingEnumerator()->getMappingsByExtension(
                        BULK_MAPPING_EXTENSION)),
        PrintMappingName());
#endif

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
