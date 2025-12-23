#include "widget/wdisplay.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "moc_wdisplay.cpp"
#include "skin/legacy/skincontext.h"
#include "widget/wpixmapstore.h"

WDisplay::WDisplay(QWidget * parent)
        : WWidget(parent),
          m_iCurrentPixmap(0),
          m_pPixmapBack(nullptr),
          m_bDisabledLoaded(false) {
    setPositions(0);
}

WDisplay::~WDisplay() {
    resetPositions();
}

void WDisplay::setup(const QDomNode& node, const SkinContext& context) {
    // Set background pixmap if available

    QDomElement backPathNode = context.selectElement(node, "BackPath");
    if (!backPathNode.isNull()) {
        setPixmapBackground(context.getPixmapSource(backPathNode),
                context.selectScaleMode(backPathNode, Paintable::DrawMode::Tile),
                context.getScaleFactor());
    }

    // Number of states
    setPositions(context.selectInt(node, "NumberStates"));

    // Load knob pixmaps
    QDomElement pathNode = context.selectElement(node, "Path");
    QString path = context.nodeToString(pathNode);
    // The implicit default in <1.12.0 was FIXED so we keep it for
    // backwards compatibility.
    Paintable::DrawMode pathMode =
            context.selectScaleMode(pathNode, Paintable::DrawMode::Fixed);
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        setPixmap(&m_pixmaps, i, context.makeSkinPath(path.arg(i)),
                  pathMode, context.getScaleFactor());
    }

    // See if disabled images is defined, and load them...
    QDomElement disabledNode = context.selectElement(node, "DisabledPath");
    if (!disabledNode.isNull()) {
        QString disabledPath = context.nodeToString(disabledNode);
        // The implicit default in <1.12.0 was FIXED so we keep it for
        // backwards compatibility.
        Paintable::DrawMode disabledMode =
                context.selectScaleMode(disabledNode, Paintable::DrawMode::Fixed);
        for (int i = 0; i < m_disabledPixmaps.size(); ++i) {
            setPixmap(&m_disabledPixmaps, i,
                      context.makeSkinPath(disabledPath.arg(i)),
                      disabledMode, context.getScaleFactor());
        }
        m_bDisabledLoaded = true;
    }
}

void WDisplay::setPositions(int iNoPos) {
    resetPositions();

    if (iNoPos < 0) {
        qWarning() << "Negative NumberStates for Display.";
        iNoPos = 0;
    }

    // QVector inserts NULLs for the new pixmaps.
    m_pixmaps.resize(iNoPos);
    m_disabledPixmaps.resize(iNoPos);
}

void WDisplay::resetPositions() {
    m_pPixmapBack.reset();
    m_pixmaps.resize(0);
    m_disabledPixmaps.resize(0);
}

void WDisplay::setPixmapBackground(const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (!m_pPixmapBack || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className() << objectName()
                 << "Error loading background pixmap:" << source.getPath();
    }
}

void WDisplay::setPixmap(
        QVector<PaintablePointer>* pPixmaps,
        int iPos,
        const QString& filename,
        Paintable::DrawMode mode,
        double scaleFactor) {
    if (iPos < 0 || iPos >= pPixmaps->size()) {
        return;
    }

    PixmapSource source(filename);
    PaintablePointer pPixmap = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (!pPixmap || pPixmap->isNull()) {
        qDebug() << metaObject()->className() << objectName()
                 << "Error loading pixmap:" << filename << "for" << iPos;
    } else {
        (*pPixmaps)[iPos] = pPixmap;
        if (mode == Paintable::DrawMode::Fixed) {
            setFixedSize(pPixmap->size());
        }
    }
}

int WDisplay::getPixmapForParameter(double dParameter) const {
    // When there are an even number of pixmaps by convention we want a value of
    // 0.5 to align to the lower of the two middle pixmaps. In Mixxx < 1.12.0 we
    // accomplished this by the below formula:
    // index = (m_value - 64.0/127.0) * (numPixmaps() - 1) + numPixmaps() / 2.0;

    // But it's just as good to use m_value * numPixmaps() - epsilon. Using
    // numPixmaps() instead of numPixmaps() - 1 ensures that every pixmap shares
    // an equal slice of the value. Using m_value * (numPixmaps() - 1) gives an
    // unequal slice of the value to the last pixmaps.

    // Example:
    // 3 pixmaps
    // m_value * numPixmaps()
    // idx: 0       1       2       3
    // val: 0.0 ... 0.3 ... 0.6 ... 1.0
    // Even distribution of value range, value 1 is out of bounds (3).

    // m_value * (numPixmaps() - 1)
    // idx: 0       1       2
    // val: 0.0 ... 0.5 ... 1.0
    // Pixmap 2 is only shown at value 1.

    // floor(m_value * (numPixmaps() - 1) + 0.5)
    // idx: 0       1        2
    // val: 0.0 ... 0.25 ... 0.75 ... 1.0
    // Pixmap 0 and Pixmap 2 only shown for 0.25 of value range

    // 4 pixmaps
    // m_value * numPixmaps()
    // idx: 0       1        2       3        4
    // val: 0.0 ... 0.25 ... 0.5 ... 0.75 ... 1.0
    // Even distribution of value range, value 1 is out of bounds (4).

    // Subtracting an epsilon prevents out of bound values at the end of the
    // range and biases the middle value towards the lower of the 2 center
    // pixmaps when there are an even number of pixmaps.
    return static_cast<int>(dParameter * numPixmaps() - 0.00001);
}

void WDisplay::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    int pixmap = getPixmapForParameter(dParameter);
    if (pixmap != m_iCurrentPixmap) {
        // paintEvent updates m_iCurrentPixmap.
        update();
    }
}

void WDisplay::paintEvent(QPaintEvent* /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(rect(), &p);
    }

    // If we are disabled, use the disabled pixmaps. If not, use the regular
    // pixmaps.
    const QVector<PaintablePointer>& pixmaps = (!isEnabled() && m_bDisabledLoaded) ?
            m_disabledPixmaps : m_pixmaps;

    if (pixmaps.empty()) {
        return;
    }

    int idx = getPixmapForParameter(getControlParameterDisplay());

    // onConnectedControlChanged uses this to detect no-ops but it does not
    // clamp so don't clamp.
    m_iCurrentPixmap = idx;

    // Clamp active pixmap index to valid ranges.
    if (idx < 0) {
        idx = 0;
    } else if (idx >= pixmaps.size()) {
        idx = pixmaps.size() - 1;
    }

    PaintablePointer pPixmap = pixmaps[idx];
    if (pPixmap) {
        pPixmap->draw(rect(), &p);
    }
}
