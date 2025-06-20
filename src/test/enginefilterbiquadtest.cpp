#include <gtest/gtest.h>

#include "engine/filters/enginefilterbiquad1.h"

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

TEST_F(EngineFilterBiquadTest, analysisPkBq) {
    char spec_d[FIDSPEC_LENGTH];
    format_fidspec(spec_d, sizeof(spec_d), "PkBq/%.10f/%.10f", 1.75, 0.0);

    char* pDesc = nullptr;
    FidFilter* filt = fid_design(spec_d, 44100, 1000, 0, 0, &pDesc);
    EXPECT_NE(pDesc, nullptr);
    free(pDesc);

    int delay = fid_calc_delay(filt);
    EXPECT_EQ(delay, 0);

    double resp0 = 0.0;
    double phase0 = 1.0;
    resp0 = fid_response_pha(filt, 1000 / 44100, &phase0);
    EXPECT_DOUBLE_EQ(resp0, 1.0);
    EXPECT_DOUBLE_EQ(phase0, 0.0);
    free(filt);
}

TEST_F(EngineFilterBiquadTest, analysisLpBe4) {
    char* pDesc = nullptr;
    FidFilter* filt = fid_design("LpBe4", 44100, 600, 0, 0, &pDesc);
    EXPECT_NE(pDesc, nullptr);
    free(pDesc);

    int delay = fid_calc_delay(filt);
    EXPECT_EQ(delay, 24);

    double resp0 = 0.0;
    double phase0 = 1.0;
    resp0 = fid_response_pha(filt, 600 / 44100, &phase0);
    EXPECT_DOUBLE_EQ(resp0, 1.0);
    EXPECT_DOUBLE_EQ(phase0, 0.0);
    free(filt);
}

} // namespace
