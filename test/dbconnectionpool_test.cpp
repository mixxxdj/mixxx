#include <gtest/gtest.h>

#include "test/mixxxtest.h"

#include "database/mixxxdb.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"

#include "library/dao/settingsdao.h"

#include "util/assert.h"


class DbConnectionPoolTest : public MixxxTest {
  protected:
    DbConnectionPoolTest()
            : m_mixxxDb(config()) {
    }

  protected:
    const MixxxDb m_mixxxDb;
};

TEST_F(DbConnectionPoolTest, MoveSemantics) {
    mixxx::DbConnectionPooler p1(m_mixxxDb.connectionPool());
    EXPECT_TRUE(p1.isPooling());

    // Move construction
    mixxx::DbConnectionPooler p2(std::move(p1));
    EXPECT_FALSE(p1.isPooling());
    EXPECT_TRUE(p2.isPooling());

    // Move assignment
    p1 = std::move(p2);
    EXPECT_TRUE(p1.isPooling());
    EXPECT_FALSE(p2.isPooling());
}
