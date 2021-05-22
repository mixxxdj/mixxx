#include "skin/legacy/legacyskin.h"

#include "skin/legacy/legacyskinparser.h"

namespace {

const QRegExp kMinSizeRegExp("<MinimumSize>(\\d+), *(\\d+)<");

} // namespace

namespace mixxx {
namespace skin {
namespace legacy {

LegacySkin::LegacySkin(const QFileInfo& path)
        : m_path(path) {
    DEBUG_ASSERT(isValid());
}

bool LegacySkin::isValid() const {
    return !m_path.filePath().isEmpty() && m_path.exists();
}

QFileInfo LegacySkin::path() const {
    DEBUG_ASSERT(isValid());
    return m_path;
}

QPixmap LegacySkin::preview(const QString& schemeName = QString()) const {
    QPixmap preview;
    if (!schemeName.isEmpty()) {
        QString schemeNameUnformatted = schemeName;
        QString schemeNameFormatted = schemeNameUnformatted.replace(" ", "");
        preview.load(m_path.absoluteFilePath() +
                QStringLiteral("/skin_preview_") + schemeNameFormatted +
                QStringLiteral(".png"));
    } else {
        preview.load(m_path.absoluteFilePath() + QStringLiteral("/skin_preview.png"));
    }
    if (!preview.isNull()) {
        return preview;
    }
    preview.load(":/images/skin_preview_placeholder.png");
    return preview;
}

QFileInfo LegacySkin::skinFile() const {
    DEBUG_ASSERT(isValid());
    return QFileInfo(path().absoluteFilePath() + QStringLiteral("/skin.xml"));
}

QString LegacySkin::name() const {
    DEBUG_ASSERT(isValid());
    return m_path.fileName();
}

QList<QString> LegacySkin::colorschemes() const {
    DEBUG_ASSERT(isValid());
    return LegacySkinParser::getSchemeList(path().absoluteFilePath());
}

QString LegacySkin::description() const {
    DEBUG_ASSERT(isValid());
    SkinManifest manifest = LegacySkinParser::getSkinManifest(
            LegacySkinParser::openSkin(path().absoluteFilePath()));
    QString description = QString::fromStdString(manifest.description());
    if (!manifest.has_description() || description.isEmpty()) {
        return QString();
    }
    return description;
}

bool LegacySkin::fitsScreenSize(const QScreen& screen) const {
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
