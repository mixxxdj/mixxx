#include "osc/query/oscquerydescription.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>

#include "preferences/configobject.h"

namespace {

template<typename T>
T take_back(std::vector<T>* pVec) {
    T last_element = std::move(pVec->back());
    pVec->pop_back();
    return last_element;
}

QString toOscAddress(const ConfigKey& key) {
    QString oscAddress = key.group + key.item;
    oscAddress.remove(QChar('['));
    oscAddress.remove(QChar(']'));
    oscAddress.replace(QChar('_'), QChar('/'));
    return oscAddress;
}

} // namespace

OscQueryDescription::OscQueryDescription()
        : m_rootObject({{"DESCRIPTION", "Mixxx OSC root node"},
                  {"FULL_PATH", "/"},
                  {"ACCESS", "0"},
                  {"CONTENTS", QJsonObject()}}) {
}

void OscQueryDescription::addAddress(
        const QString& address,
        const QString& type,
        const QString& access,
        const QString& description) {
    QStringList pathParts = address.split('/');
    QString fullPath;

    std::vector<QJsonObject> objects;
    objects.reserve(pathParts.size());

    // Create a list of QJsonObjects.
    // Note: QJsonObject is only temporary helper class for writing a QJsonValue
    for (const auto& pathPart : std::as_const(pathParts)) {
        if (pathPart.isEmpty()) {
            continue;
        }
        fullPath += "/" + pathPart;
        if (objects.size()) {
            QJsonValueRef currentContent = objects.back()["CONTENTS"];
            if (currentContent.isObject()) {
                objects.push_back(currentContent.toObject());
            } else {
                objects.emplace_back();
            }
        } else {
            objects.push_back(m_rootObject["CONTENTS"].toObject());
        }

        QJsonValueRef currentPart = objects.back()[pathPart];
        if (currentPart.isObject()) {
            objects.push_back(currentPart.toObject());
        } else {
            objects.emplace_back(QJsonObject({{"ACCESS", "0"}, {"FULL_PATH", fullPath}}));
        }
    }

    // populate
    objects.back()["FULL_PATH"] = address;
    objects.back()["TYPE"] = type;
    objects.back()["ACCESS"] = access;
    objects.back()["DESCRIPTION"] = description;

    // recreate form the leaves
    for (auto it = pathParts.crbegin(); it != pathParts.crend(); ++it) {
        objects.back()[*it] = take_back(&objects);
        if (objects.size() == 1) {
            m_rootObject["CONTENTS"] = take_back(&objects);
            break;
        }
        objects.back()["CONTENTS"] = take_back(&objects);
    }
}

void OscQueryDescription::removeAddress(const QString& address) {
    QStringList pathParts = address.split('/');

    std::vector<QJsonObject> objects;
    objects.reserve(pathParts.size());

    // Create a list of QJsonObjects.
    // Note: QJsonObject is only temporary helper class for writing a QJsonValue
    for (const auto& pathPart : std::as_const(pathParts)) {
        if (pathPart.isEmpty()) {
            continue;
        }
        if (objects.size()) {
            QJsonValueRef currentContent = objects.back()["CONTENTS"];
            if (currentContent.isObject()) {
                objects.push_back(currentContent.toObject());
            } else {
                objects.back().remove("CONTENTS");
                return;
            }
        } else {
            objects.push_back(m_rootObject["CONTENTS"].toObject());
        }

        QJsonValueRef currentPart = objects.back()[pathPart];
        if (currentPart.isObject()) {
            objects.push_back(currentPart.toObject());
        } else {
            objects.back().remove(pathPart);
            return;
        }
    }

    if (objects.back().contains("CONTENTS")) {
        objects.back().remove("TYPE");
        objects.back().remove("DESCRIPTION");
        objects.back()["ACCESS"] = "0";
    } else {
        objects.pop_back();
        objects.back().remove(pathParts.takeLast());
        if (objects.back().isEmpty()) {
            objects.pop_back();
            objects.back().remove("CONTENTS");
        } else {
            objects.back()["CONTENTS"] = take_back(&objects);
        }
    }

    // recreate form the leaves
    for (auto it = pathParts.crbegin(); it != pathParts.crend(); ++it) {
        if (!objects.back().contains("CONTENTS") && !objects.back().contains("TYPE")) {
            objects.pop_back();
            objects.back().remove(*it);
        } else {
            objects.back()[*it] = take_back(&objects);
        }
        if (objects.size() == 1) {
            m_rootObject["CONTENTS"] = take_back(&objects);
            break;
        }
        if (objects.back().isEmpty()) {
            objects.pop_back();
            objects.back().remove("CONTENTS");
        } else {
            objects.back()["CONTENTS"] = take_back(&objects);
        }
    }
}

QString OscQueryDescription::toJsonString() const {
    QJsonDocument doc(m_rootObject);
    return doc.toJson(QJsonDocument::Indented);
}

bool OscQueryDescription::saveToFile(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    file.write(toJsonString().toUtf8());
    file.close();
    return true;
}

void OscQueryDescription::insertControlKey(const ConfigKey& key) {
    QString address = toOscAddress(key);
    addAddress("/cop/" + address, "d", "3", "");
    addAddress("/get/cop/" + address, "d", "3", "");
    addAddress("/cov/" + address, "d", "3", "");
    addAddress("/get/cov/" + address, "d", "3", "");
}

void OscQueryDescription::removeControlKey(const ConfigKey& key) {
    QString address = toOscAddress(key);
    removeAddress("/cop/" + address);
    removeAddress("/get/cop/" + address);
    removeAddress("/cov/" + address);
    removeAddress("/get/cov/" + address);
}
