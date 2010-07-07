#include <QtCore>
#include <QtGui>

#include "effectsunitsview.h"
#include "widget/wknob.h"
#include "widget/wskincolor.h"

/* EffectsUnitsView
 * Create and place X EffectsUnitsWidgets
 *  * Pass m_Controller
 *  * Pass docelement for skinning X
 *  * Asign ID
 *  Load Skin File
 *  Load User Preferences
 */
EffectsUnitsView::EffectsUnitsView(QWidget * parent) : QWidget(parent)
{
    setObjectName("EffectsUnits");
    m_pGridLayout = new QGridLayout();
    this->setLayout(m_pGridLayout);


    QPalette palette;
    QColor c(0,0,0);
    palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
    QColor c2(255,255,255);
    palette.setBrush(foregroundRole(), WSkinColor::getCorrectColor(c2));

    setBackgroundRole(QPalette::Window);
    setPalette(palette);
    setAutoFillBackground(true);

    palette.setColor(QPalette::Base, WSkinColor::getCorrectColor(c));
    palette.setColor(QPalette::Text, WSkinColor::getCorrectColor(c2));


    m_EffectsUnitsController = new EffectsUnitsController();

    QComboBox * comboBox = new QComboBox(this);
    comboBox->addItem("djFlanger");
    comboBox->addItem("Plate2x2");
    comboBox->addItem("Lorenz");
    comboBox->addItem("AutoWah");
    comboBox->addItem("White");
    comboBox->show();

    connect(comboBox, SIGNAL(activated(QString)), this, SLOT(slotClick(QString)) );


}

EffectsUnitsView::~EffectsUnitsView()
{

}

void EffectsUnitsView::slotClick(QString teste){
	qDebug() << "FXUNITS: " << teste;
	m_EffectsUnitsController->activatePluginOnSource(teste, "[Channel1]");
}


