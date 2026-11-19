#include "soundio/jackautoconnector.h"

#include <QCollator>
#include <QDebug>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <algorithm>
#include <atomic>
#include <cerrno>

#if defined(MIXXX_HAVE_JACK) && defined(__linux__)
#include <jack/jack.h>
#endif

namespace {

bool hasAutoConnectPrefix(const QString& fullPortName, const QString& prefix) {
    return !prefix.isEmpty() &&
            fullPortName.section(QLatin1Char(':'), -1)
                    .startsWith(prefix, Qt::CaseSensitive);
}

} // namespace

QString jack::selectAutoConnectDestination(
        const QStringList& inputPorts,
        const QStringList& connectedPorts,
        const QString& rememberedDestination,
        const QString& prefix,
        const QSet<QString>& claimedDestinations,
        const QHash<QString, QString>& reservedDestinations,
        const QString& source,
        qsizetype channel,
        qsizetype outputCount) {
    const auto available = [&](const QString& destination) {
        return inputPorts.contains(destination) &&
                !claimedDestinations.contains(destination);
    };
    if (connectedPorts.contains(rememberedDestination) &&
            available(rememberedDestination)) {
        return rememberedDestination;
    }
    for (const QString& destination : connectedPorts) {
        if (available(destination)) {
            return destination;
        }
    }
    if (available(rememberedDestination)) {
        return rememberedDestination;
    }
    if (!rememberedDestination.isEmpty() &&
            !claimedDestinations.contains(rememberedDestination) &&
            hasAutoConnectPrefix(rememberedDestination, prefix) &&
            inputPorts.size() < outputCount) {
        return {};
    }
    if (channel >= inputPorts.size()) {
        return {};
    }
    const auto canAssign = [&](const QString& destination) {
        const auto owner = reservedDestinations.constFind(destination);
        return available(destination) &&
                (owner == reservedDestinations.cend() || *owner == source);
    };
    if (canAssign(inputPorts.at(channel))) {
        return inputPorts.at(channel);
    }
    for (const QString& destination : inputPorts) {
        if (canAssign(destination)) {
            return destination;
        }
    }
    return {};
}

struct JackAutoConnector::Impl {
#if defined(MIXXX_HAVE_JACK) && defined(__linux__)
    using ClientOpen = jack_client_t* (*)(const char*, jack_options_t, jack_status_t*, ...);
    using ClientClose = int (*)(jack_client_t*);
    using OnInfoShutdown = void (*)(jack_client_t*, JackInfoShutdownCallback, void*);
    using GetPorts = const char** (*)(jack_client_t*, const char*, const char*, unsigned long);
    using PortByName = jack_port_t* (*)(jack_client_t*, const char*);
    using PortId = jack_port_id_t (*)(const jack_port_t*);
    using PortGetAllConnections = const char** (*)(const jack_client_t*, const jack_port_t*);
    using Disconnect = int (*)(jack_client_t*, const char*, const char*);
    using Connect = int (*)(jack_client_t*, const char*, const char*);
    using Free = void (*)(void*);

    QLibrary library{QStringLiteral("libjack.so.0")};
    jack_client_t* pClient{nullptr};
    ClientOpen clientOpen{nullptr};
    ClientClose clientClose{nullptr};
    OnInfoShutdown onInfoShutdown{nullptr};
    GetPorts getPorts{nullptr};
    PortByName portByName{nullptr};
    PortId portId{nullptr};
    PortGetAllConnections portGetAllConnections{nullptr};
    Disconnect disconnect{nullptr};
    Connect connect{nullptr};
    Free free{nullptr};
    std::atomic_bool serverShutdown{false};
    bool symbolLoadAttempted{false};
    bool symbolsLoaded{false};
    bool openFailureReported{false};
    QSet<QString> reportedConnectionFailures;
    QSet<QString> reportedDuplicateRoutes;
    QSet<QString> cleanedSources;

    static void onServerShutdown(
            jack_status_t, const char*, void* pUserData) {
        static_cast<std::atomic_bool*>(pUserData)->store(true);
    }

    template<typename Function>
    bool resolve(Function& function, const char* name) {
        function = reinterpret_cast<Function>(library.resolve(name));
        if (!function) {
            qWarning() << "Could not resolve JACK function" << name;
            return false;
        }
        return true;
    }

