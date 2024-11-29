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
        smarties.setSearchInput(QString("searchInput %1").arg(i));
        smarties.setSearchSql(QString("searchSql %1").arg(i));
        smarties.setCondition1Field(QString("Condition1Field %1").arg(i));
        smarties.setCondition2Field(QString("Condition2Field %1").arg(i));
        smarties.setCondition3Field(QString("Condition3Field %1").arg(i));
        smarties.setCondition4Field(QString("Condition4Field %1").arg(i));
        smarties.setCondition5Field(QString("Condition5Field %1").arg(i));
        smarties.setCondition6Field(QString("Condition6Field %1").arg(i));
        smarties.setCondition7Field(QString("Condition7Field %1").arg(i));
        smarties.setCondition8Field(QString("Condition8Field %1").arg(i));
        smarties.setCondition9Field(QString("Condition9Field %1").arg(i));
        smarties.setCondition10Field(QString("Condition10Field %1").arg(i));
        smarties.setCondition11Field(QString("Condition11Field %1").arg(i));
        smarties.setCondition12Field(QString("Condition12Field %1").arg(i));
        smarties.setCondition1Operator(QString("Condition1Operator %1").arg(i));
        smarties.setCondition2Operator(QString("Condition2Operator %1").arg(i));
        smarties.setCondition3Operator(QString("Condition3Operator %1").arg(i));
        smarties.setCondition4Operator(QString("Condition4Operator %1").arg(i));
        smarties.setCondition5Operator(QString("Condition5Operator %1").arg(i));
        smarties.setCondition6Operator(QString("Condition6Operator %1").arg(i));
        smarties.setCondition7Operator(QString("Condition7Operator %1").arg(i));
        smarties.setCondition8Operator(QString("Condition8Operator %1").arg(i));
        smarties.setCondition9Operator(QString("Condition9Operator %1").arg(i));
        smarties.setCondition10Operator(QString("Condition10Operator %1").arg(i));
        smarties.setCondition11Operator(QString("Condition11Operator %1").arg(i));
        smarties.setCondition12Operator(QString("Condition12Operator %1").arg(i));
        smarties.setCondition1Value(QString("Condition1Value %1").arg(i));
        smarties.setCondition2Value(QString("Condition2Value %1").arg(i));
        smarties.setCondition3Value(QString("Condition3Value %1").arg(i));
        smarties.setCondition4Value(QString("Condition4Value %1").arg(i));
        smarties.setCondition5Value(QString("Condition5Value %1").arg(i));
        smarties.setCondition6Value(QString("Condition6Value %1").arg(i));
        smarties.setCondition7Value(QString("Condition7Value %1").arg(i));
        smarties.setCondition8Value(QString("Condition8Value %1").arg(i));
        smarties.setCondition9Value(QString("Condition9Value %1").arg(i));
        smarties.setCondition10Value(QString("Condition10Value %1").arg(i));
        smarties.setCondition11Value(QString("Condition11Value %1").arg(i));
        smarties.setCondition12Value(QString("Condition12Value %1").arg(i));
        smarties.setCondition1Combiner(QString("Condition1Combiner %1").arg(i));
        smarties.setCondition2Combiner(QString("Condition2Combiner %1").arg(i));
        smarties.setCondition3Combiner(QString("Condition3Combiner %1").arg(i));
        smarties.setCondition4Combiner(QString("Condition4Combiner %1").arg(i));
        smarties.setCondition5Combiner(QString("Condition5Combiner %1").arg(i));
        smarties.setCondition6Combiner(QString("Condition6Combiner %1").arg(i));
        smarties.setCondition7Combiner(QString("Condition7Combiner %1").arg(i));
        smarties.setCondition8Combiner(QString("Condition8Combiner %1").arg(i));
        smarties.setCondition9Combiner(QString("Condition9Combiner %1").arg(i));
        smarties.setCondition10Combiner(QString("Condition10Combiner %1").arg(i));
        smarties.setCondition11Combiner(QString("Condition11Combiner %1").arg(i));
        smarties.setCondition12Combiner(QString("Condition12Combiner %1").arg(i));

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
