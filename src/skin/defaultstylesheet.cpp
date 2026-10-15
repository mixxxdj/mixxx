#include "skin/defaultstylesheet.h"

#include <QFile>
#include <QString>
#include <QtDebug>

namespace mixxx {
namespace skin {

QString loadDefaultStyleSheet(const QString& resourcePath) {
    QFile file(resourcePath + QStringLiteral("/skins/default.qss"));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load default skin styles /skins/default.qss!";
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace skin
} // namespace mixxx
