#pragma once

#include <QFontDatabase>
#include <QString>
#include <QtDebug>
#include <QDir>

#include "util/cmdlineargs.h"

class FontUtils {
  public:
    static void initializeFonts(const QString& resourcePath) {
        QDir fontsDir(resourcePath);
        if (!fontsDir.cd("fonts")) {
            qWarning("FontUtils::initializeFonts: cd fonts failed");
            return;
        }

        QList<QFileInfo> files = fontsDir.entryInfoList(
            QDir::NoDotAndDotDot | QDir::Files | QDir::Readable);
        foreach (const QFileInfo& file, files) {
            const QString& path = file.filePath();

            // Skip text files (e.g. license files). For all others we let Qt tell
            // us whether the font format is supported since there is no way to
            // check other than adding.
            if (path.endsWith(".txt", Qt::CaseInsensitive)) {
                continue;
            }

            addFont(path);
        }
    }

    static bool addFont(const QString& path) {
        int result = QFontDatabase::addApplicationFont(path);
        if (result == -1) {
            qWarning() << "Failed to add font:" << path;
            return false;
        }

        // In developer mode, spit out all the families / styles / sizes
        // supported by the new font.
        if (CmdlineArgs::Instance().getDeveloper()) {
            QFontDatabase database;
            QStringList families = QFontDatabase::applicationFontFamilies(result);
            foreach (const QString& family, families) {
                QStringList styles = database.styles(family);
                foreach (const QString& style, styles) {
                    QList<int> pointSizes = database.pointSizes(family, style);
                    QStringList pointSizesStr;
                    foreach (int point, pointSizes) {
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

  private:
    FontUtils() {}
};
