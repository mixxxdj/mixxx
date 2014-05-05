#include <QApplication>
#include <QBitmap>
#include <QLabel>
#include <QPainter>

#include "wcoverart.h"
#include "wskincolor.h"

WCoverArt::WCoverArt(QWidget* parent,
                     ConfigObject<ConfigValue>* pConfig)
        : QWidget(parent),
          WBaseWidget(this),
          m_pConfig(pConfig),
          m_bCoverIsHovered(false),
          m_bCoverIsVisible(false) {
    m_defaultCover = QImage(":/images/library/vinyl-record.png");
    m_currentCover = m_defaultCover;
}

WCoverArt::~WCoverArt() {
}

void WCoverArt::setup(QDomNode node, const SkinContext& context) {
    Q_UNUSED(node);
    setMouseTracking(TRUE);

    // Background color
    QColor bgc(255,255,255);
    if (context.hasNode(node, "BgColor")) {
        bgc.setNamedColor(context.selectString(node, "BgColor"));
        setAutoFillBackground(true);
    }
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), WSkinColor::getCorrectColor(bgc));

    // Foreground color
    QColor m_fgc(0,0,0);
    if (context.hasNode(node, "FgColor")) {
        m_fgc.setNamedColor(context.selectString(node, "FgColor"));
    }
    bgc = WSkinColor::getCorrectColor(bgc);
    m_fgc = QColor(255 - bgc.red(), 255 - bgc.green(), 255 - bgc.blue());
    pal.setBrush(foregroundRole(), m_fgc);
    setPalette(pal);
}

void WCoverArt::slotHideCoverArt() {
    m_bCoverIsVisible = false;
    setMinimumSize(0, 20);
}

void WCoverArt::slotLoadCoverArt(QImage picture) {
    if (picture.isNull()) {
        m_currentCover = m_defaultCover;
    } else {
        m_currentCover = picture;
    }
    update();
}

void WCoverArt::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    painter.drawLine(0,0,width(),0);

    if (m_bCoverIsVisible) {
        QImage scaledCover = m_currentCover.scaled(
                    QSize(height()-10, height()-10),
                    Qt::KeepAspectRatioByExpanding,
                    Qt::SmoothTransformation);
        painter.drawImage(width()/2-height()/2+4, 6, scaledCover);
    } else {
        QImage sc = QImage(":/images/library/ic_library_cover_show.png");
        sc = sc.scaled(height()-1, height()-1,
                       Qt::KeepAspectRatioByExpanding,
                       Qt::SmoothTransformation);
        painter.drawImage(0, 1 ,sc);
        painter.drawText(25, 15, tr("Show Cover Art"));
    }

    if (m_bCoverIsVisible && m_bCoverIsHovered) {
        QImage hc = QImage(":/images/library/ic_library_cover_hide.png");
        hc = hc.scaled(20, 20,
                       Qt::KeepAspectRatioByExpanding,
                       Qt::SmoothTransformation);
        painter.drawImage(width()-21, 6, hc);
    }
}

void WCoverArt::resizeEvent(QResizeEvent*) {
    if (m_bCoverIsVisible) {
        setMinimumSize(0, parentWidget()->height()/3);
     } else {
        slotHideCoverArt();
    }
}

void WCoverArt::mousePressEvent(QMouseEvent* event) {
    QPoint lastPoint;
    lastPoint = event->pos();
    if (m_bCoverIsVisible) {
        if(lastPoint.x() > width() - (height() / 5)
                && lastPoint.y() < (height() / 5) + 5) {
            m_bCoverIsVisible = false;
            resize(sizeHint());
        } else {
            if (m_currentCover.operator!=(m_defaultCover)) {
                QLabel *lb = new QLabel(this, Qt::Popup |
                                              Qt::Tool |
                                              Qt::CustomizeWindowHint |
                                              Qt::WindowCloseButtonHint);
                lb->setWindowModality(Qt::ApplicationModal);
                //int index = m_sCurrentCover.lastIndexOf("/");
                //QString title = m_sCurrentCover.mid(index + 1);
                //lb->setWindowTitle(title);
                lb->setWindowTitle(tr("Cover Art"));

                QPixmap px = QPixmap::fromImage(m_currentCover);
                QSize sz = QApplication::activeWindow()->size();

                if (px.height() > sz.height() / 1.2) {
                    px = px.scaledToHeight(sz.height() / 1.2,
                                           Qt::SmoothTransformation);
                }

                lb->setPixmap(px);
                lb->setGeometry(sz.width() / 2 - px.width() / 2,
                                sz.height() / 2 - px.height() / 2.2,
                                px.width(),
                                px.height());
                lb->show();
            }
        }
    } else {
        m_bCoverIsVisible = true;
        setCursor(Qt::ArrowCursor);
        resize(sizeHint());
    }
}

void WCoverArt::mouseMoveEvent(QMouseEvent* event) {
    QPoint lastPoint;
    lastPoint = event->pos();
    if (event->HoverEnter) {
        m_bCoverIsHovered  = true;
        if (m_bCoverIsVisible) {
            if (lastPoint.x() > width() - (height() / 5)
                    && lastPoint.y() < (height() / 5) + 5) {
                setCursor(Qt::ArrowCursor);
            } else {
                if (m_currentCover.operator==(m_defaultCover)) {
                    setCursor(Qt::ArrowCursor);
                } else {
                    QPixmap pix(":/images/library/ic_library_zoom_in.png");
                    pix = pix.scaled(24, 24);
                    setCursor(QCursor(pix));
                }
            }
        } else {
            setCursor(Qt::PointingHandCursor);
        }
    }
    update();
}

void WCoverArt::leaveEvent(QEvent*) {
    m_bCoverIsHovered = false;
    update();
}
