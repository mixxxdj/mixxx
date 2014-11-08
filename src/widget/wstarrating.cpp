/***************************************************************************
                          wstarrating.cpp  -  description
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2003 by Tue Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include <QStylePainter>
#include <QStyleOption>

#include "widget/wstarrating.h"

#include "library/starrating.h"


// #include "widget/wskincolor.h"

// WStarRating::WStarRating(QWidget* pParent)
        // : WBaseWidget(this) {
// }

WStarRating::WStarRating(const char* group,
                               ConfigObject<ConfigValue>* pConfig,
                               QWidget* pParent)
        : WWidget(pParent),
          // m_starRating(0,5),
          m_pGroup(group),
          m_pConfig(pConfig) {
    // setAcceptDrops(true);
    
}

WStarRating::~WStarRating() {
}

void WStarRating::setup(QDomNode node, const SkinContext& context) {
	
	
    // Used by delegates (e.g. StarDelegate) to tell when the mouse enters a
    // cell.
    setMouseTracking(true);
	
    // setFixedSize(m_pSlider->size());
    setFixedSize(100, 20);
    // setFixedSize( m_starRating.sizeHint() );
    
    autoFillBackground();
    setForegroundRole(QPalette::NoRole);
    
    update();
	/** /
    // Colors
    QPalette pal = palette(); //we have to copy out the palette to edit it since it's const (probably for threadsafety)
    if (context.hasNode(node, "BgColor")) {
        m_qBgColor.setNamedColor(context.selectString(node, "BgColor"));
        pal.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        setAutoFillBackground(true);
    }
    m_qFgColor.setNamedColor(context.selectString(node, "FgColor"));
    pal.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));
    setPalette(pal);

    // Text
    if (context.hasNode(node, "Text"))
        m_qsText = context.selectString(node, "Text");
    setText(m_qsText);

    // Font size
    if (context.hasNode(node, "FontSize")) {
        int fontsize = context.selectString(node, "FontSize").toInt();
        setFont( QFont("Helvetica",fontsize,QFont::Normal) );
    }

    // Alignment
    if (context.hasNode(node, "Alignment")) {
        if (context.selectString(node, "Alignment").toLower() == "right") {
            setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        } else if (context.selectString(node, "Alignment").toLower() == "center") {
            setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        } else if (context.selectString(node, "Alignment").toLower() == "left") {
            setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
	/**/
}

bool WStarRating::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return WWidget::event(pEvent);
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    // WBaseWidget::fillDebugTooltip(debug);
    // *debug << QString("Text: \"%1\"").arg(text());
}

void WStarRating::slotTrackLoaded(TrackPointer track) {
    if (track) {
        qDebug() << "!!!!!!!!!!!!!!!!!!! WStarRating track chargée!!!!!!!!";
        m_pCurrentTrack = track;
        connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
                this, SLOT(updateRating(TrackInfoObject*)));
        updateRating(track.data());
    }
}

void WStarRating::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    if (m_pCurrentTrack) {
        qDebug() << "!!!!!!!!!!!!!!!!!!! WStarRating track déchargée!!!!!!!!";
        disconnect(m_pCurrentTrack.data(), 0, this, 0);
    }
    m_pCurrentTrack.clear();
    // setText("");
}

void WStarRating::updateRating(TrackInfoObject*) {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toAscii().constData());
        qDebug() << "!!!!!!!!!!!!!!!!!!! WStarRating prop" << m_property << "\n";
        // if (property.isValid() && qVariantCanConvert<QString>(property)) {
            // setText(property.toString());
        // }
		// if (qVariantCanConvert<int>(value))
			// value =  QString("(%1)").arg(value.toInt());
        if (property.isValid()) {
			
			// StarRating starRating = qVariantValue<StarRating>(
				// m_pCurrentTrack->property(m_property.toAscii().constData()) );
			// pTrack->setRating(starRating.starCount());
			// m_pCurrentTrack->setRating(starRating.starCount());
        }
    }
}

// void 	render ( QPainter * painter, const QPoint & targetOffset = QPoint(), const QRegion & sourceRegion = QRegion(), RenderFlags renderFlags = RenderFlags( DrawWindowBackground | DrawChildren ) )
/*
void WStarRating::render (
		QPainter * painter,
		const QPoint & targetOffset /*= QPoint()* /,
		const QRegion & sourceRegion /*= QRegion()* /,
		RenderFlags renderFlags /*= RenderFlags( DrawWindowBackground | DrawChildren )* / ){
	
	// todo : render the starrating
	// void StarRating::paint(QPainter *painter, const QRect &rect, const QPalette &palette,
							// EditMode mode, bool isSelected) const
	// WStarRating.paint( painter );
    
    // StarRating starRating = qVariantValue<StarRating>(index.data());
    // starRating.paint(painter, newOption.rect, newOption.palette, StarRating::ReadOnly,
                     // newOption.state & QStyle::State_Selected);
	
}
*/

void WStarRating::paintEvent(QPaintEvent *) {
    qDebug() << "!!!!!!!!!!!!!!!!!!! WStarRating paintEvent!!!!!!!!";
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);
    painter.drawPrimitive(QStyle::PE_Widget, option);
    
     // painter.setPen(Qt::blue);
     // painter.setFont(QFont("Arial", 30));
     // painter.drawText(rect(), Qt::AlignCenter, "Qt");    
    
    if (m_pCurrentTrack) {
        qDebug() << "!!!!!!!!!!!!!!!!!!! WStarRating paintEvent track ok!!!!!!!!";
        qDebug() << "value " << m_pCurrentTrack->property(m_property.toAscii().constData());
        // StarRating starRating = qVariantValue<StarRating>(
            // m_pCurrentTrack->property(m_property.toAscii().constData()) );
        
        StarRating starRating(m_pCurrentTrack->getRating(), 5);
        
        setFixedSize( starRating.sizeHint() );
        painter.fillRect(option.rect, option.palette.base());
        painter.setBrush(option.palette.text());        
        
        // StarRating starRating = qVariantValue<StarRating>(index.data());
        starRating.paint(&painter, option.rect, option.palette, StarRating::ReadOnly,
                         option.state & QStyle::State_Selected);
                         // newOption.state & QStyle::State_Selected);
        
    } else {
        
        StarRating starRating(0,5);
        setFixedSize( starRating.sizeHint() );
        painter.fillRect(option.rect, option.palette.base());
        painter.setBrush(option.palette.text());        
        
        // StarRating starRating = qVariantValue<StarRating>(index.data());
        starRating.paint(&painter, option.rect, option.palette, StarRating::ReadOnly,
                         option.state & QStyle::State_Selected);
                         // newOption.state & QStyle::State_Selected);
    }
    
    qDebug() << "rect " << option.rect;
    qDebug() << "state " << option.state;
    qDebug() << "palette highlight " << option.palette.highlight();
    qDebug() << "palette foreground " << option.palette.foreground();
    
}

/*
void WSliderComposed::resizeEvent(QResizeEvent* pEvent) {
    Q_UNUSED(pEvent);
    m_dOldValue = -1;
    m_iPos = -1;
    // Re-calculate m_iPos based on our new width/height.
    onConnectedControlChanged(getControlParameter(), 0);
}
*/
