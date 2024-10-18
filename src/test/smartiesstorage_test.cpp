#include "library/trackset/smarties/smartiesstorage.h"

#include "library/trackset/smarties/smarties.h"
#include "test/librarytest.h"

class SmartiesStorageTest : public LibraryTest {
  protected:
    SmartiesStorageTest() {
        m_smartiesStorage.connectDatabase(dbConnection());
    }

    SmartiesStorage m_smartiesStorage;
};

TEST_F(SmartiesStorageTest, persistentLifecycle) {
    constexpr uint kNumSmarties = 10;

    // Insert some smarties
    for (auto i = kNumSmarties; i > 0; --i) {
        Smarties smarties;
        smarties.setName(QString("Smarties %1").arg(i));
        ASSERT_TRUE(m_smartiesStorage.onInsertingSmarties(smarties));
    }
    EXPECT_EQ(kNumSmarties, m_smartiesStorage.countSmarties());

    // Identify one of the "middle" smarties by name
    const auto kSmartiesIndex = (kNumSmarties + 1) / 2;
    const auto kSmartiesName = QString("Smarties %1").arg(kSmartiesIndex);
    SmartiesId smartiesId;

    // Find this smarties by name
    {
        Smarties smarties;
        ASSERT_TRUE(m_smartiesStorage.readSmartiesByName(kSmartiesName, &smarties));
        EXPECT_EQ(kSmartiesName, smarties.getName());
        smartiesId = smarties.getId();
        ASSERT_TRUE(smartiesId.isValid());
    }

    // Find this smarties smarties again, but now by id
    Smarties smarties;
    {
        ASSERT_TRUE(m_smartiesStorage.readSmartiesById(smartiesId, &smarties));
        EXPECT_EQ(smartiesId, smarties.getId());
        EXPECT_EQ(kSmartiesName, smarties.getName());
    }

    // Update the smarties's name
    const auto kNewSmartiesName = QString("New%1").arg(kSmartiesName);
    smarties.setName(kNewSmartiesName);
    ASSERT_TRUE(m_smartiesStorage.onUpdatingSmarties(smarties));
    // Reading the smarties by its old name should fail
    EXPECT_FALSE(m_smartiesStorage.readSmartiesByName(kSmartiesName));
    // Reading by id should reflect the updated name
    {
        Smarties updatedSmarties;
        EXPECT_TRUE(m_smartiesStorage.readSmartiesById(smartiesId, &updatedSmarties));
        EXPECT_EQ(kNewSmartiesName, updatedSmarties.getName());
    }

    // Finally delete this smarties
    ASSERT_TRUE(m_smartiesStorage.onDeletingSmarties(smartiesId));
    EXPECT_FALSE(m_smartiesStorage.readSmartiesById(smartiesId));
    EXPECT_FALSE(m_smartiesStorage.readSmartiesByName(kSmartiesName));
    EXPECT_FALSE(m_smartiesStorage.readSmartiesByName(kNewSmartiesName));
    EXPECT_EQ(kNumSmarties - 1, m_smartiesStorage.countSmarties());
}
