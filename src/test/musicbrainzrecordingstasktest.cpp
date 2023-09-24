#include "test/musicbrainzrecordingstasktest.h"

#include <gtest/gtest.h>

#include <QNetworkAccessManager>
#include <QTest>

#include "moc_musicbrainzrecordingstasktest.cpp"
#include "musicbrainz/web/musicbrainzrecordingstask.h"
#include "test/mixxxtest.h"
#include "test/mock_networkaccessmanager.h"

class MusicBrainzRecordingsTaskTest : public MixxxTest {
  protected:
    MusicBrainzRecordingsTaskTest() {
        m_pReceiver.reset(new MockMusicBrainzReceiver());
        m_recordingIds = {
                QUuid::fromString(QLatin1String("416a273e-51b8-4b1c-8873-4c9b4ed54a0f")),
                QUuid::fromString(QLatin1String("5e0aa7e4-01cb-441c-9fc2-d89e890cb981")),
                QUuid::fromString(QLatin1String("5f6340ae-9cab-4f00-83d5-7ad00ac35f5b"))};
        m_expectedParams = {
                {"inc", "artists+artist-credits+releases+release-groups+media"}};

        m_pMusicBrainzTask.reset(new mixxx::MusicBrainzRecordingsTask(
                &m_network,
                m_recordingIds,
                m_pReceiver.data()));
        QObject::connect(m_pMusicBrainzTask.data(),
                &mixxx::MusicBrainzRecordingsTask::succeeded,
                m_pReceiver.data(),
                &MockMusicBrainzReceiver::slotMusicBrainzTaskSucceeded);
        QObject::connect(m_pMusicBrainzTask.data(),
                &mixxx::MusicBrainzRecordingsTask::failed,
                m_pReceiver.data(),
                &MockMusicBrainzReceiver::slotMusicBrainzTaskFailed);
        QObject::connect(m_pMusicBrainzTask.data(),
                &mixxx::MusicBrainzRecordingsTask::aborted,
                m_pReceiver.data(),
                &MockMusicBrainzReceiver::slotMusicBrainzTaskAborted);
        QObject::connect(m_pMusicBrainzTask.data(),
                &mixxx::MusicBrainzRecordingsTask::networkError,
                m_pReceiver.data(),
                &MockMusicBrainzReceiver::slotMusicBrainzTaskNetworkError);
    };

    QScopedPointer<MockMusicBrainzReceiver> m_pReceiver;
    QScopedPointer<mixxx::MusicBrainzRecordingsTask> m_pMusicBrainzTask;
    QList<QUuid> m_recordingIds;
    QMap<QString, QString> m_expectedParams;
    MockNetworkAccessManager m_network;
};

void MockMusicBrainzReceiver::slotMusicBrainzTaskSucceeded(
        const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases) {
    Q_UNUSED(guessedTrackReleases);
    qDebug() << "MockMusicBrainzReceiver::slotMusicBrainzTaskSucceeded";
    MocSucceeded();
}

void MockMusicBrainzReceiver::slotMusicBrainzTaskFailed(
        const mixxx::network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    Q_UNUSED(response);
    Q_UNUSED(errorCode);
    Q_UNUSED(errorMessage);
    qDebug() << "MockMusicBrainzReceiver::slotMusicBrainzTaskFailed";
    MocFailed();
}

void MockMusicBrainzReceiver::slotMusicBrainzTaskAborted() {
    qDebug() << "MockMusicBrainzReceiver::slotMusicBrainzTaskAborted";
    MocAborted();
}

void MockMusicBrainzReceiver::slotMusicBrainzTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    Q_UNUSED(errorCode);
    Q_UNUSED(errorString);
    Q_UNUSED(responseWithContent);
    qDebug() << "MockMusicBrainzReceiver::slotMusicBrainzTaskNetworkError";
    MocNetworkError();
}

TEST_F(MusicBrainzRecordingsTaskTest, ClinetSideTimeout) {
    EXPECT_CALL(*m_pReceiver.data(), MocNetworkError()).Times(1);
    m_network.ExpectGet(
            m_recordingIds[0].toString(QUuid::WithoutBraces),
            m_expectedParams,
            404,
            QByteArray());
    m_pMusicBrainzTask->invokeStart(100);

    // Wait until the timeout happens
    while (m_pMusicBrainzTask->isBusy()) {
        application()->processEvents();
    }
}

TEST_F(MusicBrainzRecordingsTaskTest, RespodsEmpty) {
    EXPECT_CALL(*m_pReceiver.data(), MocFailed()).Times(1);
    MockNetworkReply* pReply0 = m_network.ExpectGet(
            m_recordingIds[0].toString(QUuid::WithoutBraces),
            m_expectedParams,
            200,
            QByteArray());
    m_pMusicBrainzTask->invokeStart(10000);

    application()->processEvents();
    qDebug() << "pReply0->Done()" << pReply0;
    pReply0->Done();
    MockNetworkReply* pReply1 = m_network.ExpectGet(
            m_recordingIds[1].toString(QUuid::WithoutBraces),
            m_expectedParams,
            200,
            QByteArray());

    QTest::qSleep(1100); // millis
    application()->processEvents();

    qDebug() << "pReply1->Done()" << pReply1;
    pReply1->Done();
    MockNetworkReply* pReply2 = m_network.ExpectGet(
            m_recordingIds[2].toString(QUuid::WithoutBraces),
            m_expectedParams,
            200,
            QByteArray());

    QTest::qSleep(1100); // millis
    application()->processEvents();

    qDebug() << "pReply2->Done()" << pReply2;
    pReply2->Done();

    while (m_pMusicBrainzTask->isBusy()) {
        application()->processEvents();
    }
}
