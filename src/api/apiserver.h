#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QThread>
#include <atomic>
#include <memory>

class PlayerManager;

/// HTTP REST API server for remote control and automation.
/// Provides endpoints for deck control, mixer, EQ, effects, library,
/// and real-time state polling. Binds to localhost by default.
class ApiServer : public QObject {
    Q_OBJECT

  public:
    explicit ApiServer(int port,
            const QString& bindAddress,
            PlayerManager* pPlayerManager,
            QObject* parent = nullptr);
    ~ApiServer();

    void start();
    void stop();

  private:
    void run();

    // Helpers
    double getControl(const QString& group, const QString& key);
    void setControl(const QString& group, const QString& key, double value);
    QString deckGroup(int deck);
    QJsonObject getTrackInfo(int deck);

    int m_port;
    QString m_bindAddress;
    PlayerManager* m_pPlayerManager;
    std::unique_ptr<QThread> m_thread;
    std::atomic<bool> m_running{false};
};
