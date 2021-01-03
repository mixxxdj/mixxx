#pragma once

#include <QDateTime>
#include <QVariant>

namespace mixxx {
namespace sqlite {

QDateTime readGeneratedTimestamp(const QVariant& value);

QVariant writeGeneratedTimestamp(const QDateTime& value);

} // namespace sqlite

} // namespace mixxx