    bool loadSymbols() {
        if (symbolLoadAttempted) {
            return symbolsLoaded;
        }
        symbolLoadAttempted = true;
        if (!library.load()) {
            qWarning() << "Could not load JACK library:" << library.errorString();
            return false;
        }
        symbolsLoaded =
                resolve(clientOpen, "jack_client_open") &&
                resolve(clientClose, "jack_client_close") &&
                resolve(onInfoShutdown, "jack_on_info_shutdown") &&
                resolve(getPorts, "jack_get_ports") &&
                resolve(portByName, "jack_port_by_name") &&
                resolve(portGetAllConnections, "jack_port_get_all_connections") &&
                resolve(disconnect, "jack_disconnect") &&
                resolve(connect, "jack_connect") &&
                resolve(free, "jack_free");
        if (!symbolsLoaded) {
            library.unload();
        } else {
            portId = reinterpret_cast<PortId>(library.resolve("jack_port_id"));
        }
        return symbolsLoaded;
    }

    bool openClient() {
        if (!loadSymbols()) {
            return false;
        }
        jack_status_t status = {};
        pClient = clientOpen(
                "MixxxAutoConnector", JackNoStartServer, &status);
        if (!pClient) {
            if (!openFailureReported) {
                qWarning() << "Could not connect JACK auto-connector to the server;"
                           << "will retry";
                openFailureReported = true;
            }
            return false;
        }
        openFailureReported = false;
        serverShutdown.store(false);
        onInfoShutdown(pClient, onServerShutdown, &serverShutdown);
        return true;
    }

    void closeClient() {
        if (pClient) {
            clientClose(pClient);
            pClient = nullptr;
        }
        serverShutdown.store(false);
        reportedConnectionFailures.clear();
        reportedDuplicateRoutes.clear();
        cleanedSources.clear();
    }
#endif
    QHash<QString, QString> destinationsBySource;
};

JackAutoConnector::JackAutoConnector(
        UserSettingsPointer pConfig, QObject* pParent)
        : QObject(pParent),
          m_pImpl(std::make_unique<Impl>()),
          m_pConfig(std::move(pConfig)) {
    const QString savedRoutes = m_pConfig->getValue(
            ConfigKey("[SoundManager]", "jack_auto_connect_routes"),
            QStringLiteral("{}"));
    QJsonParseError parseError;
    const QJsonDocument routesDocument =
            QJsonDocument::fromJson(savedRoutes.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError ||
            !routesDocument.isObject()) {
        qWarning() << "Ignoring invalid saved JACK auto-connect routes:"
                   << parseError.errorString();
    } else {
        static const QRegularExpression sourcePortNameRegex(
                QStringLiteral("^(out_\\d+|mixxx_[A-Za-z0-9_]+)$"));
        const QJsonObject routes = routesDocument.object();
        for (auto it = routes.constBegin(); it != routes.constEnd(); ++it) {
            const QString destination = it.value().toString();
            if (sourcePortNameRegex.match(it.key()).hasMatch() &&
                    it.value().isString() &&
                    destination.contains(QLatin1Char(':'))) {
                m_pImpl->destinationsBySource.insert(it.key(), destination);
            }
        }
    }

    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &JackAutoConnector::connectPorts);
}

JackAutoConnector::~JackAutoConnector() {
    stop();
}

void JackAutoConnector::start() {
#if defined(MIXXX_HAVE_JACK) && defined(__linux__)
    if (!m_timer.isActive()) {
        m_timer.start();
    }
    connectPorts();
#endif
}

void JackAutoConnector::stop() {
    m_timer.stop();
#if defined(MIXXX_HAVE_JACK) && defined(__linux__)
    m_pImpl->closeClient();
#endif
}

void JackAutoConnector::saveDestinations() const {
    QJsonObject routes;
    for (auto it = m_pImpl->destinationsBySource.constBegin();
            it != m_pImpl->destinationsBySource.constEnd();
            ++it) {
        routes.insert(it.key(), it.value());
    }
    m_pConfig->setValue(
            ConfigKey("[SoundManager]", "jack_auto_connect_routes"),
            QString::fromUtf8(
                    QJsonDocument(routes).toJson(QJsonDocument::Compact)));
}

