#include "library/trackset/searchcrate/searchcratestorage.h"

#include "library/trackset/searchcrate/searchcrate.h"
#include "test/librarytest.h"

class SearchCrateStorageTest : public LibraryTest {
  protected:
    SearchCrateStorageTest() {
        m_searchCrateStorage.connectDatabase(dbConnection());
    }

    SearchCrateStorage m_searchCrateStorage;
};

TEST_F(SearchCrateStorageTest, persistentLifecycle) {
    constexpr uint kNumSearchCrate = 10;

    // Insert some searchCrate
    for (auto i = kNumSearchCrate; i > 0; --i) {
        SearchCrate searchCrate;
        searchCrate.setName(QString("SearchCrate %1").arg(i));
        searchCrate.setSearchInput(QString("searchInput %1").arg(i));
        searchCrate.setSearchSql(QString("searchSql %1").arg(i));
        searchCrate.setCondition1Field(QString("Condition1Field %1").arg(i));
        searchCrate.setCondition2Field(QString("Condition2Field %1").arg(i));
        searchCrate.setCondition3Field(QString("Condition3Field %1").arg(i));
        searchCrate.setCondition4Field(QString("Condition4Field %1").arg(i));
        searchCrate.setCondition5Field(QString("Condition5Field %1").arg(i));
        searchCrate.setCondition6Field(QString("Condition6Field %1").arg(i));
        searchCrate.setCondition7Field(QString("Condition7Field %1").arg(i));
        searchCrate.setCondition8Field(QString("Condition8Field %1").arg(i));
        searchCrate.setCondition9Field(QString("Condition9Field %1").arg(i));
        searchCrate.setCondition10Field(QString("Condition10Field %1").arg(i));
        searchCrate.setCondition11Field(QString("Condition11Field %1").arg(i));
        searchCrate.setCondition12Field(QString("Condition12Field %1").arg(i));
        searchCrate.setCondition1Operator(QString("Condition1Operator %1").arg(i));
        searchCrate.setCondition2Operator(QString("Condition2Operator %1").arg(i));
        searchCrate.setCondition3Operator(QString("Condition3Operator %1").arg(i));
        searchCrate.setCondition4Operator(QString("Condition4Operator %1").arg(i));
        searchCrate.setCondition5Operator(QString("Condition5Operator %1").arg(i));
        searchCrate.setCondition6Operator(QString("Condition6Operator %1").arg(i));
        searchCrate.setCondition7Operator(QString("Condition7Operator %1").arg(i));
        searchCrate.setCondition8Operator(QString("Condition8Operator %1").arg(i));
        searchCrate.setCondition9Operator(QString("Condition9Operator %1").arg(i));
        searchCrate.setCondition10Operator(QString("Condition10Operator %1").arg(i));
        searchCrate.setCondition11Operator(QString("Condition11Operator %1").arg(i));
        searchCrate.setCondition12Operator(QString("Condition12Operator %1").arg(i));
        searchCrate.setCondition1Value(QString("Condition1Value %1").arg(i));
        searchCrate.setCondition2Value(QString("Condition2Value %1").arg(i));
        searchCrate.setCondition3Value(QString("Condition3Value %1").arg(i));
        searchCrate.setCondition4Value(QString("Condition4Value %1").arg(i));
        searchCrate.setCondition5Value(QString("Condition5Value %1").arg(i));
        searchCrate.setCondition6Value(QString("Condition6Value %1").arg(i));
        searchCrate.setCondition7Value(QString("Condition7Value %1").arg(i));
        searchCrate.setCondition8Value(QString("Condition8Value %1").arg(i));
        searchCrate.setCondition9Value(QString("Condition9Value %1").arg(i));
        searchCrate.setCondition10Value(QString("Condition10Value %1").arg(i));
        searchCrate.setCondition11Value(QString("Condition11Value %1").arg(i));
        searchCrate.setCondition12Value(QString("Condition12Value %1").arg(i));
        searchCrate.setCondition1Combiner(QString("Condition1Combiner %1").arg(i));
        searchCrate.setCondition2Combiner(QString("Condition2Combiner %1").arg(i));
        searchCrate.setCondition3Combiner(QString("Condition3Combiner %1").arg(i));
        searchCrate.setCondition4Combiner(QString("Condition4Combiner %1").arg(i));
        searchCrate.setCondition5Combiner(QString("Condition5Combiner %1").arg(i));
        searchCrate.setCondition6Combiner(QString("Condition6Combiner %1").arg(i));
        searchCrate.setCondition7Combiner(QString("Condition7Combiner %1").arg(i));
        searchCrate.setCondition8Combiner(QString("Condition8Combiner %1").arg(i));
        searchCrate.setCondition9Combiner(QString("Condition9Combiner %1").arg(i));
        searchCrate.setCondition10Combiner(QString("Condition10Combiner %1").arg(i));
        searchCrate.setCondition11Combiner(QString("Condition11Combiner %1").arg(i));
        searchCrate.setCondition12Combiner(QString("Condition12Combiner %1").arg(i));

        ASSERT_TRUE(m_searchCrateStorage.onInsertingSearchCrate(searchCrate));
    }
    EXPECT_EQ(kNumSearchCrate, m_searchCrateStorage.countSearchCrate());

    // Identify one of the "middle" searchCrate by name
    const auto kSearchCrateIndex = (kNumSearchCrate + 1) / 2;
    const auto kSearchCrateName = QString("SearchCrate %1").arg(kSearchCrateIndex);
    SearchCrateId searchCrateId;

    // Find this searchCrate by name
    {
        SearchCrate searchCrate;
        ASSERT_TRUE(m_searchCrateStorage.readSearchCrateByName(kSearchCrateName, &searchCrate));
        EXPECT_EQ(kSearchCrateName, searchCrate.getName());
        searchCrateId = searchCrate.getId();
        ASSERT_TRUE(searchCrateId.isValid());
    }

    // Find this searchCrate searchCrate again, but now by id
    SearchCrate searchCrate;
    {
        ASSERT_TRUE(m_searchCrateStorage.readSearchCrateById(searchCrateId, &searchCrate));
        EXPECT_EQ(searchCrateId, searchCrate.getId());
        EXPECT_EQ(kSearchCrateName, searchCrate.getName());
    }

    // Update the searchCrate's name
    const auto kNewSearchCrateName = QString("New%1").arg(kSearchCrateName);
    searchCrate.setName(kNewSearchCrateName);
    ASSERT_TRUE(m_searchCrateStorage.onUpdatingSearchCrate(searchCrate));
    // Reading the searchCrate by its old name should fail
    EXPECT_FALSE(m_searchCrateStorage.readSearchCrateByName(kSearchCrateName));
    // Reading by id should reflect the updated name
    {
        SearchCrate updatedSearchCrate;
        EXPECT_TRUE(m_searchCrateStorage.readSearchCrateById(searchCrateId, &updatedSearchCrate));
        EXPECT_EQ(kNewSearchCrateName, updatedSearchCrate.getName());
    }

    // Finally delete this searchCrate
    ASSERT_TRUE(m_searchCrateStorage.onDeletingSearchCrate(searchCrateId));
    EXPECT_FALSE(m_searchCrateStorage.readSearchCrateById(searchCrateId));
    EXPECT_FALSE(m_searchCrateStorage.readSearchCrateByName(kSearchCrateName));
    EXPECT_FALSE(m_searchCrateStorage.readSearchCrateByName(kNewSearchCrateName));
    EXPECT_EQ(kNumSearchCrate - 1, m_searchCrateStorage.countSearchCrate());
}
