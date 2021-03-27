#include "util/imageutils.h"

#include <gtest/gtest.h>

#include <QtDebug>

namespace {

class ImageUtilsTest : public testing::Test {
  protected:
    static void fillImageWithColor(QImage& image, QRgb color) {
        for (int x = 0; x < image.width(); ++x) {
            for (int y = 0; y < image.height(); ++y) {
                image.setPixel(x, y, color);
            }
        }
    }
};

TEST_F(ImageUtilsTest, NullImageDigest) {
    EXPECT_EQ(mixxx::ImageDigest(), mixxx::digestImage(QImage()));
}

TEST_F(ImageUtilsTest, OneBitImageDigest) {
    EXPECT_FALSE(mixxx::digestImage(QImage(1, 1, QImage::Format_Mono)).isEmpty());
}

TEST_F(ImageUtilsTest, NullImageBackgroundColor) {
    EXPECT_EQ(QColor(), mixxx::extractImageBackgroundColor(QImage()));
}

TEST_F(ImageUtilsTest, SolidImageBackgroundColor) {
    QImage image(16, 16, QImage::Format_RGB32);
    QRgb color;

    // Black
    color = QRgb(0x000000);
    fillImageWithColor(image, color);
    EXPECT_EQ(QColor(color), mixxx::extractImageBackgroundColor(image));

    // White
    color = QRgb(0xFFFFFF);
    fillImageWithColor(image, color);
    EXPECT_EQ(QColor(color), mixxx::extractImageBackgroundColor(image));

    // Custom
    color = QRgb(0x84CA2F);
    fillImageWithColor(image, color);
    EXPECT_EQ(QColor(color), mixxx::extractImageBackgroundColor(image));
}

} // anonymous namespace
