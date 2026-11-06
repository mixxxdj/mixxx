#include "preferences/colorpaletteeditormodel.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QMimeData>
#include <QStandardItem>
#include <memory>

#include "test/mixxxtest.h"
#include "util/color/rgbcolor.h"

namespace {

constexpr int kColorColumn = 0;
constexpr int kHotcueIndexColumn = 1;

const QColor kTestColors[] = {
        QColor(0xff, 0x00, 0x00),
        QColor(0x00, 0xff, 0x00),
        QColor(0x00, 0x00, 0xff),
        QColor(0xff, 0xff, 0x00),
};

/// Simulates dragging a row and dropping it above `destinationRow`, the way
/// QAbstractItemView does it for an internal move. Note that the dragged row is
/// removed from the model by QAbstractItemViewPrivate::clearOrRemove() after
/// the drop has been accepted.
void dragAndDropRow(
        ColorPaletteEditorModel* pModel, int sourceRow, int destinationRow) {
    const QModelIndexList indexes{
            pModel->index(sourceRow, kColorColumn),
            pModel->index(sourceRow, kHotcueIndexColumn),
    };
    const std::unique_ptr<QMimeData> pMimeData(pModel->mimeData(indexes));
    ASSERT_TRUE(pMimeData.get() != nullptr);
    ASSERT_TRUE(pModel->dropMimeData(pMimeData.get(),
            Qt::MoveAction,
            destinationRow,
            kColorColumn,
            QModelIndex()));
    ASSERT_TRUE(pModel->removeRows(sourceRow, 1, QModelIndex()));
}

} // namespace

class ColorPaletteEditorModelTest : public MixxxTest {};

TEST_F(ColorPaletteEditorModelTest, MoveRowKeepsAllItems) {
    ColorPaletteEditorModel model;
    model.setColumnCount(2);
    model.appendRow(kTestColors[0], {1});
    model.appendRow(kTestColors[1], {2});
    model.appendRow(kTestColors[2], {3});
    model.appendRow(kTestColors[3], {});
    ASSERT_EQ(model.rowCount(), 4);

    dragAndDropRow(&model, 0, 2);

    ASSERT_EQ(model.rowCount(), 4);
    for (int row = 0; row < model.rowCount(); ++row) {
        ASSERT_NE(model.item(row, kColorColumn), nullptr);
        ASSERT_NE(model.item(row, kHotcueIndexColumn), nullptr);
    }
    EXPECT_EQ(model.item(0, kColorColumn)->text(), kTestColors[1].name());
    EXPECT_EQ(model.item(1, kColorColumn)->text(), kTestColors[0].name());
    EXPECT_EQ(model.item(2, kColorColumn)->text(), kTestColors[2].name());
    EXPECT_EQ(model.item(3, kColorColumn)->text(), kTestColors[3].name());

    // The hotcue assignments are still intact, i.e. the moved row kept its
    // HotcueIndexListItem including the hotcue index list.
    EXPECT_EQ(model.item(0, kHotcueIndexColumn)->text(), QStringLiteral("2"));
    EXPECT_EQ(model.item(1, kHotcueIndexColumn)->text(), QStringLiteral("1"));
    EXPECT_EQ(model.item(2, kHotcueIndexColumn)->text(), QStringLiteral("3"));
}

TEST_F(ColorPaletteEditorModelTest, MoveRowToTheEnd) {
    ColorPaletteEditorModel model;
    model.setColumnCount(2);
    model.appendRow(kTestColors[0], {1});
    model.appendRow(kTestColors[1], {2});
    model.appendRow(kTestColors[2], {3});
    ASSERT_EQ(model.rowCount(), 3);

    dragAndDropRow(&model, 0, model.rowCount());

    ASSERT_EQ(model.rowCount(), 3);
    EXPECT_EQ(model.item(0, kColorColumn)->text(), kTestColors[1].name());
    EXPECT_EQ(model.item(1, kColorColumn)->text(), kTestColors[2].name());
    EXPECT_EQ(model.item(2, kColorColumn)->text(), kTestColors[0].name());
    EXPECT_EQ(model.item(2, kHotcueIndexColumn)->text(), QStringLiteral("1"));
}

TEST_F(ColorPaletteEditorModelTest, GetColorPaletteAfterMove) {
    ColorPaletteEditorModel model;
    model.setColumnCount(2);
    model.appendRow(kTestColors[0], {1});
    model.appendRow(kTestColors[1], {2});
    model.appendRow(kTestColors[2], {3});

    dragAndDropRow(&model, 0, 3);

    // This used to crash: the moved row had lost its HotcueIndexListItem, so
    // the hotcue index column contained a nullptr, which
    // toHotcueIndexListItem() dereferenced through the virtual type() call.
    const ColorPalette palette = model.getColorPalette(QStringLiteral("Test"));
    ASSERT_EQ(palette.size(), 3);
    EXPECT_EQ(mixxx::RgbColor::toQColor(palette.at(0)), kTestColors[1]);
    EXPECT_EQ(mixxx::RgbColor::toQColor(palette.at(1)), kTestColors[2]);
    EXPECT_EQ(mixxx::RgbColor::toQColor(palette.at(2)), kTestColors[0]);
}

TEST_F(ColorPaletteEditorModelTest, RowWithoutHotcueItemIsSkipped) {
    ColorPaletteEditorModel model;
    model.setColumnCount(2);
    model.appendRow(kTestColors[0], {1});
    // Simulate a corrupted row as it may be produced by a plain
    // QStandardItemModel drag & drop operation.
    model.insertRow(1,
            QList<QStandardItem*>{new QStandardItem(kTestColors[1].name())});

    const ColorPalette palette = model.getColorPalette(QStringLiteral("Test"));
    ASSERT_EQ(palette.size(), 1);
    EXPECT_EQ(mixxx::RgbColor::toQColor(palette.at(0)), kTestColors[0]);
}

TEST_F(ColorPaletteEditorModelTest, DropRejectsForeignData) {
    ColorPaletteEditorModel model;
    model.setColumnCount(2);
    model.appendRow(kTestColors[0], {1});
    model.appendRow(kTestColors[1], {2});

    QMimeData foreignData;
    foreignData.setText(QStringLiteral("not from this model"));
    EXPECT_FALSE(model.dropMimeData(
            &foreignData, Qt::MoveAction, 0, kColorColumn, QModelIndex()));
    EXPECT_FALSE(model.dropMimeData(
            nullptr, Qt::MoveAction, 0, kColorColumn, QModelIndex()));
    EXPECT_EQ(model.rowCount(), 2);
}

TEST_F(ColorPaletteEditorModelTest, SetColorAndEditHotcueIndices) {
    ColorPaletteEditorModel model;
    model.setColumnCount(2);
    model.appendRow(kTestColors[0], {1});
    model.appendRow(kTestColors[1], {});

    model.setColor(0, kTestColors[2]);
    EXPECT_EQ(model.item(0, kColorColumn)->text(), kTestColors[2].name());

    // Assigning an already used hotcue index frees it from the other row.
    ASSERT_TRUE(model.setData(model.index(1, kHotcueIndexColumn), QStringLiteral("1")));
    EXPECT_EQ(model.item(0, kHotcueIndexColumn)->text(), QString());
    EXPECT_EQ(model.item(1, kHotcueIndexColumn)->text(), QStringLiteral("1"));
}
