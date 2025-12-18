#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <optional>

#include <QObject>

#include "network/rest/restserver.h"
#include "preferences/restserversettings.h"

class QTcpServer;

namespace mixxx::network::rest {

struct RestServerValidationResult {
    RestServer::Settings settings;
    std::optional<RestServer::TlsResult> tlsConfiguration;
    bool success{false};
    QString error;
    QString tlsError;
};

class RestServerValidator {
  public:
    RestServerValidator(
            RestServer::Settings activeSettings,
            bool serverRunning,
            CertificateGenerator* certificateGenerator);

    RestServerValidationResult validate(const RestServer::Settings& settings) const;

  private:
    bool isPortAvailable(const RestServer::Settings& settings) const;

    RestServer::Settings m_activeSettings;
    bool m_serverRunning;
    CertificateGenerator* m_certificateGenerator;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER

