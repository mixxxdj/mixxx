#include "clipboard.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

namespace {

QByteArray urlsToUtf8(const QList<QUrl>& urls) {
    QByteArray result;
    for (const QUrl& url : urls) {
        result += url.toEncoded();
        result += '\n';
    }
    if (!result.isEmpty()) {
        result.chop(1);
    }
    return result;
}

} // namespace

void Clipboard::start() {
    instance()->m_urls.clear();
}

void Clipboard::add(const QUrl& url) {
    instance()->m_urls.append(url);
}

void Clipboard::finish() {
    QMimeData* pData = new QMimeData;
    pData->setUrls(instance()->m_urls);
    // "x-special/gnome-copied-files" is used for many file managers
    // https://indigo.re/posts/2021-12-21-clipboard-data.html
    pData->setData(QStringLiteral("x-special/gnome-copied-files"),
            "copy\n" + urlsToUtf8(instance()->m_urls));
    QApplication::clipboard()->setMimeData(pData);
}

QList<QUrl> Clipboard::urls() {
    const QMimeData* data = QApplication::clipboard()->mimeData();
    return data ? data->urls() : QList<QUrl>();
}
