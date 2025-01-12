#include "util/semanticversion.h"

#include <QRegularExpression>
#include <QString>
#include <QStringView>

namespace {
// while this is accessed mutably concurrently below QRegularExpression is fully
// threadsafe (even though not documented). See `tst_QRegularExpression::threadSafety`
// from the Qt sources.
QRegularExpression regex(
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
