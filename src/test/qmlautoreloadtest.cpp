#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
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
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto filePath = directory.filePath(QStringLiteral("watched.qml"));
    {
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(1, file.write("x"));
    }
    mixxx::qml::QmlAutoReload reloader;
    registerFromWorker(&reloader, QUrl::fromLocalFile(filePath));
    QCoreApplication::sendPostedEvents(&reloader, QEvent::MetaCall);

    QEventLoop loop;
    bool triggered = false;
    QObject::connect(&reloader, &mixxx::qml::QmlAutoReload::triggered, &loop, [&]() {
        triggered = true;
        EXPECT_EQ(reloader.thread(), QThread::currentThread());
        loop.quit();
    });
    {
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Append));
        ASSERT_EQ(1, file.write("x"));
        ASSERT_TRUE(file.flush());
    }
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_TRUE(triggered);
}

TEST_F(QmlAutoReloadTest, ClearDiscardsPendingWorkerRegistration) {
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto filePath = directory.filePath(QStringLiteral("watched.qml"));
    {
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(1, file.write("x"));
    }
    mixxx::qml::QmlAutoReload reloader;
    registerFromWorker(&reloader, QUrl::fromLocalFile(filePath));
    reloader.clear();
    QCoreApplication::sendPostedEvents(&reloader, QEvent::MetaCall);

    QEventLoop loop;
    bool triggered = false;
    QObject::connect(&reloader, &mixxx::qml::QmlAutoReload::triggered, &loop, [&]() {
        triggered = true;
        loop.quit();
    });
    {
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Append));
        ASSERT_EQ(1, file.write("x"));
        ASSERT_TRUE(file.flush());
    }
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
    EXPECT_FALSE(triggered);
}

} // namespace
