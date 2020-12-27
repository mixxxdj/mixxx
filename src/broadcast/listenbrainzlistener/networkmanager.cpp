#include "broadcast/listenbrainzlistener/networkmanager.h"

#include <iostream>

#include "broadcast/listenbrainzlistener/networkreply.h"
#include "broadcast/listenbrainzlistener/networkrequest.h"
#include "moc_networkmanager.cpp"

NetworkReply* FakeNetworkManager::post(const NetworkRequest* request, const QByteArray& data) {
    NetworkReply* reply = new FakeNetworkReply;
    FakeNetworkReply* fakeReply = qobject_cast<FakeNetworkReply*>(reply);
    fakeReply->setNetworkError(QNetworkReply::NoError);
    fakeReply->setHttpError(200);
    qDebug() << "Fake network manager sending POST request.";
    qDebug() << "Headers:";
    const QList<QByteArray> headerList = request->rawHeaderList();
    for (const auto& header : headerList) {
        qDebug() << header;
    }
    qDebug() << "POST Body:";
    qDebug() << data;
    return reply;
}
