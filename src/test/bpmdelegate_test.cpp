#include "library/tabledelegates/bpmdelegate.h"

#include <gtest/gtest.h>

#include <QDoubleSpinBox>
#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>
#include <QPointer>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <memory>

#include "test/mixxxtest.h"
#include "util/parented_ptr.h"

namespace mixxx {
namespace {

class DestructionTrackingEditorCreator : public QItemEditorCreatorBase {
  public:
    explicit DestructionTrackingEditorCreator(int* pDestructionCount)
            : m_pDestructionCount(pDestructionCount) {
    }

    ~DestructionTrackingEditorCreator() override {
        ++*m_pDestructionCount;
    }

    QWidget* createWidget(QWidget*) const override {
        return nullptr;
    }

    QByteArray valuePropertyName() const override {
        return QByteArray("value");
    }

  private:
    int* m_pDestructionCount;
};

class BPMDelegateTest : public MixxxTest {};

TEST_F(BPMDelegateTest, ReplacingDelegateDestroysFactoryCreator) {
    int destructionCount = 0;
    {
        QStandardItemModel model(1, 1);
        QTableView table;
        table.setModel(&model);
        auto pOldDelegate = std::make_unique<BPMDelegate>(&table);
        QPointer<BPMDelegate> oldDelegateGuard(pOldDelegate.get());
        ASSERT_NE(nullptr, pOldDelegate->itemEditorFactory());
        pOldDelegate->itemEditorFactory()->registerEditor(QMetaType::QString,
                std::make_unique<DestructionTrackingEditorCreator>(&destructionCount).release());
        table.setItemDelegateForColumn(0, pOldDelegate.get());
        EXPECT_EQ(&table, pOldDelegate->parent());
        EXPECT_EQ(0, destructionCount);

        auto pNewDelegate = make_parented<BPMDelegate>(&table);
        table.setItemDelegateForColumn(0, pNewDelegate.get());
        EXPECT_EQ(0, destructionCount);
        pOldDelegate.reset();

        EXPECT_TRUE(oldDelegateGuard.isNull());
        EXPECT_EQ(1, destructionCount);
        EXPECT_EQ(pNewDelegate.get(), table.itemDelegateForColumn(0));
        EXPECT_EQ(&table, pNewDelegate->parent());
    }
    EXPECT_EQ(1, destructionCount);
}

TEST_F(BPMDelegateTest, DestroyingTableDestroysFactoryCreator) {
    int destructionCount = 0;
    QPointer<BPMDelegate> delegateGuard;
    {
        QStandardItemModel model(1, 1);
        QTableView table;
        table.setModel(&model);
        auto pDelegate = make_parented<BPMDelegate>(&table);
        delegateGuard = pDelegate.get();
        ASSERT_NE(nullptr, pDelegate->itemEditorFactory());
        pDelegate->itemEditorFactory()->registerEditor(QMetaType::QString,
                std::make_unique<DestructionTrackingEditorCreator>(&destructionCount).release());
        table.setItemDelegateForColumn(0, pDelegate.get());

        EXPECT_EQ(&table, pDelegate->parent());
        EXPECT_EQ(0, destructionCount);
    }
    EXPECT_TRUE(delegateGuard.isNull());
    EXPECT_EQ(1, destructionCount);
}

// checking if the BPM input box still works
// after the memory leak issue resolved
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
