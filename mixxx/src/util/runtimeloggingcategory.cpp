#include "util/runtimeloggingcategory.h"

#include <QRegularExpression>

namespace {
const QRegularExpression invalidLoggingCategoryChars("[^a-zA-Z0-9]");
} // anonymous namespace

QString RuntimeLoggingCategory::removeInvalidCharsFromCategory(QString categoryName) {
    return categoryName.remove(invalidLoggingCategoryChars);
}
