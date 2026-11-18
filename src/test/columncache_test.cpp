#include "library/columncache.h"

#include <gtest/gtest.h>

#include <QStringList>

#include "control/controlobject.h"
#include "library/dao/trackschema.h"
#include "test/mixxxtest.h"
#include "track/keyutils.h"

namespace {

class ColumnCacheTest : public MixxxTest {
  protected:
    ColumnCacheTest()
            // ColumnCache reads the key notation through a ControlProxy, so
            // provide the control instead of letting it fall back to Invalid.
            : m_keyNotation(ConfigKey(
                      QStringLiteral("[Library]"), QStringLiteral("key_notation"))) {
        m_keyNotation.set(static_cast<double>(KeyUtils::KeyNotation::Lancelot));
    }

    ControlObject m_keyNotation;
};

// The temporary tables of the external library features Rekordbox, Traktor, Serato
// carry a textual "key" column but no numeric
// "key_id" to sort by. Sorting by key must not emit SQL referencing it: the
// query would fail as a whole and the view would silently show no tracks.
TEST_F(ColumnCacheTest, keySortFallsBackWhenKeyIdColumnIsMissing) {
    const ColumnCache columnCache(QStringList{
            LIBRARYTABLE_ID,
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_KEY});

    const int keyColumnIndex =
            columnCache.fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    ASSERT_GE(keyColumnIndex, 0);

    EXPECT_EQ(LIBRARYTABLE_KEY,
            columnCache.columnSortForFieldIndex(keyColumnIndex));
}

// Tables that do have the column keep sorting by the semantic key, so that the
// order follows the circle of fifths rather than the spelling.
TEST_F(ColumnCacheTest, keySortUsesKeyIdColumnWhenPresent) {
    const ColumnCache columnCache(QStringList{
            LIBRARYTABLE_ID,
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_KEY,
            LIBRARYTABLE_KEY_ID});

    const int keyColumnIndex =
            columnCache.fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    ASSERT_GE(keyColumnIndex, 0);

    const QString keySortSql =
            columnCache.columnSortForFieldIndex(keyColumnIndex);
    EXPECT_TRUE(keySortSql.startsWith(
            QStringLiteral("CASE ") + LIBRARYTABLE_KEY_ID))
            << keySortSql.toStdString();
}

} // namespace
