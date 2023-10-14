#include "clipboard.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

void Clipboard::start() {
    instance()->m_urls.clear();
}

void Clipboard::add(const QUrl& url) {
    instance()->m_urls.append(url);
}

void Clipboard::finish() {
    QMimeData* data = new QMimeData;
    data->setUrls(instance()->m_urls);
    QApplication::clipboard()->setMimeData(data);
}

QList<QUrl> Clipboard::urls() {
    const QMimeData* data = QApplication::clipboard()->mimeData();
    return data ? data->urls() : QList<QUrl>();
}
