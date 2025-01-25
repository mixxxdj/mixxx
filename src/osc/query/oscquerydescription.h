#pragma once

#include <QJsonObject>

class QString;
class QVariant;
class ConfigKey;

class OscQueryDescription {
  public:
    OscQueryDescription();

    void addAddress(
            const QString& address,
            const QString& type,
            const QString& access,
            const QString& description);

    void removeAddress(const QString& address);

    void insertControlKey(const ConfigKey& key);
    void removeControlKey(const ConfigKey& key);

    QString toJsonString() const;

    bool saveToFile(const QString& filePath) const;

  private:
    QJsonObject m_rootObject;
};
