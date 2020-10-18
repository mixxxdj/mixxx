#include <gtest/gtest.h>

#include "library/dao/settingsdao.h"
#include "test/mixxxdbtest.h"
#include "util/db/dbconnectionpooler.h"

class DbConnectionPoolTest : public MixxxTest {};

TEST_F(DbConnectionPoolTest, MoveSemantics) {
    mixxx::DbConnectionPooler p1(MixxxDb(config()).connectionPool());
    ASSERT_TRUE(p1.isPooling());

    // Move construction
    mixxx::DbConnectionPooler p2(std::move(p1));
    EXPECT_FALSE(p1.isPooling());
    EXPECT_TRUE(p2.isPooling());

    // Move assignment
    p1 = std::move(p2);
    EXPECT_TRUE(p1.isPooling());
    EXPECT_FALSE(p2.isPooling());
}
