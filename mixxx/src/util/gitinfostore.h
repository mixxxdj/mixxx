#pragma once

#include <QString>

namespace GitInfoStore {
QString branch();
QString describe();
QString date();
int commitCount();
bool dirty();
} // namespace GitInfoStore
