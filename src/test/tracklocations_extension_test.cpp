#include <gtest/gtest.h>

#include <QFile>
#include <QSqlQuery>
#include <QString>
#include <QTemporaryDir>

#include "library/dao/trackschema.h"
#include "test/librarytest.h"

/// Tests for schema revision 41: track_locations CMRT extension columns.
///
/// Verifies that the four new columns (track_id, offset_samples,
/// quality_indicator, fingerprint_hash) were correctly added to
/// track_locations and that they can be read/written.
class TrackLocationsExtensionTest : public LibraryTest {
  protected:
    /// Add a temporary location row directly and return its track_locations.id.
    int addLocationRow(const QString& filename) {
        const QString path = m_tempDir.filePath(filename);
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.close();
        QSqlQuery q(dbConnection());
        q.prepare(QStringLiteral(
                "INSERT INTO track_locations "
                "(location, filename, directory, filesize, fs_deleted, needs_verification) "
                "VALUES (:loc, :fn, :dir, 0, 0, 0)"));
        q.bindValue(QStringLiteral(":loc"), path);
        q.bindValue(QStringLiteral(":fn"), filename);
        q.bindValue(QStringLiteral(":dir"), m_tempDir.path());
        EXPECT_TRUE(q.exec()) << q.lastError().text().toStdString();
        return q.lastInsertId().toInt();
    }

    QTemporaryDir m_tempDir;
};

TEST_F(TrackLocationsExtensionTest, ColumnsExist) {
    // PRAGMA table_info returns one row per column; column 1 = name.
    QSqlQuery q(dbConnection());
    ASSERT_TRUE(q.exec(QStringLiteral("PRAGMA table_info(track_locations)")));

    QStringList columnNames;
    while (q.next()) {
        columnNames << q.value(1).toString();
    }

    EXPECT_TRUE(columnNames.contains(TRACKLOCATIONSTABLE_TRACK_ID))
            << "track_id column missing from track_locations";
    EXPECT_TRUE(columnNames.contains(TRACKLOCATIONSTABLE_OFFSET_SAMPLES))
            << "offset_samples column missing from track_locations";
    EXPECT_TRUE(columnNames.contains(TRACKLOCATIONSTABLE_QUALITY_INDICATOR))
            << "quality_indicator column missing from track_locations";
    EXPECT_TRUE(columnNames.contains(TRACKLOCATIONSTABLE_FINGERPRINT_HASH))
            << "fingerprint_hash column missing from track_locations";
}

TEST_F(TrackLocationsExtensionTest, DefaultValues) {
    const int locId = addLocationRow(QStringLiteral("defaults_test.flac"));
    ASSERT_GT(locId, 0);

    QSqlQuery q(dbConnection());
    q.prepare(QStringLiteral(
            "SELECT track_id, offset_samples, quality_indicator, fingerprint_hash "
            "FROM track_locations WHERE id = :id"));
    q.bindValue(QStringLiteral(":id"), locId);
    ASSERT_TRUE(q.exec());
    ASSERT_TRUE(q.next());

    // track_id should be NULL by default (not yet linked to a library entry)
    EXPECT_TRUE(q.value(0).isNull());
    // offset_samples should default to 0.0
    EXPECT_DOUBLE_EQ(q.value(1).toDouble(), 0.0);
    // quality_indicator should default to 0
    EXPECT_EQ(q.value(2).toInt(), 0);
    // fingerprint_hash should be NULL by default
    EXPECT_TRUE(q.value(3).isNull());
}

