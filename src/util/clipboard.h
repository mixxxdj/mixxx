#pragma once

#include <QList>
#include <QUrl>

#include "util/singleton.h"

class Clipboard : public Singleton<Clipboard> {
    QList<QUrl> m_urls;

  public:
    static QList<QUrl> urls();
    static void start();
    static void finish();
    static void add(const QUrl& url);
};
