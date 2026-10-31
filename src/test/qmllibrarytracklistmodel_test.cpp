#include "qml/qmllibrarytracklistmodel.h"

#include <gtest/gtest.h>

#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QUrl>

#include "library/trackmodel.h"
#include "qml/qmllibrarytracklistcolumn.h"
#include "track/track.h"

namespace {

// The source model supplies controlled track resolution. Index mapping,
// cover metadata and cover URL behavior use the real Qt/Mixxx objects.
class CoverTrackSource final : public QStandardItemModel, public TrackModel {
  public:
    CoverTrackSource()
            : QStandardItemModel(1, 1),
              TrackModel(QSqlDatabase(), "cover-model-test") {
    }

    TrackPointer getTrack(const QModelIndex& index) const override {
        ++getTrackCalls;
        lastTrackIndex = index;
        return track;
    }
    TrackPointer getTrackByRef(const TrackRef&) const override {
        return track;
    }
    QUrl getTrackUrl(const QModelIndex&) const override {
        return {};
    }
    QString getTrackLocation(const QModelIndex&) const override {
        return {};
    }
    TrackId getTrackId(const QModelIndex&) const override {
        return {};
    }
    CoverInfo getCoverInfo(const QModelIndex&) const override {
        return {};
    }
    const QVector<int> getTrackRows(TrackId) const override {
        return {};
    }
    void search(const QString&) override {
    }
    const QString currentSearch() const override {
        return {};
    }
    bool isColumnInternal(int) override {
        return false;
    }
    bool isColumnHiddenByDefault(int) override {
        return false;
    }
    SortColumnId sortColumnIdFromColumnIndex(int) const override {
        return SortColumnId::Invalid;
    }
    int columnIndexFromSortColumnId(SortColumnId) const override {
        return -1;
    }
    QString modelKey(bool) const override {
        return QStringLiteral("cover-model-test");
    }
    bool updateTrackGenre(Track*, const QString&) const override {
        return false;
    }
#if defined(__EXTRA_METADATA__)
    bool updateTrackMood(Track*, const QString&) const override {
        return false;
    }
#endif
    void select() override {
        ++selectCalls;
    }

    TrackPointer track;
    mutable int getTrackCalls = 0;
    mutable QModelIndex lastTrackIndex;
    int selectCalls = 0;
};

class QmlLibraryTrackListModelCoverTest : public testing::Test {
  protected:
    QmlLibraryTrackListModelCoverTest()
            : column(nullptr,
                      QStringLiteral("Cover"),
                      1,
                      -1,
                      100.0,
                      -1.0,
                      nullptr,
                      mixxx::qml::QmlLibraryTrackListColumn::Role::Cover),
              model({&column}, &source) {
    }

    QVariant coverData() {
        const auto index = model.index(0, 0);
        EXPECT_TRUE(index.isValid());
        EXPECT_EQ(1, source.selectCalls);
        return model.data(index, mixxx::qml::QmlLibraryTrackListModel::CoverArt);
    }

    CoverTrackSource source;
    mixxx::qml::QmlLibraryTrackListColumn column;
    mixxx::qml::QmlLibraryTrackListModel model;
};

TEST_F(QmlLibraryTrackListModelCoverTest, UnresolvedTrackReturnsInvalidVariant) {
    ASSERT_FALSE(source.track);
    EXPECT_FALSE(coverData().isValid());
    EXPECT_EQ(1, source.getTrackCalls);
    EXPECT_EQ(source.index(0, 0), source.lastTrackIndex);
}

TEST_F(QmlLibraryTrackListModelCoverTest, ResolvedCoverReturnsEncodedUrl) {
    source.track = Track::newTemporary();
    ASSERT_TRUE(source.track);
    CoverInfoRelative cover;
    cover.source = CoverInfoRelative::USER_SELECTED;
    cover.type = CoverInfoRelative::FILE;
    cover.coverLocation = QStringLiteral("cover.png");
    source.track->setCoverInfo(cover);

    const auto value = coverData();
    ASSERT_TRUE(value.isValid());
    ASSERT_TRUE(value.canConvert<QUrl>());
    EXPECT_EQ(QUrl(QStringLiteral("image://mixxx/coverart/Y292ZXIucG5n")), value.toUrl());
    EXPECT_EQ(1, source.getTrackCalls);
    EXPECT_EQ(source.index(0, 0), source.lastTrackIndex);
}

TEST_F(QmlLibraryTrackListModelCoverTest, ResolvedTrackWithoutCoverReturnsInvalidVariant) {
    source.track = Track::newTemporary();
    ASSERT_TRUE(source.track);
    EXPECT_FALSE(coverData().isValid());
    EXPECT_EQ(1, source.getTrackCalls);
}

TEST_F(QmlLibraryTrackListModelCoverTest, InvalidIndexDoesNotResolveTrack) {
    EXPECT_FALSE(model.data({}, mixxx::qml::QmlLibraryTrackListModel::CoverArt).isValid());
    EXPECT_EQ(0, source.getTrackCalls);
}

} // namespace
