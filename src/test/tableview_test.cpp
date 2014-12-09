// Tests for tableview-related things
// Right now it's just testing the serialize-unserialize of the header state code.
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>
#include "proto/headers.pb.h"
#include "widget/wtracktableviewheader.h"

class HeaderViewStateTest : public testing::Test {
};

TEST_F(HeaderViewStateTest, RoundTrip) {
    mixxx::library::HeaderViewState headerViewState_pb;
    mixxx::library::HeaderViewState::HeaderState* header_state_pb =
                headerViewState_pb.add_header_state();

    header_state_pb->set_hidden(true);
    header_state_pb->set_size(50);
    header_state_pb->set_logical_index(10);
    header_state_pb->set_visual_index(2);
    header_state_pb->set_column_id(1);

    header_state_pb =
                headerViewState_pb.add_header_state();

    header_state_pb->set_hidden(false);
    header_state_pb->set_size(22);
    header_state_pb->set_logical_index(6);
    header_state_pb->set_visual_index(3);
    header_state_pb->set_column_id(4);

    headerViewState_pb.set_sort_indicator_shown(true);
    headerViewState_pb.set_sort_indicator_section(1);
    headerViewState_pb.set_sort_order(Qt::DescendingOrder);

    QHeaderViewState view_state(QString::fromStdString(
            headerViewState_pb.SerializeAsString()));

    ASSERT_EQ(headerViewState_pb.SerializeAsString(),
              view_state.saveState().toStdString());
}
