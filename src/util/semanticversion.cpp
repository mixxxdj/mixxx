#include "util/semanticversion.h"

#include <QRegularExpression>
#include <QString>
#include <QStringView>

namespace {
// thread local because SemanticVersion constructors below accesses it mutably.
// making it thread_local should avoid a datarace without having to introduce locking.
thread_local QRegularExpression regex(
        QStringLiteral("(?<major>\\d+)\\.(?<minor>\\d+)\\.(?<patch>\\d+)"));

} // anonymous namespace

namespace mixxx {

SemanticVersion::SemanticVersion(const QString& versionString)
        : SemanticVersion() {
    QRegularExpressionMatch match = regex.match(versionString);
    if (match.hasMatch()) {
        majorVersion = match.capturedView(QStringView(u"major")).toUInt();
        minorVersion = match.capturedView(QStringView(u"minor")).toUInt();
        patchVersion = match.capturedView(QStringView(u"patch")).toUInt();
    }
}

} // namespace mixxx
