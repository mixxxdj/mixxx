#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <thread>

#include "qml/qmlautoreload.h"

namespace {

class QmlAutoReloadTest : public testing::Test {
  protected:
    void registerFromWorker(mixxx::qml::QmlAutoReload* pReloader,
            const QUrl& url) {
        QUrl intercepted;
        std::thread worker([&]() {
            intercepted = pReloader->intercept(url,
                    QQmlAbstractUrlInterceptor::QmlFile);
        });
        worker.join();
        EXPECT_EQ(url, intercepted);
    }
};

TEST_F(QmlAutoReloadTest, WorkerRegistrationTriggersOnOwnerThread) {
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    mixxx::qml::QmlAutoReload reloader;
    registerFromWorker(&reloader, QUrl::fromLocalFile(file.fileName()));
    QCoreApplication::sendPostedEvents(&reloader, QEvent::MetaCall);

    QEventLoop loop;
    bool triggered = false;
    QObject::connect(&reloader, &mixxx::qml::QmlAutoReload::triggered, &loop, [&]() {
        triggered = true;
        EXPECT_EQ(reloader.thread(), QThread::currentThread());
        loop.quit();
    });
    ASSERT_EQ(1, file.write("x"));
    ASSERT_TRUE(file.flush());
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_TRUE(triggered);
}

TEST_F(QmlAutoReloadTest, ClearDiscardsPendingWorkerRegistration) {
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    mixxx::qml::QmlAutoReload reloader;
    registerFromWorker(&reloader, QUrl::fromLocalFile(file.fileName()));
    reloader.clear();
    QCoreApplication::sendPostedEvents(&reloader, QEvent::MetaCall);

    QEventLoop loop;
    bool triggered = false;
    QObject::connect(&reloader, &mixxx::qml::QmlAutoReload::triggered, &loop, [&]() {
        triggered = true;
        loop.quit();
    });
    ASSERT_EQ(1, file.write("x"));
    ASSERT_TRUE(file.flush());
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_FALSE(triggered);
}

} // namespace
