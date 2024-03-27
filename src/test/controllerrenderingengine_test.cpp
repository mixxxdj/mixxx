#include "controllers/rendering/controllerrenderingengine.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDomDocument>
#include <QTest>

#include "controllers/legacycontrollermappingfilehandler.h"
#include "helpers/log_test.h"
#include "test/mixxxtest.h"

using ::testing::_;

class ControllerRenderingEngineTest : public MixxxTest {
  public:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        SETUP_LOG_CAPTURE();
    }

    QList<QImage::Format> supportedPixelFormat() const {
        return LegacyControllerMappingFileHandler::kSupportedPixelFormat.values();
    }
};

class MockRenderingEngine : public ControllerRenderingEngine {
  public:
    MockRenderingEngine(const LegacyControllerMapping::ScreenInfo& info)
            : ControllerRenderingEngine(info, nullptr){};
};

TEST_F(ControllerRenderingEngineTest, createValidRendererWithSupportedTypes) {
    for (auto pixelFormat : supportedPixelFormat()) {
        MockRenderingEngine screenTest(LegacyControllerMapping::ScreenInfo(
                "",               // identifier
                QSize(0, 0),      // size
                10,               // target_fps
                10,               // splash_off
                pixelFormat,      // pixelFormat
                std::endian::big, // endian
                false,            // reversedColor
                false             // rawData
                ));
        EXPECT_TRUE(screenTest.isValid());
        EXPECT_TRUE(screenTest.stop());
    }
}
