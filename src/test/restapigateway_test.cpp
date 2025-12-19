#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QJsonArray>
#include <QJsonDocument>

#include "control/controlindicatortimer.h"
#include "database/mixxxdb.h"
#include "effects/effectsmanager.h"
#include "engine/channelhandle.h"
#include "engine/enginemixer.h"
#include "library/coverartcache.h"
#include "library/dao/playlistdao.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "network/rest/restapigateway.h"
#include "soundio/soundmanager.h"
#include "test/mixxxdbtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/track.h"

namespace {

const QString kTrackLocationTest1 = QStringLiteral("id3-test-data/cover-test-png.mp3");

void deleteTrack(Track* pTrack) {
    // Delete track objects directly in unit tests with
    // no main event loop
    delete pTrack;
}

class RestApiGatewayTest : public MixxxDbTest, SoundSourceProviderRegistration {
  public:
    RestApiGatewayTest()
            : MixxxDbTest(true) {
    }

    void SetUp() override {
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

        CoverArtCache::createInstance();

        m_pPlayerManager = std::make_shared<PlayerManager>(m_pConfig,
                m_pSoundManager.get(),
                m_pEffectsManager.get(),
                m_pEngine.get());

        const auto dbConnection = mixxx::DbConnectionPooled(dbConnectionPooler());
        ASSERT_TRUE(MixxxDb::initDatabaseSchema(dbConnection));
        m_pTrackCollectionManager = std::make_unique<TrackCollectionManager>(
                nullptr,
                m_pConfig,
                dbConnectionPooler(),
                deleteTrack);

        m_pGateway = std::make_unique<mixxx::network::rest::RestApiGateway>(
                m_pPlayerManager.get(),
                m_pTrackCollectionManager.get(),
                m_pConfig);
    }

    void TearDown() override {
        m_pGateway.reset();
        m_pPlayerManager.reset();
        m_pTrackCollectionManager.reset();
        m_pSoundManager.reset();
        m_pEngine.reset();
        m_pEffectsManager.reset();
        m_pControlIndicatorTimer.reset();
        CoverArtCache::destroy();
    }

  protected:
    TrackId addTrackToCollection(const QString& trackLocation) const {
        TrackPointer track = m_pTrackCollectionManager->getOrAddTrack(
                TrackRef::fromFilePath(getTestDir().filePath(trackLocation)));
        return track ? track->getId() : TrackId();
    }

    std::shared_ptr<mixxx::ControlIndicatorTimer> m_pControlIndicatorTimer;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::shared_ptr<EngineMixer> m_pEngine;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
    std::unique_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    std::unique_ptr<mixxx::network::rest::RestApiGateway> m_pGateway;
};

TEST_F(RestApiGatewayTest, PlaylistAddRejectsOutOfRangePosition) {
    auto* const collection = m_pTrackCollectionManager->internalCollection();
    ASSERT_NE(nullptr, collection);
    PlaylistDAO& playlistDao = collection->getPlaylistDAO();
    const int playlistId = playlistDao.createPlaylist(QStringLiteral("Test"));
    ASSERT_GT(playlistId, 0);

    const TrackId trackId = addTrackToCollection(kTrackLocationTest1);
    ASSERT_TRUE(trackId.isValid());
    playlistDao.appendTracksToPlaylist(QList<TrackId>{trackId}, playlistId);

    QJsonObject body;
    body.insert("action", "add");
    body.insert("playlist_id", playlistId);
    body.insert("track_ids", QJsonArray{trackId.toString()});
    body.insert("position", 5);

    const QHttpServerResponse response = m_pGateway->playlistCommand(body);
    EXPECT_EQ(response.statusCode(), QHttpServerResponse::StatusCode::BadRequest);
    const QJsonObject payload = QJsonDocument::fromJson(response.data()).object();
    EXPECT_THAT(payload.value("error").toString().toStdString(),
            ::testing::HasSubstr("out of range"));
}

TEST_F(RestApiGatewayTest, PlaylistReorderRejectsOutOfRangePosition) {
    auto* const collection = m_pTrackCollectionManager->internalCollection();
    ASSERT_NE(nullptr, collection);
    PlaylistDAO& playlistDao = collection->getPlaylistDAO();
    const int playlistId = playlistDao.createPlaylist(QStringLiteral("Test"));
    ASSERT_GT(playlistId, 0);

    const TrackId trackId = addTrackToCollection(kTrackLocationTest1);
    ASSERT_TRUE(trackId.isValid());
    playlistDao.appendTracksToPlaylist(QList<TrackId>{trackId}, playlistId);

    QJsonObject body;
    body.insert("action", "reorder");
    body.insert("playlist_id", playlistId);
    body.insert("from", 0);
    body.insert("to", 2);

    const QHttpServerResponse response = m_pGateway->playlistCommand(body);
    EXPECT_EQ(response.statusCode(), QHttpServerResponse::StatusCode::BadRequest);
    const QJsonObject payload = QJsonDocument::fromJson(response.data()).object();
    EXPECT_THAT(payload.value("error").toString().toStdString(),
            ::testing::HasSubstr("out of range"));
}

TEST_F(RestApiGatewayTest, AutoDjMoveRejectsOutOfRangePosition) {
    auto* const collection = m_pTrackCollectionManager->internalCollection();
    ASSERT_NE(nullptr, collection);
    PlaylistDAO& playlistDao = collection->getPlaylistDAO();
    const int autoDjPlaylistId =
            playlistDao.createPlaylist(AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
    ASSERT_GT(autoDjPlaylistId, 0);

    const TrackId trackId = addTrackToCollection(kTrackLocationTest1);
    ASSERT_TRUE(trackId.isValid());
    playlistDao.appendTracksToPlaylist(QList<TrackId>{trackId}, autoDjPlaylistId);

    QJsonObject body;
    body.insert("action", "move");
    body.insert("from", 0);
    body.insert("to", 2);

    const QHttpServerResponse response = m_pGateway->autoDj(body);
    EXPECT_EQ(response.statusCode(), QHttpServerResponse::StatusCode::BadRequest);
    const QJsonObject payload = QJsonDocument::fromJson(response.data()).object();
    EXPECT_THAT(payload.value("error").toString().toStdString(),
            ::testing::HasSubstr("out of range"));
}

} // namespace

#endif // MIXXX_HAS_HTTP_SERVER
