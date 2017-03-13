#ifndef REGEX_H
#define REGEX_H

#include <QRegExp>
#include <QStringList>
#include <QString>

class RegexUtils {
  public:
    static QString fileExtensionsRegex(QStringList extensions) {
        // Escape every extension appropriately
        for (int i = 0; i < extensions.size(); ++i) {
            extensions[i] = QRegExp::escape(extensions[i]);
        }
        // Turn the list into a "\\.(jpg|gif|etc)$" style regex string
        return QString("\\.(%1)$").arg(extensions.join("|"));
    }

  private:
    RegexUtils() {}
};


#endif /* REGEX_H */
