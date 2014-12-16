#ifndef FONT_H
#define FONT_H

#include <QFontDatabase>
#include <QString>
#include <QtDebug>

#include "util/cmdlineargs.h"

class FontUtils {
  public:
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

#endif /* FONT_H */
