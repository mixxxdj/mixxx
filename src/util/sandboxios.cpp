#include "util/sandboxios.h"

#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QStringLiteral>
#include <QtGlobal>

namespace mixxx {

#ifdef Q_OS_IOS

static const QRegularExpression sandboxPrefixRegex = QRegularExpression(
        QStringLiteral("^(?:/private)?/var/mobile/Containers/Data/Application/"
                       "[a-zA-Z0-9\\-]+(/.*)"));

QString updateIOSSandboxPath(const QString& path) {
    QRegularExpressionMatch match = sandboxPrefixRegex.match(path);
    if (!match.hasMatch()) {
        qWarning() << "Tried updating iOS sandbox prefix in path outside sandbox:"
                   << path
                   << "Perhaps the regex in util/sandboxios.cpp needs to be "
                      "updated?";
        return path;
    }

    QString newSandboxPrefix = QDir::homePath();
    QString relativePath = match.captured(1);

    return newSandboxPrefix + relativePath;
}

#endif

} // namespace mixxx
