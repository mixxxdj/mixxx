#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <memory>

#include "preferences/usersettings.h"

namespace jack {
QString selectAutoConnectDestination(
        const QStringList& inputPorts,
        const QStringList& connectedPorts,
        const QString& rememberedDestination,
        const QString& prefix,
        const QSet<QString>& claimedDestinations,
        const QHash<QString, QString>& reservedDestinations,
        const QString& source,
        qsizetype channel,
        qsizetype outputCount);
}

class JackAutoConnector final : public QObject {
  public:
    explicit JackAutoConnector(
            UserSettingsPointer pConfig, QObject* pParent = nullptr);
    ~JackAutoConnector() override;

    void start();
    void stop();

  private:
    struct Impl;

    void connectPorts();
    void saveDestinations() const;

    std::unique_ptr<Impl> m_pImpl;
    QTimer m_timer;
    UserSettingsPointer m_pConfig;
};
