#pragma once

#include <QJsonObject>

namespace mixxx {

namespace hid {

class HidUsageTables {
  public:
    explicit HidUsageTables(const QString& filePath);
    QString getUsagePageDescription(unsigned short usagePage) const;
    QString getUsageDescription(unsigned short usagePage, unsigned short usage) const;

  private:
    QJsonObject m_hidUsageTables;
};

} // namespace hid

} // namespace mixxx
