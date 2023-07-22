#include "clipboard.h"

#include <QApplication>
#include <QClipboard>

void Clipboard::start() {
    instance()->m_text.clear();
}

void Clipboard::add(const QUrl& url) {
    instance()->m_text.append(url.toString() + "\n");
}

void Clipboard::finish() {
    QApplication::clipboard()->setText(instance()->m_text);
}

QList<QUrl> Clipboard::urls() {
    const QString text = QApplication::clipboard()->text();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList strings = text.split("\n", Qt::SkipEmptyParts);
#else
    QStringList strings = text.split("\n", QString::SkipEmptyParts);
#endif
    QList<QUrl> result;
    for (const QString& string : strings) {
        result.append(QUrl(string));
    }
    return result;
}
