#include "util/imageutils.h"

#include <gtest/gtest.h>

#include <QtDebug>

namespace {

constexpr int kThumbnailSize = 64;
constexpr int kBlurRadius = 5;

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

TEST_F(ImageUtilsTest, NullImageBlurredThumbnail) {
    EXPECT_TRUE(mixxx::blurredThumbnail(QImage(), kThumbnailSize, kBlurRadius).isNull());
}

TEST_F(ImageUtilsTest, BlurredThumbnailIsSmallAndKeepsAspectRatio) {
    QImage image(400, 200, QImage::Format_RGB32);
    fillImageWithColor(image, QRgb(0x336699));
    const QImage blurred = mixxx::blurredThumbnail(image, kThumbnailSize, kBlurRadius);
    EXPECT_EQ(QSize(64, 32), blurred.size());
}

TEST_F(ImageUtilsTest, BlurredThumbnailKeepsSolidColor) {
    QImage image(100, 100, QImage::Format_RGB32);
    const QRgb color = qRgb(0x33, 0x66, 0x99);
    fillImageWithColor(image, color);
    const QImage blurred = mixxx::blurredThumbnail(image, kThumbnailSize, kBlurRadius);
    EXPECT_EQ(color, blurred.pixel(0, 0));
    EXPECT_EQ(color, blurred.pixel(32, 32));
    EXPECT_EQ(color, blurred.pixel(63, 63));
}

TEST_F(ImageUtilsTest, BlurredThumbnailSmoothsEdges) {
    // The left half of the image is black and the right half is white. After
    // blurring, the brightness must rise gradually from the left to the right.
    QImage image(128, 128, QImage::Format_RGB32);
    for (int x = 0; x < image.width(); ++x) {
        for (int y = 0; y < image.height(); ++y) {
            image.setPixel(x, y, x < image.width() / 2 ? qRgb(0, 0, 0) : qRgb(255, 255, 255));
        }
    }
    const QImage blurred = mixxx::blurredThumbnail(image, kThumbnailSize, kBlurRadius);
    const int row = blurred.height() / 2;
    EXPECT_LT(qRed(blurred.pixel(0, row)), 10);
    EXPECT_GT(qRed(blurred.pixel(blurred.width() - 1, row)), 245);
    const int leftOfEdge = qRed(blurred.pixel(blurred.width() / 2 - 4, row));
    const int rightOfEdge = qRed(blurred.pixel(blurred.width() / 2 + 3, row));
    EXPECT_GT(leftOfEdge, 20);
    EXPECT_LT(leftOfEdge, 128);
    EXPECT_GT(rightOfEdge, 128);
    EXPECT_LT(rightOfEdge, 235);
}

} // anonymous namespace