TEST_F(TrackLocationsExtensionTest, WriteAndReadFingerprintHash) {
    const int locId = addLocationRow(QStringLiteral("fingerprint_test.flac"));
    ASSERT_GT(locId, 0);

    // 0x1234ABCD is just an example 32-bit SimHash value.
    const unsigned int testHash = 0x1234ABCD;

    QSqlQuery update(dbConnection());
    update.prepare(QStringLiteral(
            "UPDATE track_locations SET fingerprint_hash = :hash WHERE id = :id"));
    update.bindValue(QStringLiteral(":hash"), testHash);
    update.bindValue(QStringLiteral(":id"), locId);
    ASSERT_TRUE(update.exec());

    QSqlQuery select(dbConnection());
    select.prepare(QStringLiteral(
            "SELECT fingerprint_hash FROM track_locations WHERE id = :id"));
    select.bindValue(QStringLiteral(":id"), locId);
    ASSERT_TRUE(select.exec());
    ASSERT_TRUE(select.next());
    EXPECT_EQ(select.value(0).toUInt(), testHash);
}

TEST_F(TrackLocationsExtensionTest, WriteAndReadOffsetAndQuality) {
    const int locId = addLocationRow(QStringLiteral("offset_quality_test.stem.mp4"));
    ASSERT_GT(locId, 0);

    // 2112 is the typical AAC encoder delay in samples at 44100 Hz.
    const double expectedOffset = 2112.0;
    // Quality ranks: 0 = unknown, 1 = lossy, 2 = lossless, 3 = stem
    const int expectedQuality = 3;

    QSqlQuery update(dbConnection());
    update.prepare(QStringLiteral(
            "UPDATE track_locations "
            "SET offset_samples = :off, quality_indicator = :qual "
            "WHERE id = :id"));
    update.bindValue(QStringLiteral(":off"), expectedOffset);
    update.bindValue(QStringLiteral(":qual"), expectedQuality);
    update.bindValue(QStringLiteral(":id"), locId);
    ASSERT_TRUE(update.exec());

    QSqlQuery select(dbConnection());
    select.prepare(QStringLiteral(
            "SELECT offset_samples, quality_indicator FROM track_locations WHERE id = :id"));
    select.bindValue(QStringLiteral(":id"), locId);
    ASSERT_TRUE(select.exec());
    ASSERT_TRUE(select.next());
    EXPECT_DOUBLE_EQ(select.value(0).toDouble(), expectedOffset);
    EXPECT_EQ(select.value(1).toInt(), expectedQuality);
}

TEST_F(TrackLocationsExtensionTest, FingerprintLookupFindsMultipleMatches) {
    // Verify that two location rows sharing the same fingerprint hash are
    // both returned by a fingerprint-based lookup (simulating CMRT duplicate
    // detection across different encodings of the same track).
    const unsigned int sharedHash = 0xCAFEBABE;
    const int loc1 = addLocationRow(QStringLiteral("match1.flac"));
    const int loc2 = addLocationRow(QStringLiteral("match2.stem.mp4"));
    const int loc3 = addLocationRow(QStringLiteral("no_match.mp3"));
    ASSERT_GT(loc1, 0);
    ASSERT_GT(loc2, 0);
    ASSERT_GT(loc3, 0);

    QSqlQuery update(dbConnection());
    update.prepare(QStringLiteral(
            "UPDATE track_locations SET fingerprint_hash = :hash "
            "WHERE id = :id1 OR id = :id2"));
    update.bindValue(QStringLiteral(":hash"), sharedHash);
    update.bindValue(QStringLiteral(":id1"), loc1);
    update.bindValue(QStringLiteral(":id2"), loc2);
    ASSERT_TRUE(update.exec());

    QSqlQuery select(dbConnection());
    select.prepare(QStringLiteral(
            "SELECT id FROM track_locations WHERE fingerprint_hash = :hash"));
    select.bindValue(QStringLiteral(":hash"), sharedHash);
    ASSERT_TRUE(select.exec());

    QList<int> matchedIds;
    while (select.next()) {
        matchedIds << select.value(0).toInt();
    }
    EXPECT_EQ(matchedIds.size(), 2);
    EXPECT_TRUE(matchedIds.contains(loc1));
    EXPECT_TRUE(matchedIds.contains(loc2));
    EXPECT_FALSE(matchedIds.contains(loc3));
}
