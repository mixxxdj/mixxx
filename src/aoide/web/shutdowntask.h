#pragma once

#include "network/jsonwebtask.h"

namespace aoide {

class ShutdownTask : public mixxx::network::JsonWebTask {
    Q_OBJECT

  public:
    ShutdownTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl);
    ~ShutdownTask() override = default;
};

} // namespace aoide
