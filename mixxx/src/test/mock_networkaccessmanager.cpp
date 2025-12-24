/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mock_networkaccessmanager.h"

#include <QUrlQuery>
#include <QtDebug>
#include <algorithm>

#include "moc_mock_networkaccessmanager.cpp"

using std::min;

using ::testing::MakeMatcher;
using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;
using ::testing::Return;

class RequestForUrlMatcher : public MatcherInterface<const QNetworkRequest&> {
  public:
    RequestForUrlMatcher(const QString& contains,
            const QMap<QString, QString>& expected_params)
            : m_contains(contains),
              m_expected_params(expected_params) {
    }
    virtual ~RequestForUrlMatcher() {
    }

    virtual bool Matches(const QNetworkRequest& req) const {
        const QUrl& url = req.url();

        if (!url.toString().contains(m_contains)) {
            return false;
        }

        QUrlQuery url_query(url);
        for (QMap<QString, QString>::const_iterator it = m_expected_params.constBegin();
                it != m_expected_params.constEnd();
                ++it) {
            if (!url_query.hasQueryItem(it.key()) ||
                    url_query.queryItemValue(it.key()) != it.value()) {
                return false;
            }
        }
        return true;
    }

    virtual bool MatchAndExplain(const QNetworkRequest& req, MatchResultListener* listener) const {
        *listener << "which is " << req.url().toString().toUtf8().constData();
        return Matches(req);
    }

    virtual void DescribeTo(::std::ostream* os) const {
        *os << "matches url";
    }

  private:
    QString m_contains;
    QMap<QString, QString> m_expected_params;
};

inline Matcher<const QNetworkRequest&> RequestForUrl(
        const QString& contains,
        const QMap<QString, QString>& params) {
    return MakeMatcher(new RequestForUrlMatcher(contains, params));
}

MockNetworkReply* MockNetworkAccessManager::ExpectGet(
        const QString& contains,
        const QMap<QString, QString>& expected_params,
        int status,
        const QByteArray& data) {
    MockNetworkReply* reply = new MockNetworkReply(data);
    reply->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);

    EXPECT_CALL(*this,
            createRequest(GetOperation,
                    RequestForUrl(contains, expected_params),
                    nullptr))
            .WillOnce(Return(reply));

    return reply;
}

MockNetworkReply::MockNetworkReply(const QByteArray& data /* = nullptr */)
        : m_data(data),
          m_pos(0) {
}

void MockNetworkReply::SetData(const QByteArray& data) {
    m_data = data;
    m_pos = 0;
}

void MockNetworkReply::abort() {
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, {});
    setError(OperationCanceledError, tr("Operation canceled"));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    emit error(OperationCanceledError);
#else
    emit errorOccurred(OperationCanceledError);
#endif
    emit finished();
}

qint64 MockNetworkReply::readData(char* data, qint64 size) {
    if (m_data.size() == m_pos) {
        return -1;
    }
    qint64 bytes_to_read = min(m_data.size() - m_pos, size);
    memcpy(data, m_data.constData() + m_pos, bytes_to_read);
    m_pos += bytes_to_read;
    return bytes_to_read;
}

qint64 MockNetworkReply::writeData(const char* data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);
    ADD_FAILURE() << "Something tried to write to a QNetworkReply";
    return -1;
}

void MockNetworkReply::Done() {
    setOpenMode(QIODevice::ReadOnly);
    emit finished();
}

void MockNetworkReply::setAttribute(QNetworkRequest::Attribute code, const QVariant& value) {
    QNetworkReply::setAttribute(code, value);
}
