//
// Created by augier on 15/07/24.
//

#ifndef MIXXX_MIXXXSCREEN_H
#define MIXXX_MIXXXSCREEN_H

#include <QtQml/qqmlregistration.h>

#include <QImage>
#include <QObject>
#include <QQuickItem>
#include <QSize>

namespace mixxx {
namespace qml {

class MixxxScreen : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

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

    Q_PROPERTY(QQuickItem item MEMBER m_item)

    Q_CLASSINFO("DefaultProperty", "item")

  public:
    enum class ColorEndian {
        Big = static_cast<int>(std::endian::big),
        Little = static_cast<int>(std::endian::little),
    };
    Q_ENUM(ColorEndian)

    void classBegin() override;
    void componentComplete() override;

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
    // The screen identifier.
    QString m_screenId;
    // The size of the screen.
    QSize m_size = QSize(0, 0);
    // The maximum FPS to render.
    uint m_targetFps = 30;
    // The MSAA value to use for render.
    uint m_msaa = 1;
    // The rendering grace time given when the
    // screen is requested to shutdown.
    std::chrono::milliseconds m_splashOff = std::chrono::milliseconds(
            3000);
    // The pixel encoding format.
    QImage::Format m_pixelType =
            QImage::Format_RGB888;
    // The pixel endian format.
    ColorEndian m_endian = ColorEndian::Little;
    // Whether or not the RGB is swapped BGR.
    bool m_reversedColor = false;
    // Whether or not the screen is allowed to receive bare data,
    // not transformed.
    bool m_rawData = false;
    // The item to render
    QQuickItem m_item;
    // Transform function
    std::function<QVariant(const QByteArray, const QDateTime&)> transform =
            [](const QByteArray input, const QDateTime& timestamp) {
                return QVariant(input);
            };

    inline static QByteArray kScreenTransformFunctionUntypedSignature =
            QMetaObject::normalizedSignature(
                    "transformFrame(QVariant,QVariant)");
    inline static QByteArray kScreenTransformFunctionTypedSignature =
            QMetaObject::normalizedSignature("transformFrame(QVariant,QDateTime)");

    QVariant transform(QMetaMethod transformMethod,
            const QByteArray input,
            const QDateTime& timestamp,
            bool typed);
};

} // namespace qml
} // namespace mixxx

#endif // MIXXX_MIXXXSCREEN_H
