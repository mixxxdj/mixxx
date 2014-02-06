#include <gtest/gtest.h>

#include <QTestEventList>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "widget/wtrackproperty.h"

class WTrackPropertyTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pTrackProperty.reset(new WTrackProperty("fake", config(), NULL));
    }

    QScopedPointer<WTrackProperty> m_pTrackProperty;
    QTestEventList m_Events;
    const char* m_pGroup;
};

TEST_F(WTrackPropertyTest, AlwaysTrueTest) {
    ASSERT_EQ(1.0, 1.0);
}
