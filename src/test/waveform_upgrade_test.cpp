#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

#include <QDebug>
#include <tuple>

// Importing CPP file since testing function in anonymous namespace
#include "preferences/configobject.h"
#include "preferences/upgrade.cpp"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

class UpgradeTest : public testing::Test {
  protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST_F(UpgradeTest, useCorrectWaveformType) {
    struct test_case {
        QString waveformName;
        int oldTypeId;
        WaveformWidgetType::Type expectedType;
        WaveformWidgetBackend expectedBackend;
        WaveformRendererSignalBase::Options expectedOptions;
    };

    QList<test_case> testCases = {
            test_case{"EmptyWaveform",
                    0,
                    WaveformWidgetType::Empty,
                    WaveformWidgetBackend::None,
                    WaveformRendererSignalBase::Option::None},
            test_case{"SoftwareWaveform",
                    2, //  Filtered
                    WaveformWidgetType::Filtered,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"QtSimpleWaveform",
                    3, //  Simple Qt
                    WaveformWidgetType::Simple,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"QtWaveform",
                    4, //  Filtered Qt
                    WaveformWidgetType::Filtered,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLSimpleWaveform",
                    5, //  Simple GL
                    WaveformWidgetType::Simple,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLFilteredWaveform",
                    6, //  Filtered GL
                    WaveformWidgetType::Filtered,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLSLFilteredWaveform",
                    7, //  Filtered GLSL
                    WaveformWidgetType::Filtered,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::HighDetail},
            test_case{"HSVWaveform",
                    8, //  HSV
                    WaveformWidgetType::HSV,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLVSyncTest",
                    9, //  VSync GL
                    WaveformWidgetType::VSyncTest,
                    WaveformWidgetBackend::None,
                    WaveformRendererSignalBase::Option::None},
            test_case{"RGBWaveform",
                    10, // RGB
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLRGBWaveform",
                    11, // RGB GL
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLSLRGBWaveform",
                    12, // RGB GLSL
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::HighDetail},
            test_case{"QtVSyncTest",
                    13, // VSync Qt
                    WaveformWidgetType::VSyncTest,
                    WaveformWidgetBackend::None,
                    WaveformRendererSignalBase::Option::None},
            test_case{"QtHSVWaveform",
                    14, // HSV Qt
                    WaveformWidgetType::HSV,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"QtRGBWaveform",
                    15, // RGB Qt
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"GLSLRGBStackedWaveform",
                    16, // RGB Stacked
                    WaveformWidgetType::Stacked,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::HighDetail},
            test_case{"AllShaderRGBWaveform",
                    17, // RGB (all-shaders)
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"AllShaderLRRGBWaveform",
                    18, // L/R RGB (all-shaders)
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::SplitStereoSignal},
            test_case{"AllShaderFilteredWaveform",
                    19, // Filtered (all-shaders)
                    WaveformWidgetType::Filtered,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"AllShaderSimpleWaveform",
                    20, // Simple (all-shaders)
                    WaveformWidgetType::Simple,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"AllShaderHSVWaveform",
                    21, // HSV (all-shaders)
                    WaveformWidgetType::HSV,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"AllShaderTexturedFiltered",
                    22, // Filtered (textured) (all-shaders)
                    WaveformWidgetType::Filtered,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::HighDetail},
            test_case{"AllShaderTexturedRGB",
                    23, // RGB (textured) (all-shaders)
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::HighDetail},
            test_case{"AllShaderTexturedStacked",
                    24, // Stacked (textured) (all-shaders)
                    WaveformWidgetType::Stacked,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::HighDetail},
            test_case{"AllShaderRGBStackedWaveform",
                    26, // Stacked (all-shaders)
                    WaveformWidgetType::Stacked,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None},
            test_case{"Count_WaveformwidgetType",
                    27, //    Also used as invalid value
                    WaveformWidgetType::RGB,
                    WaveformWidgetBackend::AllShader,
                    WaveformRendererSignalBase::Option::None}};

    for (const auto& testCase : testCases) {
        int waveformType = testCase.oldTypeId;
        int waveformBackend = -1;
        int waveformOptions = -1;

        qDebug() << "Testing upgrade for" << testCase.waveformName;

        auto [type, backend, options] = upgradeToAllShaders(
                waveformType, waveformBackend, waveformOptions);
        ASSERT_EQ(type, testCase.expectedType);
        ASSERT_EQ(backend, testCase.expectedBackend);
        ASSERT_EQ(options, testCase.expectedOptions);
    }
}
