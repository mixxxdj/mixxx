#include "aoide/web/shutdowntask.h"

namespace {

mixxx::network::JsonWebRequest buildRequest() {
    return mixxx::network::JsonWebRequest{
            mixxx::network::HttpRequestMethod::Post,
            QStringLiteral("/shutdown"),
            QUrlQuery(),
            QJsonDocument()};
}

} // anonymous namespace

namespace aoide {

ShutdownTask::ShutdownTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl)
        : mixxx::network::JsonWebTask(
                  networkAccessManager,
                  std::move(baseUrl),
                  buildRequest()) {
}

} // namespace aoide
