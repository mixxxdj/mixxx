#include "waveformoverviewrenderer.h"

#include <QPainter>

#include "util/math.h"

QImage WaveformOverviewRenderer::render(ConstWaveformPointer pWaveform) const {
    const int dataSize = pWaveform->getDataSize();
    if (dataSize <= 0) {
        return QImage();
    }

    QImage image(dataSize / 2, 2 * 255, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0, 0, 0, 0).value());

    QPainter painter(&image);
    painter.translate(0.0, static_cast<double>(image.height()) / 2.0);

    QColor color;

    for (int i = 0, x = 0; i < dataSize; i += 2, ++x) {
        unsigned char all = pWaveform->getAll(i);
        unsigned char low = pWaveform->getLow(i);
        unsigned char mid = pWaveform->getMid(i);
        unsigned char high = pWaveform->getHigh(i);
        unsigned char max = math_max3(low, mid, high);
        qreal maxF = static_cast<qreal>(max);

        if (maxF > 0.0) {
            color.setRgbF(low / maxF, mid / maxF, high / maxF);
            painter.setPen(color);
            painter.drawLine(x, -all, x, 0);
        }

        all = pWaveform->getAll(i + 1);
        low = pWaveform->getLow(i + 1);
        mid = pWaveform->getMid(i + 1);
        high = pWaveform->getHigh(i + 1);
        max = math_max3(low, mid, high);
        maxF = static_cast<qreal>(max);

        if (maxF > 0.0) {
            color.setRgbF(low / maxF, mid / maxF, high / maxF);
            painter.setPen(color);
            painter.drawLine(x, 0, x, all);
        }
    }

    return image;
}
