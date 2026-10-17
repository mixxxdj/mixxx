// AI-generated code.
// Reviewed by the author

#include "widget/wtracktableview.h"

#include <gtest/gtest.h>

#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>
#include <QPointer>
#include <QStringList>
#include <memory>
#include <utility>
#include <vector>

#include "control/controlobject.h"
#include "library/basesqltablemodel.h"
#include "library/dao/trackschema.h"
#include "library/tabledelegates/bpmdelegate.h"
#include "mixer/playerinfo.h"
#include "test/librarytest.h"

namespace mixxx {
namespace {

// Keeps the default Capability::None, so the WTrackMenu created by
// loadTrackModel() does not use the Library that these tests do not create.
class DelegateTestTableModel : public BaseSqlTableModel {
  public:
    DelegateTestTableModel(TrackCollectionManager* pTrackCollectionManager,
            const char* settingsNamespace,
            QStringList columns)
            : BaseSqlTableModel(nullptr, pTrackCollectionManager, settingsNamespace) {
        setTable(QStringLiteral("library"), LIBRARYTABLE_ID, std::move(columns), {});
    }

    bool isColumnInternal(int column) override {
        return column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID);
    }
};

class DelegateTestTableView : public WTrackTableView {
  public:
    explicit DelegateTestTableView(UserSettingsPointer pConfig)
            : WTrackTableView(nullptr, std::move(pConfig), nullptr, 1.0) {
    }

  protected:
    // WTrackTableView::selectionChanged() uses the Library.
    void selectionChanged(const QItemSelection& selected,
            const QItemSelection& deselected) override {
        QTableView::selectionChanged(selected, deselected);
    }
};

// Must outlive the observed delegates: their destroyed() handlers
// increment m_destructionCounts.
class ColumnDelegateObserver {
  public:
    explicit ColumnDelegateObserver(int columnCount)
            : m_delegates(columnCount),
              m_destructionCounts(columnCount, 0) {
    }

    void observe(const WTrackTableView& table) {
        ASSERT_EQ(m_delegates.size(), static_cast<size_t>(table.model()->columnCount()));
        for (size_t column = 0; column < m_delegates.size(); ++column) {
            auto* pDelegate = table.itemDelegateForColumn(static_cast<int>(column));
            ASSERT_NE(nullptr, pDelegate);
            m_delegates[column] = pDelegate;
            QObject::connect(pDelegate, &QObject::destroyed, [this, column] {
                ++m_destructionCounts[column];
            });
        }
    }

    void expectAlive(const WTrackTableView& table) const {
        for (size_t column = 0; column < m_delegates.size(); ++column) {
            SCOPED_TRACE(column);
            ASSERT_FALSE(m_delegates[column].isNull());
            EXPECT_EQ(&table, m_delegates[column]->parent());
            EXPECT_EQ(m_delegates[column].data(),
                    table.itemDelegateForColumn(static_cast<int>(column)));
            EXPECT_EQ(0, m_destructionCounts[column]);
        }
    }

    void expectDestroyed() const {
        for (size_t column = 0; column < m_delegates.size(); ++column) {
            SCOPED_TRACE(column);
            EXPECT_TRUE(m_delegates[column].isNull());
            EXPECT_EQ(1, m_destructionCounts[column]);
        }
    }

  private:
    std::vector<QPointer<QAbstractItemDelegate>> m_delegates;
    std::vector<int> m_destructionCounts;
};

// QItemEditorFactory is not a QObject, but it deletes its registered
// creators, so this creator reports when the factory is destroyed.
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

QStringList delegateTestColumns() {
    return {LIBRARYTABLE_ID,
            LIBRARYTABLE_TITLE,
            LIBRARYTABLE_RATING,
            LIBRARYTABLE_COMMENT,
            LIBRARYTABLE_KEY,
            LIBRARYTABLE_BPM};
}

void observeBpmFactory(const DelegateTestTableModel& model,
        const WTrackTableView& table,
        int* pDestructionCount) {
    const int bpmColumn = model.fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
    ASSERT_GE(bpmColumn, 0);
    auto* pDelegate = qobject_cast<BPMDelegate*>(table.itemDelegateForColumn(bpmColumn));
    ASSERT_NE(nullptr, pDelegate);
    ASSERT_NE(nullptr, pDelegate->itemEditorFactory());
    pDelegate->itemEditorFactory()->registerEditor(QMetaType::QString,
            new DestructionTrackingEditorCreator(pDestructionCount));
}

// WTrackTableView and the PlayerInfo singleton it uses require these
// controls.
class WTrackTableViewTest : public LibraryTest {
  protected:
    WTrackTableViewTest() {
        const ConfigKey keys[] = {
                {QStringLiteral("[Master]"), QStringLiteral("crossfader")},
                {QStringLiteral("[App]"), QStringLiteral("num_decks")},
                {QStringLiteral("[App]"), QStringLiteral("num_samplers")},
                {QStringLiteral("[App]"), QStringLiteral("num_preview_decks")},
                {QStringLiteral("[App]"), QStringLiteral("gui_tick_50ms_period_s")},
                {QStringLiteral("[Library]"), QStringLiteral("sort_column")},
                {QStringLiteral("[Library]"), QStringLiteral("sort_order")},
        };
        for (const auto& key : keys) {
            m_controls.push_back(std::make_unique<ControlObject>(key));
        }
        PlayerInfo::create();
    }

