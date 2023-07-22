#pragma once

#include <QString>
#include <QUrl>

#include "util/singleton.h"

class Clipboard : public Singleton<Clipboard> {
    QString m_text;

  public:
    static QList<QUrl> urls();
    static void start();
    static void finish();
    static void add(const QUrl& url);
};
