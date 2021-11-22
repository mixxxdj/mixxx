#pragma once

#include <QCollator>
#include <QColor>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QString>
#include <QStringRef>

namespace {

static const QRegularExpression kExtRgxp(R"(\(\*\.(.*)\)$)");

} //anonymous namespace

// The default comparison of strings for sorting.
class StringCollator {
  public:
    explicit StringCollator(QLocale locale = QLocale())
        : m_collator(std::move(locale)) {
        m_collator.setCaseSensitivity(Qt::CaseInsensitive);
    }

    int compare(const QString& s1, const QString& s2) const {
        return m_collator.compare(s1, s2);
    }

    int compare(const QStringRef& s1, const QStringRef& s2) const {
        return m_collator.compare(s1, s2);
    }

  private:
    QCollator m_collator;
};

// Helper to create html link strings to be used for ui files, mostly in
// Preferences dialogs.
inline QString coloredLinkString(
        const QColor& color,
        const QString& text,
        const QString& baseUrl,
        const QString& extUrl = nullptr) {
    return QStringLiteral("<a style=\"color:") + color.name() +
            QStringLiteral(";\" href=\"") + baseUrl + extUrl +
            QStringLiteral("\">") + text + QStringLiteral("</a>");
}

// Check if the extension from the file filter was added to the file base name.
// Otherwise add it manually.
// Works around https://bugreports.qt.io/browse/QTBUG-27186
inline QString filePathWithSelectedExtension(const QString& fileLocationInput,
        const QString& fileFilter) {
    if (fileLocationInput.isEmpty()) {
        return {};
    }
    QString fileLocation = fileLocationInput;
    if (fileFilter.isEmpty()) {
        return fileLocation;
    }

    // Extract 'ext' from QFileDialog file filter string 'Funky type (*.ext)'
    QRegularExpressionMatch extMatch = kExtRgxp.match(fileFilter);
    if (!extMatch.hasMatch()) {
        return fileLocation;
    }
    const QString ext = extMatch.captured(1);
    QFileInfo fileName(fileLocation);
    if (!ext.isEmpty() && fileName.suffix() != ext) {
        fileLocation.append(".").append(ext);
    }
    return fileLocation;
}