    ~WTrackTableViewTest() override {
        PlayerInfo::destroy();
    }

  private:
    std::vector<std::unique_ptr<ControlObject>> m_controls;
};

TEST_F(WTrackTableViewTest, DestroysColumnDelegatesBeforeQtParentCleanup) {
    DelegateTestTableModel model(trackCollectionManager(),
            "delegate-destruction-test",
            delegateTestColumns());
    ColumnDelegateObserver delegates(model.columnCount());
    QPointer<QAbstractItemDelegate> defaultDelegate;
    int factoryCreatorDestructionCount = 0;
    bool tableDestructionObserved = false;
    {
        DelegateTestTableView table(config());
        defaultDelegate = table.itemDelegate();
        ASSERT_FALSE(defaultDelegate.isNull());
        table.loadTrackModel(&model);
        ASSERT_NO_FATAL_FAILURE(delegates.observe(table));
        delegates.expectAlive(table);
        ASSERT_NO_FATAL_FAILURE(observeBpmFactory(model, table, &factoryCreatorDestructionCount));
        EXPECT_EQ(0, factoryCreatorDestructionCount);

        // QWidget emits destroyed() before deleting its children. By then the
        // column delegates must be gone, while Qt's default delegate, owned
        // only by the table, must still be alive.
        QObject::connect(&table, &QObject::destroyed, [&] {
            tableDestructionObserved = true;
            delegates.expectDestroyed();
            EXPECT_EQ(1, factoryCreatorDestructionCount);
            EXPECT_FALSE(defaultDelegate.isNull());
        });
    }
    EXPECT_TRUE(tableDestructionObserved);
    EXPECT_TRUE(defaultDelegate.isNull());
    delegates.expectDestroyed();
    EXPECT_EQ(1, factoryCreatorDestructionCount);
}

TEST_F(WTrackTableViewTest, SwitchingToSmallerModelDestroysAllOldColumnDelegates) {
    DelegateTestTableModel largeModel(trackCollectionManager(),
            "large-delegate-test",
            delegateTestColumns());
    DelegateTestTableModel smallModel(trackCollectionManager(),
            "small-delegate-test",
            {LIBRARYTABLE_ID, LIBRARYTABLE_TITLE});
    ASSERT_GT(largeModel.columnCount(), smallModel.columnCount());
    ColumnDelegateObserver firstDelegates(largeModel.columnCount());
    ColumnDelegateObserver smallDelegates(smallModel.columnCount());
    ColumnDelegateObserver lastDelegates(largeModel.columnCount());
    int firstFactoryCreatorDestructionCount = 0;
    int lastFactoryCreatorDestructionCount = 0;
    {
        DelegateTestTableView table(config());
        table.loadTrackModel(&largeModel);
        ASSERT_NO_FATAL_FAILURE(firstDelegates.observe(table));
        firstDelegates.expectAlive(table);
        ASSERT_NO_FATAL_FAILURE(
                observeBpmFactory(largeModel, table, &firstFactoryCreatorDestructionCount));

        // Includes the delegates of columns the small model does not have.
        table.loadTrackModel(&smallModel);
        firstDelegates.expectDestroyed();
        EXPECT_EQ(1, firstFactoryCreatorDestructionCount);
        ASSERT_NO_FATAL_FAILURE(smallDelegates.observe(table));
        smallDelegates.expectAlive(table);
        EXPECT_TRUE(table.findChildren<BPMDelegate*>().isEmpty());

        table.loadTrackModel(&largeModel);
        smallDelegates.expectDestroyed();
        firstDelegates.expectDestroyed();
        ASSERT_NO_FATAL_FAILURE(lastDelegates.observe(table));
        lastDelegates.expectAlive(table);
        ASSERT_NO_FATAL_FAILURE(
                observeBpmFactory(largeModel, table, &lastFactoryCreatorDestructionCount));
        EXPECT_EQ(0, lastFactoryCreatorDestructionCount);
    }
    firstDelegates.expectDestroyed();
    smallDelegates.expectDestroyed();
    lastDelegates.expectDestroyed();
    EXPECT_EQ(1, firstFactoryCreatorDestructionCount);
    EXPECT_EQ(1, lastFactoryCreatorDestructionCount);
}

TEST_F(WTrackTableViewTest, ReloadingSameModelKeepsColumnDelegates) {
    DelegateTestTableModel model(trackCollectionManager(),
            "delegate-reload-test",
            delegateTestColumns());
    ColumnDelegateObserver delegates(model.columnCount());
    int factoryCreatorDestructionCount = 0;
    {
        DelegateTestTableView table(config());
        table.loadTrackModel(&model);
        ASSERT_NO_FATAL_FAILURE(delegates.observe(table));
        delegates.expectAlive(table);
        ASSERT_NO_FATAL_FAILURE(observeBpmFactory(model, table, &factoryCreatorDestructionCount));

        // loadTrackModel() returns early for an unchanged model.
        table.loadTrackModel(&model);
        delegates.expectAlive(table);
        EXPECT_EQ(0, factoryCreatorDestructionCount);
    }
    delegates.expectDestroyed();
    EXPECT_EQ(1, factoryCreatorDestructionCount);
}

} // namespace
} // namespace mixxx

// AI-generated code.
// Reviewed by the author
