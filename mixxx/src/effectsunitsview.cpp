#include <QtCore>
#include <QtGui>

#include "effectsunitsview.h"
#include "widget/wskincolor.h"
#include "widget/wpixmapstore.h"

/* EffectsUnitsView
 * Create and place X EffectsUnitsWidget
 *  * Pass m_Controller
 *  * Pass docelement for skinning X
 *  * Asign ID
 *  Load Skin File (Skin for like 10 widgets.)
 *  Load User Preferences
 */
EffectsUnitsView::EffectsUnitsView(QWidget * parent) : QWidget(parent)
{
    setObjectName("EffectsUnits");
    m_pGridLayout = new QGridLayout();
    this->setLayout(m_pGridLayout);


    m_EffectsUnitsController = new EffectsUnitsController();

    QComboBox * comboBox = new QComboBox(this);
    comboBox->addItem("djFlanger");
    comboBox->addItem("Plate2x2");
    comboBox->addItem("Lorenz");
    comboBox->addItem("AutoWah");
    comboBox->addItem("White");
    comboBox->show();

    m_label = new QLabel("THIS", this);
    m_label->move(100, 100);
    m_label->show();

    ConfigKey *key = new ConfigKey("[FX]", "DryWet");
    ControlPotmeter *control = new ControlPotmeter(*key, 0.0, 1.0);

    QDomDocument skin("LADSPASkin");
    QFile file(WWidget::getPath("ladspa_skin.xml"));
    skin.setContent(&file);
    QDomElement docElement = skin.documentElement();
    QDomElement slotTableElement = docElement.firstChildElement("SlotTable");
    QDomElement slotElement = slotTableElement.firstChildElement("Slot");
    QDomElement m_qKnobElement = slotElement.firstChildElement("Knob");
    QString keyString = QString("[FX],DryWet");
    m_qKnobElement.firstChildElement(QString("Connection")).firstChildElement(QString("ConfigKey")).firstChild().setNodeValue(keyString);
    m_qKnobElement.firstChildElement(QString("Tooltip")).firstChild().setNodeValue("Dry/wet");

    QDomElement bgElement = docElement.firstChildElement("Background");
    QString filename = bgElement.firstChildElement("Path").text();
    QPixmap *background = WPixmapStore::getPixmapNoCache(WWidget::getPath(filename));

    QPalette palette;
    QColor c(0,0,0);
    palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
    QColor c2(255,255,255);
    palette.setBrush(foregroundRole(), WSkinColor::getCorrectColor(c2));

    palette.setColor(QPalette::Base, WSkinColor::getCorrectColor(c));
    palette.setColor(QPalette::Text, WSkinColor::getCorrectColor(c2));

    setBackgroundRole(QPalette::Window);
    setPalette(palette);
    setAutoFillBackground(true);

    //m_pGridLayout->setPallete(pallete);

    m_pDryWetKnob = new WKnob(this);
    m_pDryWetKnob->setup(m_qKnobElement);
    m_pDryWetKnob->move(100, 70);
    m_pDryWetKnob->show();

    connect(comboBox, SIGNAL(activated(QString)), this, SLOT(slotClick(QString)) );


}

EffectsUnitsView::~EffectsUnitsView()
{

}

void EffectsUnitsView::slotClick(QString teste){
	ControlObject * WetDry = ControlObject::getControl(ConfigKey("[FX]", "DryWet"));
	qDebug() << "FXUNITS: " << teste << "oi:" << WetDry->get();
	m_label->setText(teste);
	m_EffectsUnitsController->activatePluginOnSource(teste, "[Channel1]");
}


