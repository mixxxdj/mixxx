#include "util/font.h"

#include <QDir>
#include <QFontDatabase>
#include <QString>
#include <QtDebug>

#include "util/cmdlineargs.h"

namespace {

bool addFont(const QString& path) {
    int result = QFontDatabase::addApplicationFont(path);
    if (result == -1) {
        qWarning() << "Failed to add font:" << path;
        return false;
    }

    // In developer mode, spit out all the families / styles / sizes
    // supported by the new font.
    if (CmdlineArgs::Instance().getDeveloper()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QFontDatabase database;
#endif
        QStringList pointSizesStr;
        const QStringList families = QFontDatabase::applicationFontFamilies(result);
        for (const QString& family : families) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QStringList styles = QFontDatabase::styles(family);
#else
            const QStringList styles = database.styles(family);
#endif
            for (const QString& style : styles) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                const QList<int> pointSizes = QFontDatabase::pointSizes(family, style);
#else
                const QList<int> pointSizes = database.pointSizes(family, style);
#endif
                pointSizesStr.clear();
                pointSizesStr.reserve(pointSizes.count());
                for (int point : pointSizes) {
                    pointSizesStr.append(QString::number(point));
                }
                qDebug() << "FONT LOADED family:" << family
                         << "style:" << style
                         << "point sizes:" << pointSizesStr.join(",");
            }
        }
    }
    return true;
}

} // namespace

void FontUtils::initializeFonts(const QString& resourcePath) {
    QDir fontsDir(resourcePath);
    if (!fontsDir.cd("fonts")) {
#ifdef __LINUX__
        // If the fonts already have been installed via the package
        // manager, this is okay. We currently have no way to verify that
        // though.
        qDebug()
#else
        qWarning()
#endif
                << "No fonts directory found in" << resourcePath;
        return;
    }

    const QFileInfoList files = fontsDir.entryInfoList(
            QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    for (const QFileInfo& file : files) {
        // Skip text files (e.g. license files). For all others we let Qt tell
        // us whether the font format is supported since there is no way to
        // check other than adding.
        if (file.suffix().toLower() == QStringLiteral("txt")) {
            continue;
        }

        addFont(file.canonicalFilePath());
    }
}
