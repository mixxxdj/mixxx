//
// Created by augier on 15/07/24.
//

#ifndef MIXXX_MIXXXSCREEN_H
#define MIXXX_MIXXXSCREEN_H

#include <QtQml/qqmlregistration.h>

#include <QImage>
#include <QObject>
#include <QSize>

namespace mixxx {
namespace qml {

class MixxxScreen : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString screenId MEMBER m_screenId REQUIRED)
    Q_PROPERTY(int width READ width WRITE setWidth)
    Q_PROPERTY(int height READ height WRITE setHeight)
    Q_PROPERTY(uint targetFps MEMBER m_targetFps)
    Q_PROPERTY(uint msaa MEMBER m_msaa)
    Q_PROPERTY(uint splashOff READ splashOff WRITE setSplashOff)
    Q_PROPERTY(QImage::Format pixelType MEMBER m_pixelType)
    Q_PROPERTY(ColorEndian endian MEMBER m_endian)
    Q_PROPERTY(bool reversedColor MEMBER m_reversedColor)
    Q_PROPERTY(bool rawData MEMBER m_rawData)

  public:
    enum class ColorEndian {
        Big = static_cast<int>(std::endian::big),
        Little = static_cast<int>(std::endian::little),
    };
    Q_ENUM(ColorEndian)

    int width();
    void setWidth(int value);
    int height();
    void setHeight(int value);
    uint splashOff();
    void setSplashOff(uint value);

  signals:
    void init();
    void shutdown();

  private:
    QString m_screenId;         // The screen identifier.
    QSize m_size = QSize(0, 0); // The size of the screen.
    uint m_targetFps = 30;      // The maximum FPS to render.
    uint m_msaa = 1;            // The MSAA value to use for render.
    std::chrono::milliseconds m_splashOff = std::chrono::milliseconds(
            3000); // The rendering grace time given when the screen is
                   // requested to shutdown.
    QImage::Format m_pixelType =
            QImage::Format_RGB888;              // The pixel encoding format.
    ColorEndian m_endian = ColorEndian::Little; // The pixel endian format.
    bool m_reversedColor = false;               // Whether or not the RGB is swapped BGR.
    bool m_rawData = false;                     // Whether or not the screen is allowed to receive
                                                // bare data, not transformed.
};

} // namespace qml
} // namespace mixxx

#endif // MIXXX_MIXXXSCREEN_H
