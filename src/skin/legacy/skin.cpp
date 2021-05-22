#include "skin/legacy/skin.h"

#include "skin/legacy/legacyskinparser.h"

namespace {

const QRegExp kMinSizeRegExp("<MinimumSize>(\\d+), *(\\d+)<");

} // namespace

namespace mixxx {
namespace skin {
namespace legacy {

QFileInfo Skin::skinFile() const {
    return QFileInfo(path().absoluteFilePath() + QStringLiteral("/skin.xml"));
}

QString Skin::name() const {
    return m_path.fileName();
}

QList<QString> Skin::colorschemes() const {
    return LegacySkinParser::getSchemeList(path().absoluteFilePath());
}

QString Skin::description() const {
    SkinManifest manifest = LegacySkinParser::getSkinManifest(
            LegacySkinParser::openSkin(path().absoluteFilePath()));
    QString description = QString::fromStdString(manifest.description());
    if (!manifest.has_description() || description.isEmpty()) {
        return QString();
    }
    return description;
}

bool Skin::fitsScreenSize(const QScreen& screen) const {
    // Use the full resolution of the entire screen that is
    // available in full-screen mode.
    const auto screenSize = screen.size();
    QFile file(skinFile().absoluteFilePath());
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        bool found_size = false;
        while (!in.atEnd()) {
            if (kMinSizeRegExp.indexIn(in.readLine()) != -1) {
                found_size = true;
                break;
            }
        }
        if (found_size) {
            return !(kMinSizeRegExp.cap(1).toInt() > screenSize.width() ||
                    kMinSizeRegExp.cap(2).toInt() > screenSize.height());
        }
    }

    // If regex failed, fall back to skin name parsing.
    QString skinName = name().left(name().indexOf(QRegExp("\\d")));
    QString resName = name().right(name().count() - skinName.count());
    QString res = resName.left(resName.lastIndexOf(QRegExp("\\d")) + 1);
    QString skinWidth = res.left(res.indexOf("x"));
    QString skinHeight = res.right(res.count() - skinWidth.count() - 1);
    return skinWidth.toInt() <= screenSize.width() &&
            skinHeight.toInt() <= screenSize.height();
}

} // namespace legacy
} // namespace skin
} // namespace mixxx
