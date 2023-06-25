#pragma once

#include <QString>
#include <QUrl>

class Clipboard {
    static QString& text();

  public:
    static QList<QUrl> urls();
    static void begin();
    static void end();
    static void add(const QUrl& url);
};
