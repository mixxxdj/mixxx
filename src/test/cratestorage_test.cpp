#include "library/trackset/crate/cratestorage.h"

#include "library/trackset/crate/crate.h"
#include "test/librarytest.h"

class CrateStorageTest : public LibraryTest {
  protected:
    CrateStorageTest() {
        m_crateStorage.connectDatabase(dbConnection());
    }

    CrateStorage m_crateStorage;
};

TEST_F(CrateStorageTest, persistentLifecycle) {
    constexpr uint kNumCrates = 10;

    // Insert some crates
    for (auto i = kNumCrates; i > 0; --i) {
        Crate crate;
        crate.setName(QString("Crate %1").arg(i));
        ASSERT_TRUE(m_crateStorage.onInsertingCrate(crate));
    }
    EXPECT_EQ(kNumCrates, m_crateStorage.countCrates());

    // Identify one of the "middle" crates by name
    const auto kCrateIndex = (kNumCrates + 1) / 2;
    const auto kCrateName = QString("Crate %1").arg(kCrateIndex);
    CrateId crateId;

    // Find this crate by name
    {
        Crate crate;
        ASSERT_TRUE(m_crateStorage.readCrateByName(kCrateName, &crate));
        EXPECT_EQ(kCrateName, crate.getName());
        crateId = crate.getId();
        ASSERT_TRUE(crateId.isValid());
    }

    // Find this crate crate again, but now by id
    Crate crate;
    {
        ASSERT_TRUE(m_crateStorage.readCrateById(crateId, &crate));
        EXPECT_EQ(crateId, crate.getId());
        EXPECT_EQ(kCrateName, crate.getName());
    }

    // Update the crate's name
    const auto kNewCrateName = QString("New%1").arg(kCrateName);
    crate.setName(kNewCrateName);
    ASSERT_TRUE(m_crateStorage.onUpdatingCrate(crate));
    // Reading the crate by its old name should fail
    EXPECT_FALSE(m_crateStorage.readCrateByName(kCrateName));
    // Reading by id should reflect the updated name
    {
        Crate updatedCrate;
        EXPECT_TRUE(m_crateStorage.readCrateById(crateId, &updatedCrate));
        EXPECT_EQ(kNewCrateName, updatedCrate.getName());
    }

    // Finally delete this crate
    ASSERT_TRUE(m_crateStorage.onDeletingCrate(crateId));
    EXPECT_FALSE(m_crateStorage.readCrateById(crateId));
    EXPECT_FALSE(m_crateStorage.readCrateByName(kCrateName));
    EXPECT_FALSE(m_crateStorage.readCrateByName(kNewCrateName));
    EXPECT_EQ(kNumCrates - 1, m_crateStorage.countCrates());
}
