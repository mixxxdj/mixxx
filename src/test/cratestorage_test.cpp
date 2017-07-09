#include "test/librarytest.h"

#include "library/crate/cratestorage.h"


class CrateStorageTest : public LibraryTest {
  protected:
    CrateStorageTest() {
        m_crateStorage.connectDatabase(dbConnection());
    }

    CrateStorage m_crateStorage;
};

TEST_F(CrateStorageTest, persistentLifecycle) {
    const uint kNumCrates = 10;

    // Insert some crates
    for (auto i = kNumCrates; i > 0; --i) {
        Crate crate;
        crate.setName(QString("Crate %1").arg(i));
        ASSERT_TRUE(m_crateStorage.onInsertingCrate(crate));
    }
    EXPECT_EQ(kNumCrates, m_crateStorage.countCrates());

    // The "middle" crate
    const auto kCrateIndex = (kNumCrates + 1) / 2;
    const auto kCrateName = QString("Crate %1").arg(kCrateIndex);
    CrateId crateId;

    // Find crate by name
    Crate crateByName;
    {
        ASSERT_TRUE(m_crateStorage.readCrateByName(kCrateName, &crateByName));
        EXPECT_EQ(kCrateName, crateByName.getName());
        crateId = crateByName.getId();
        ASSERT_TRUE(crateId.isValid());
    }

    // Find crate by id
    Crate crateById;
    {
        ASSERT_TRUE(m_crateStorage.readCrateById(crateId, &crateById));
        EXPECT_EQ(crateId, crateById.getId());
        EXPECT_EQ(kCrateName, crateById.getName());
    }

    // Update crate name
    const auto kNewCrateName = QString("New%1").arg(kCrateName);
    Crate crateUpdated = crateById;
    crateUpdated.setName(kNewCrateName);
    ASSERT_TRUE(m_crateStorage.onUpdatingCrate(crateUpdated));
    EXPECT_TRUE(m_crateStorage.readCrateById(crateId));
    EXPECT_FALSE(m_crateStorage.readCrateByName(kCrateName));

    // Find crate by new name
    Crate crateByNewName;
    {
        ASSERT_TRUE(m_crateStorage.readCrateByName(kNewCrateName, &crateByNewName));
        EXPECT_EQ(crateId, crateByNewName.getId());
        EXPECT_EQ(kNewCrateName, crateByNewName.getName());
    }

    // Delete crate
    ASSERT_TRUE(m_crateStorage.onDeletingCrate(crateId));
    EXPECT_FALSE(m_crateStorage.readCrateById(crateId));
    EXPECT_FALSE(m_crateStorage.readCrateByName(kCrateName));
    EXPECT_FALSE(m_crateStorage.readCrateByName(kNewCrateName));
    EXPECT_EQ(kNumCrates - 1, m_crateStorage.countCrates());
}
