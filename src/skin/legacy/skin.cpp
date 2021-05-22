#include "skin/legacy/skin.h"

#include "skin/legacy/legacyskinparser.h"

namespace {

const QRegExp kMinSizeRegExp("<MinimumSize>(\\d+), *(\\d+)<");

} // namespace

namespace mixxx {
namespace skin {
namespace legacy {

Skin::Skin(const QFileInfo& path)
        : m_path(path) {
    DEBUG_ASSERT(isValid());
}

bool Skin::isValid() const {
    return !m_path.filePath().isEmpty() && m_path.exists();
}

QFileInfo Skin::path() const {
    DEBUG_ASSERT(isValid());
    return m_path;
}

QFileInfo Skin::skinFile() const {
    DEBUG_ASSERT(isValid());
    return QFileInfo(path().absoluteFilePath() + QStringLiteral("/skin.xml"));
}

QString Skin::name() const {
    DEBUG_ASSERT(isValid());
    return m_path.fileName();
}

QList<QString> Skin::colorschemes() const {
    DEBUG_ASSERT(isValid());
    return LegacySkinParser::getSchemeList(path().absoluteFilePath());
}

QString Skin::description() const {
    DEBUG_ASSERT(isValid());
    SkinManifest manifest = LegacySkinParser::getSkinManifest(
            LegacySkinParser::openSkin(path().absoluteFilePath()));
    QString description = QString::fromStdString(manifest.description());
    if (!manifest.has_description() || description.isEmpty()) {
        return QString();
    }
    return description;
}

bool Skin::fitsScreenSize(const QScreen& screen) const {
    DEBUG_ASSERT(isValid());
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
