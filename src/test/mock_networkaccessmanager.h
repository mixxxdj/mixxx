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

#ifndef MOCK_NETWORKACCESSMANAGER_H
#define MOCK_NETWORKACCESSMANAGER_H

#include <QByteArray>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "test_utils.h"
#include "gmock/gmock.h"

// Usage:
// Create a MockNetworkAccessManager.
// Call ExpectGet() with appropriate expectations and the data you want back.
// This will return a MockNetworkReply*. When you are ready for the reply to
// arrive, call MockNetworkReply::Done().

class MockNetworkReply : public QNetworkReply {
  Q_OBJECT
 public:
  MockNetworkReply();
  MockNetworkReply(const QByteArray& data);

  // Use these to set expectations.
  void SetData(const QByteArray& data);
  virtual void setAttribute(QNetworkRequest::Attribute code, const QVariant& value);

  // Call this when you are ready for the finished() signal.
  void Done();

 protected:
  MOCK_METHOD0(abort, void());
  virtual qint64 readData(char* data, qint64);
  virtual qint64 writeData(const char* data, qint64);

  QByteArray data_;
  qint64 pos_;
};


class MockNetworkAccessManager : public QNetworkAccessManager {
  Q_OBJECT
 public:
  MockNetworkReply* ExpectGet(
      const QString& contains,  // A string that should be present in the URL.
      const QMap<QString, QString>& params,  // Required URL parameters.
      int status,  // Returned HTTP status code.
      const QByteArray& ret_data);  // Returned data.
 protected:
  MOCK_METHOD3(createRequest, QNetworkReply*(Operation, const QNetworkRequest&, QIODevice*));
};

#endif
