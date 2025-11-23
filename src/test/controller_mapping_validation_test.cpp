#include "test/controller_mapping_validation_test.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <qapplication.h>
#include <qglobal.h>

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

void MappingFixture::SetUp() {
#ifdef MIXXX_USE_QML
    // This setup mirrors coreservices -- it would be nice if we could use coreservices instead
    // but it does a lot of local disk / settings setup.
    auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
    m_pEffectsManager = std::make_shared<EffectsManager>(m_pConfig, pChannelHandleFactory);
    m_pEngine = std::make_shared<EngineMixer>(
            m_pConfig,
            "[Master]",
            m_pEffectsManager.get(),
            pChannelHandleFactory,
            true);
    m_pSoundManager = std::make_shared<SoundManager>(m_pConfig, m_pEngine.get());
    m_pControlIndicatorTimer = std::make_shared<mixxx::ControlIndicatorTimer>(nullptr);
    m_pEngine->registerNonEngineChannelSoundIO(gsl::make_not_null(m_pSoundManager.get()));
    m_pPlayerManager = std::make_shared<PlayerManager>(m_pConfig,
            m_pSoundManager.get(),
            m_pEffectsManager.get(),
            m_pEngine.get());

    m_pPlayerManager->addConfiguredDecks();
    m_pPlayerManager->addSampler();
    PlayerInfo::create();
    m_pEffectsManager->setup();

    const auto dbConnection = mixxx::DbConnectionPooled(dbConnectionPooler());
    if (!MixxxDb::initDatabaseSchema(dbConnection)) {
        exit(1);
    }
    m_pTrackCollectionManager = std::make_shared<TrackCollectionManager>(
            nullptr,
            m_pConfig,
            dbConnectionPooler(),
            deleteTrack);

    m_pRecordingManager = std::make_shared<RecordingManager>(m_pConfig, m_pEngine.get());
    CoverArtCache::createInstance();
    m_pLibrary = std::make_shared<Library>(
            nullptr,
            m_pConfig,
            dbConnectionPooler(),
            m_pTrackCollectionManager.get(),
            m_pPlayerManager.get(),
            m_pRecordingManager.get());

    m_pPlayerManager->bindToLibrary(m_pLibrary.get());
    mixxx::qml::QmlPlayerManagerProxy::registerPlayerManager(m_pPlayerManager);
    ControllerScriptEngineBase::registerTrackCollectionManager(m_pTrackCollectionManager);
}

void MappingFixture::TearDown() {
    PlayerInfo::destroy();
    CoverArtCache::destroy();
    mixxx::qml::QmlPlayerManagerProxy::registerPlayerManager(nullptr);
    ControllerScriptEngineBase::registerTrackCollectionManager(nullptr);
#endif
}

bool MappingFixture::testLoadMapping(const QString& mappingPath) {
    std::shared_ptr<LegacyControllerMapping> pMapping =
            LegacyControllerMappingFileHandler::loadMapping(
                    QFileInfo(mappingPath), QDir(RESOURCE_FOLDER "/controllers"));
    if (!pMapping) {
        return false;
    }

    FakeController controller;
    controller.setMapping(pMapping);
    bool result = controller.applyMapping(getTestDir().filePath(RESOURCE_FOLDER));
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

std::string PrintMappingName(const ::testing::TestParamInfo<std::string>& info) {
    auto name = QFileInfo(QString::fromStdString(info.param));
    return name.fileName().replace(QRegularExpression("[^\\w]+"), "_").toStdString();
}

// Inhibit the output of the mapping info to avoid spamming the console
// std::ostream& operator<<(std::ostream& os, const QString& MappingPath) {
//     return os << MappingPath.toStdString();
// }

TEST_P(MappingFixture, ValidateMappingXML) {
    const auto& mappingPath = QString::fromStdString(GetParam());
    qDebug() << "ValidateMappingXML " << mappingPath;
    std::string errorDescription = "Error while validating XML file " +
            GetParam();
    const auto mapping = MappingInfo(mappingPath);
    EXPECT_TRUE(mapping.isValid()) << errorDescription;
    EXPECT_TRUE(lintMappingInfo(mapping)) << errorDescription;
}

TEST_P(MappingFixture, LoadMapping) {
    const auto& mappingPath = QString::fromStdString(GetParam());
    qDebug() << "LoadMapping " << mappingPath;
    std::string errorDescription = "Error while loading " + mappingPath.toStdString();
    EXPECT_TRUE(testLoadMapping(mappingPath)) << errorDescription;
}

// #ifdef __HID__
// INSTANTIATE_TEST_SUITE_P(HidMappings,
//         MappingFixture,
//         ::testing::ValuesIn(
//                 findMappings(HID_MAPPING_EXTENSION)),
//         PrintMappingName);
// #endif

INSTANTIATE_TEST_SUITE_P(MidiMappings,
        MappingFixture,
        ::testing::Values(CONTROLLER_MAPPING_MIDI),
        PrintMappingName);

// #ifdef __BULK__
// INSTANTIATE_TEST_SUITE_P(BulkMappings,
//         MappingFixture,
//         ::testing::Values(
//                 ,
//         PrintMappingName);
// #endif
