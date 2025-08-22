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
        QStringList pointSizesStr;
        const QStringList families = QFontDatabase::applicationFontFamilies(result);
        for (const QString& family : families) {
            const QStringList styles = QFontDatabase::styles(family);
            for (const QString& style : styles) {
                const QList<int> pointSizes = QFontDatabase::pointSizes(family, style);
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

    const QList<QString> files = fontsDir.entryList(
            QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
    for (const QString& path : files) {
        // Skip text files (e.g. license files). For all others we let Qt tell
        // us whether the font format is supported since there is no way to
        // check other than adding.
        if (path.endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive)) {
            continue;
        }

        addFont(path);
    }
}
