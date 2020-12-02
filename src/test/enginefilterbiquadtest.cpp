#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "engine/filters/enginefilterbiquad1.h"
#include "engine/filters/enginefilteriir.h"
#include "gtest/gtest_pred_impl.h"

namespace {

class EngineFilterBiquadTest : public testing::Test {
};

TEST_F(EngineFilterBiquadTest, fidlibInputRespectsLocale) {
    char spec[FIDSPEC_LENGTH];

    format_fidspec(spec, sizeof(spec), "%.10f", 1.22);
    ASSERT_DOUBLE_EQ(1.22, strtod(spec, NULL));
}

TEST_F(EngineFilterBiquadTest, fidspecLengthIsLongEnough) {
    // negative sign adds an extra char
    ASSERT_TRUE(FIDSPEC_LENGTH > strlen("LsBq/1.2200000000/-12.0000000000"));
}

}
