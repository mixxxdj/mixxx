#include "vinylcontrol/vinylcontrolsignalwidget.h"

#include "moc_vinylcontrolsignalwidget.cpp"

VinylControlSignalWidget::VinylControlSignalWidget()
        : QWidget(),
          m_iVinylInput(-1),
          m_iSize(MIXXX_VINYL_SCOPE_SIZE),
          m_qImage(),
          m_imageData(nullptr),
          m_iAngle(0),
          m_fSignalQuality(0.0f),
          m_bVinylActive(false) {
}

void VinylControlSignalWidget::setSize(int size) {
    m_iSize = size;
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    setMinimumSize(size, size);
    setMaximumSize(size, size);
    m_imageData = new uchar[size * size * 4];
    m_qImage = QImage(m_imageData, size, size, 0, QImage::Format_ARGB32);
}

void VinylControlSignalWidget::setVinylInput(int input) {
    m_iVinylInput = input;
}

VinylControlSignalWidget::~VinylControlSignalWidget() {
    delete [] m_imageData;
}

void VinylControlSignalWidget::setVinylActive(bool active)
{
    if (m_bVinylActive != active && !active) {
        resetWidget();
    }
    m_bVinylActive = active;
}

void VinylControlSignalWidget::onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) {
    // If this is a signal quality update for an input we don't care about,
    // ignore.
    if (report.processor != m_iVinylInput) {
        return;
    }

    m_iAngle = static_cast<int>(report.angle);
    m_fSignalQuality = report.timecode_quality;

    int r,g,b;
    QColor qual_color = QColor();
    //color is related to signal quality
    //hsv:  s=1, v=1
    //h is the only variable.
    //h=0 is red, h=120 is green
    qual_color.setHsv((int)(120.0 * m_fSignalQuality), 255, 255);
    qual_color.getRgb(&r, &g, &b);

    if (m_imageData == nullptr) {
        return;
    }

    for (int x = 0; x < m_iSize; ++x) {
        for(int y = 0; y < m_iSize; ++y) {
            //XXX: endianness means this is backwards....
            //does this break on other platforms?
            m_imageData[4*(x+m_iSize*y)+0] = (uchar)b;
            m_imageData[4*(x+m_iSize*y)+1] = (uchar)g;
            m_imageData[4*(x+m_iSize*y)+2] = (uchar)r;
            m_imageData[4*(x+m_iSize*y)+3] = (uchar)report.scope[x+m_iSize*y];
        }
    }
    update();
}

void VinylControlSignalWidget::resetWidget()
{
    if (m_imageData != nullptr) {
        memset(m_imageData, 0, sizeof(uchar) * m_iSize * m_iSize * 4);
    }
}

void VinylControlSignalWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    int sizeX = this->width();
    int sizeY = this->height();

    QPainter painter(this);
    painter.fillRect(this->rect(), QBrush(QColor(0, 0, 0)));

    if (m_bVinylActive) { //if timer is stopped, only draw the BG
        //main axes
        painter.setPen(QColor(0, 255, 0));
        painter.drawLine(sizeX / 2, 0, sizeX / 2, sizeY);
        painter.drawLine(0, sizeY / 2, sizeX, sizeY / 2);

        //quarter axes
        painter.setPen(QColor(0, 127, 0));
        painter.drawLine(QLineF(sizeX * 0.25, 0, sizeX * 0.25, sizeY));
        painter.drawLine(QLineF(sizeX * 0.75, 0, sizeX * 0.75, sizeY));
        painter.drawLine(QLineF(0, sizeY * 0.25, sizeX, sizeY * 0.25));
        painter.drawLine(QLineF(0, sizeY * 0.75, sizeX, sizeY * 0.75));

        //sweep
        if (m_iAngle >= 0) {
            //sweep fades along with signal quality
            painter.setPen(QColor(255, 255, 255, (int)(127.0 * m_fSignalQuality)));
            painter.setBrush(QColor(255, 255, 255, (int)(127.0 * m_fSignalQuality)));
            painter.drawPie(0, 0, sizeX, sizeY, m_iAngle*16, 1*16);
        }

        if (!m_qImage.isNull()) {
            //vinyl signal -- thanks xwax!
            painter.drawImage(this->rect(), m_qImage);
        }
    }
}
