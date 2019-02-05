/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/

#include <QCoreApplication>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>

#include "musicbrainz/network.h"

NetworkAccessManager::NetworkAccessManager(QObject* parent)
                    : QNetworkAccessManager(parent) {
}

QNetworkReply* NetworkAccessManager::createRequest(Operation op,
                                                   const QNetworkRequest& request,
                                                   QIODevice* outgoingData) {
    QNetworkRequest new_request(request);
    new_request.setRawHeader("User-Agent", QString("%1 %2").arg(
        QCoreApplication::applicationName(),
        QCoreApplication::applicationVersion()).toUtf8());

    if (op == QNetworkAccessManager::PostOperation &&
        !new_request.header(QNetworkRequest::ContentTypeHeader).isValid()) {
        new_request.setHeader(QNetworkRequest::ContentTypeHeader,
                            "application/x-www-form-urlencoded");
    }

    // Prefer the cache unless the caller has changed the setting already
    if (request.attribute(QNetworkRequest::CacheLoadControlAttribute).toInt()
        == QNetworkRequest::PreferNetwork) {
        new_request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                                QNetworkRequest::PreferCache);
    }

    return QNetworkAccessManager::createRequest(op, new_request, outgoingData);
}


NetworkTimeouts::NetworkTimeouts(int timeout_msec, QObject* parent)
                : QObject(parent),
                  m_timeout_msec(timeout_msec) {
}

void NetworkTimeouts::addReply(QNetworkReply* reply) {
    if (m_timers.contains(reply))
        return;

    connect(reply, &QNetworkReply::destroyed, this, &NetworkTimeouts::replyFinished);
    connect(reply, &QNetworkReply::finished, this, &NetworkTimeouts::replyFinished);
    m_timers[reply] = startTimer(m_timeout_msec);
}

void NetworkTimeouts::replyFinished() {
    QNetworkReply* reply = reinterpret_cast<QNetworkReply*>(sender());
    if (m_timers.contains(reply)) {
        killTimer(m_timers.take(reply));
    }
}

void NetworkTimeouts::timerEvent(QTimerEvent* e) {
    QNetworkReply* reply = m_timers.key(e->timerId());
    if (reply) {
      reply->abort();
    }
}
