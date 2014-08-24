#include <gtest/gtest.h>
#include <clocale>
#include <QString>

#include "engine/enginefilterbiquad1.h"

namespace {

class EngineFilterBiquadTest : public testing::Test {
  protected:
    QString originalLocale;

    virtual void SetUp() {
        originalLocale = setlocale(LC_ALL, NULL);
    }
    virtual void TearDown() {
        setlocale(LC_ALL, originalLocale.toStdString().c_str());
    }
};

TEST_F(EngineFilterBiquadTest, fidlibInputRespectsLocale) {
    char spec[FIDSPEC_LENGTH];

    // Indonesia uses commas as a decimal separator.
    setlocale(LC_ALL, "Indonesian_Indonesia.1252");    
    format_fidspec(spec, sizeof(spec), "LsBq/%.10f/%.10f", 1.22, -12.0);
    ASSERT_STREQ("LsBq/1,2200000000/-12,0000000000", spec);
    // The reason this is important is that fidlib will do strtod on parts
    // of this string:
    ASSERT_DOUBLE_EQ(1.22, strtod("1,2200000000", NULL));
}

}
