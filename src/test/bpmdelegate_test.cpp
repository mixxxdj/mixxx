// AI-generated code.
// Reviewed by the author

#include "library/tabledelegates/bpmdelegate.h"

#include <gtest/gtest.h>

#include <QDoubleSpinBox>
#include <QItemEditorFactory>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <memory>

#include "test/mixxxtest.h"

namespace mixxx {
namespace {

class BPMDelegateTest : public MixxxTest {};

TEST_F(BPMDelegateTest, CreatesCustomBpmEditor) {
    QStandardItemModel model(1, 1);
    const QModelIndex index = model.index(0, 0);
    ASSERT_TRUE(model.setData(index, 123.45678901, Qt::EditRole));
    QTableView table;
    table.setModel(&model);
    BPMDelegate delegate(&table);
    const QStyleOptionViewItem option;
    std::unique_ptr<QWidget> pEditor(delegate.createEditor(table.viewport(), option, index));
    auto* pSpinBox = qobject_cast<QDoubleSpinBox*>(pEditor.get());
    ASSERT_NE(nullptr, pSpinBox);

    EXPECT_EQ(table.viewport(), pSpinBox->parentWidget());
    EXPECT_EQ(QStringLiteral("LibraryBPMSpinBox"), pSpinBox->objectName());
    EXPECT_FALSE(pSpinBox->hasFrame());
    EXPECT_DOUBLE_EQ(0.0, pSpinBox->minimum());
    EXPECT_DOUBLE_EQ(9999.0, pSpinBox->maximum());
    EXPECT_EQ(8, pSpinBox->decimals());
    EXPECT_DOUBLE_EQ(0.001, pSpinBox->singleStep());
    delegate.setEditorData(pEditor.get(), index);
    EXPECT_DOUBLE_EQ(123.45678901, pSpinBox->value());
}

TEST_F(BPMDelegateTest, RepeatedEditsReuseFactoryAndUpdateModel) {
    QStandardItemModel model(1, 1);
    const QModelIndex index = model.index(0, 0);
    double expectedBpm = 120.0;
    ASSERT_TRUE(model.setData(index, expectedBpm, Qt::EditRole));
    QTableView table;
    table.setModel(&model);
    BPMDelegate delegate(&table);
    auto* pFactory = delegate.itemEditorFactory();
    ASSERT_NE(nullptr, pFactory);
    const QStyleOptionViewItem option;

    for (const double editedBpm : {128.12345678, 0.0, 9999.0}) {
        std::unique_ptr<QWidget> pEditor(delegate.createEditor(table.viewport(), option, index));
        auto* pSpinBox = qobject_cast<QDoubleSpinBox*>(pEditor.get());
        ASSERT_NE(nullptr, pSpinBox);
        EXPECT_EQ(pFactory, delegate.itemEditorFactory());
        delegate.setEditorData(pEditor.get(), index);
        EXPECT_DOUBLE_EQ(expectedBpm, pSpinBox->value());

        pSpinBox->setValue(editedBpm);
        delegate.setModelData(pEditor.get(), &model, index);
        EXPECT_DOUBLE_EQ(editedBpm, model.data(index, Qt::EditRole).toDouble());
        expectedBpm = editedBpm;
    }
}

} // namespace
} // namespace mixxx

// AI-generated code.
// Reviewed by the author