void JackAutoConnector::connectPorts() {
#if defined(MIXXX_HAVE_JACK) && defined(__linux__)
    auto& impl = *m_pImpl;
    if (impl.serverShutdown.exchange(false)) {
        impl.closeClient();
    }
    if (!impl.pClient && !impl.openClient()) {
        return;
    }
    const QString autoConnectPrefix =
            m_pConfig
                    ->getValue(ConfigKey("[SoundManager]",
                                       "jack_auto_connect_prefix"),
                            QStringLiteral("Mixxx"))
                    .trimmed();

    const char** pOutputPorts = impl.getPorts(
            impl.pClient, nullptr, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput);
    const char** pInputPorts = impl.getPorts(
            impl.pClient, nullptr, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput);

    QStringList outputPorts;
    QStringList inputPorts;
    QSet<QString> ownClientNames;
    static const QRegularExpression outputPortIndexRegex(QStringLiteral("out_(\\d+)$"));
    const auto isMixxxClient = [](const QString& name) {
        return name.compare(QStringLiteral("mixxx"), Qt::CaseInsensitive) == 0 ||
                name.startsWith(QStringLiteral("mixxx-"), Qt::CaseInsensitive);
    };
    for (const char** pPortName = pOutputPorts;
            pPortName && *pPortName;
            ++pPortName) {
        const QString fullPortName = QString::fromLocal8Bit(*pPortName);
        const QString shortPortName = fullPortName.section(QLatin1Char(':'), -1);
        const QString clientName =
                fullPortName.section(QLatin1Char(':'), 0, 0);
        const bool isOwnedOutput =
                outputPortIndexRegex.match(shortPortName).hasMatch() ||
                shortPortName.startsWith(QStringLiteral("mixxx_"),
                        Qt::CaseInsensitive);
        if (!isOwnedOutput || !isMixxxClient(clientName)) {
            continue;
        }

        outputPorts.push_back(fullPortName);
        ownClientNames.insert(clientName);
    }

    for (const char** pPortName = pInputPorts;
            pPortName && *pPortName;
            ++pPortName) {
        const QString portName = QString::fromLocal8Bit(*pPortName);
        const QString clientName =
                portName.section(QLatin1Char(':'), 0, 0);
        if (!ownClientNames.contains(clientName) &&
                hasAutoConnectPrefix(portName, autoConnectPrefix)) {
            inputPorts.push_back(portName);
        }
    }
    if (pOutputPorts) {
        impl.free(const_cast<void*>(static_cast<const void*>(pOutputPorts)));
    }
    if (pInputPorts) {
        impl.free(const_cast<void*>(static_cast<const void*>(pInputPorts)));
    }

    bool outputPortsSortedById = false;
    if (impl.portId) {
        QHash<QString, jack_port_id_t> outputPortIds;
        for (const QString& outputPort : std::as_const(outputPorts)) {
            jack_port_t* pOutputPort =
                    impl.portByName(impl.pClient, outputPort.toLocal8Bit().constData());
            if (pOutputPort) {
                outputPortIds.insert(outputPort, impl.portId(pOutputPort));
            }
        }
        if (outputPortIds.size() == outputPorts.size()) {
            std::sort(outputPorts.begin(),
                    outputPorts.end(),
                    [&outputPortIds](
                            const QString& left, const QString& right) {
                        return outputPortIds.value(left) <
                                outputPortIds.value(right);
                    });
            outputPortsSortedById = true;
        }
    }
    if (!outputPortsSortedById) {
        const bool nativeJackPorts = std::all_of(
                outputPorts.cbegin(), outputPorts.cend(), [](const QString& port) {
                    return port.section(QLatin1Char(':'), -1)
                            .startsWith(QStringLiteral("mixxx_"));
                });
        if (!nativeJackPorts) {
            QCollator portNameOrder;
            portNameOrder.setNumericMode(true);
            std::sort(outputPorts.begin(),
                    outputPorts.end(),
                    [&portNameOrder](
                            const QString& left, const QString& right) {
                        return portNameOrder.compare(left, right) < 0;
                    });
        }
    }

    QCollator portNameOrder;
    portNameOrder.setNumericMode(true);
    std::sort(inputPorts.begin(),
            inputPorts.end(),
            [&portNameOrder](const QString& left, const QString& right) {
                return portNameOrder.compare(left, right) < 0;
            });
    QHash<QString, QString> reservedDestinations;
    for (qsizetype channel = 0; channel < outputPorts.size(); ++channel) {
        const QString shortSourceName =
                outputPorts.at(channel).section(QLatin1Char(':'), -1);
        QString destination = impl.destinationsBySource.value(shortSourceName);
        if (destination.isEmpty() &&
                shortSourceName.startsWith(QStringLiteral("mixxx_"))) {
            destination = impl.destinationsBySource.value(
                    QStringLiteral("out_%1").arg(channel));
        }
        if (inputPorts.contains(destination) &&
                !reservedDestinations.contains(destination)) {
            reservedDestinations.insert(destination, shortSourceName);
        }
    }
    QSet<QString> claimedDestinations;
    bool destinationsChanged = false;
    for (qsizetype channel = 0; channel < outputPorts.size(); ++channel) {
        const QString& source = outputPorts.at(channel);
        const QString shortSourceName =
                source.section(QLatin1Char(':'), -1);
        jack_port_t* pSourcePort =
                impl.portByName(impl.pClient, source.toLocal8Bit().constData());
        if (!pSourcePort) {
            continue;
        }

        QString rememberedDestination =
                impl.destinationsBySource.value(shortSourceName);
        if (rememberedDestination.isEmpty() &&
                shortSourceName.startsWith(QStringLiteral("mixxx_"))) {
            const QString legacySourceName =
                    QStringLiteral("out_%1").arg(channel);
            rememberedDestination =
                    impl.destinationsBySource.take(legacySourceName);
            if (!rememberedDestination.isEmpty()) {
                impl.destinationsBySource.insert(
                        shortSourceName, rememberedDestination);
                destinationsChanged = true;
            }
        }

        const char** pConnections =
                impl.portGetAllConnections(impl.pClient, pSourcePort);
        QStringList connectedPorts;
        for (const char** pConnection = pConnections;
                pConnection && *pConnection;
                ++pConnection) {
            const QString connectedPort = QString::fromLocal8Bit(*pConnection);
            connectedPorts.push_back(connectedPort);
        }
        if (pConnections) {
            impl.free(const_cast<void*>(static_cast<const void*>(pConnections)));
        }

        const QString destination =
                jack::selectAutoConnectDestination(inputPorts,
                        connectedPorts,
                        rememberedDestination,
                        autoConnectPrefix,
                        claimedDestinations,
                        reservedDestinations,
                        shortSourceName,
                        channel,
                        outputPorts.size());
        if (destination.isEmpty()) {
            const bool duplicateConnected = std::any_of(connectedPorts.cbegin(),
                    connectedPorts.cend(),
                    [&claimedDestinations](const QString& port) {
                        return claimedDestinations.contains(port);
                    });
            if (duplicateConnected && !impl.reportedDuplicateRoutes.contains(source)) {
                qWarning() << "Could not resolve duplicate JACK route for"
                           << source << "because no matching input is available";
                impl.reportedDuplicateRoutes.insert(source);
            } else if (!duplicateConnected) {
                impl.reportedDuplicateRoutes.remove(source);
            }
            continue;
        }
        bool destinationConnected = connectedPorts.contains(destination);
        if (!destinationConnected) {
            const int result = impl.connect(
                    impl.pClient,
                    source.toLocal8Bit().constData(),
                    destination.toLocal8Bit().constData());
            if (result == 0) {
                qInfo() << "Connected JACK output" << source << "to" << destination;
                impl.reportedConnectionFailures.remove(
                        source + QLatin1Char('>') + destination);
                destinationConnected = true;
            } else if (result == EEXIST) {
                destinationConnected = true;
            } else {
                const QString key = source + QLatin1Char('>') + destination;
                if (!impl.reportedConnectionFailures.contains(key)) {
                    qWarning() << "Could not connect JACK output" << source
                               << "to" << destination << "(error" << result << ")";
                    impl.reportedConnectionFailures.insert(key);
                }
            }
        }

        if (!destinationConnected) {
            continue;
        }
        impl.reportedDuplicateRoutes.remove(source);
        if (impl.destinationsBySource.value(shortSourceName) != destination) {
            impl.destinationsBySource.insert(shortSourceName, destination);
            destinationsChanged = true;
        }
        const bool previouslyCleaned = impl.cleanedSources.contains(source);
        bool cleanupSucceeded = true;
        for (const QString& connectedPort : std::as_const(connectedPorts)) {
            if (connectedPort == destination ||
                    (previouslyCleaned && !claimedDestinations.contains(connectedPort))) {
                continue;
            }
            const int result = impl.disconnect(
                    impl.pClient,
                    source.toLocal8Bit().constData(),
                    connectedPort.toLocal8Bit().constData());
            const QString key = source + QLatin1Char('>') + connectedPort;
            if (result != 0 && result != ENOENT) {
                cleanupSucceeded = false;
                if (!impl.reportedConnectionFailures.contains(key)) {
                    qWarning() << "Could not remove non-matching JACK connection from"
                               << source << "to" << connectedPort << "(error"
                               << result << ")";
                    impl.reportedConnectionFailures.insert(key);
                }
            } else if (result == 0) {
                qInfo() << "Removed non-matching JACK connection from" << source
                        << "to" << connectedPort;
                impl.reportedConnectionFailures.remove(key);
            }
        }
        if (cleanupSucceeded) {
            impl.cleanedSources.insert(source);
        }
        claimedDestinations.insert(destination);
    }
    if (destinationsChanged) {
        saveDestinations();
    }
#endif
}
