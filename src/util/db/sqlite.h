#pragma once

#include <QDateTime>
#include <QVariant>

namespace sqlite {

QDateTime readGeneratedTimestamp(const QVariant& value);

QVariant writeGeneratedTimestamp(const QDateTime& value);

} // namespace sqlite
