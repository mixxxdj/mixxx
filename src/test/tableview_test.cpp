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
    header_state_pb->set_column_name("MyCol");

    header_state_pb =
                headerViewState_pb.add_header_state();

    header_state_pb->set_hidden(false);
    header_state_pb->set_size(22);
    header_state_pb->set_logical_index(6);
    header_state_pb->set_visual_index(3);
    header_state_pb->set_column_name("MyOtherCol");

    headerViewState_pb.set_sort_indicator_shown(true);
    headerViewState_pb.set_sort_indicator_section(1);
    headerViewState_pb.set_sort_order(Qt::DescendingOrder);

    // Create a HeaderViewState based on the proto.
    HeaderViewState view_state(headerViewState_pb);

    // Get a serialized form of the state.
    QString saved_state = view_state.saveState();

    // Initialize a new state object with the saved state.
    HeaderViewState loaded_state(saved_state);

    // Compare the old saved state with the new one.
    ASSERT_EQ(saved_state, loaded_state.saveState());

    // Ensure that the serialization is not bullshit.
    ASSERT_NE("", saved_state);
}
