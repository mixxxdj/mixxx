#include <QWidget>
#include <QString>
#include <QPair>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

#include "dlgpreflv2.h"
#include "engine/enginefilterbessel4.h"
#include "controlobject.h"
#include "util/math.h"

#define pluginName first.first
#define isAvailable first.second
#define pluginId second

DlgPrefLV2::DlgPrefLV2(QWidget* pParent, LV2Backend* lv2Backend,
                       ConfigObject<ConfigValue>* pConfig)
        : DlgPreferencePage(pParent),
          m_pLV2Backend(lv2Backend),
          m_iCheckedParameters(0) {
    Q_UNUSED(pConfig);
    setupUi(this);
    if (!m_pLV2Backend) {
        return;
    }
    QList<QPair<QPair<QString, bool>, QString> > allPlugins = m_pLV2Backend->getAllDiscoveredPlugins();

    for (int i = 0; i < allPlugins.size(); i++) {
        QPushButton* button = new QPushButton(this);
        button->setText(allPlugins[i].pluginName);
        if (!allPlugins[i].isAvailable) {
            button->setDisabled(true);
        } else {
            button->setDisabled(false);
        }
        lv2_vertical_layout_left->addWidget(button);
        button->setProperty("id", QVariant(allPlugins[i].pluginId));
        connect(button, SIGNAL(clicked()), this, SLOT(slotDisplayParameters()));
    }
}

DlgPrefLV2::~DlgPrefLV2() {
}

void DlgPrefLV2::slotDisplayParameters() {
    // Set the number of checked parameters to 0 because new parameters are
    // displayed
    m_iCheckedParameters = 0;

    // Clear the right vertical layout
    foreach (QCheckBox* box, m_pluginParameters) {
        delete box;
    }
    m_pluginParameters.clear();

    QLayoutItem* item;
    while ((item = lv2_vertical_layout_right->takeAt(1)) != 0) {
        lv2_vertical_layout_right->removeWidget(item->widget());
        delete item->widget();
    }

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString pluginId = button->property("id").toString();
    EffectManifest& manifest = m_pLV2Backend->getManifestReference(pluginId);
    // Need to do the remapping here
    foreach (EffectManifestParameter param, manifest.parameters()) {
        QCheckBox* entry = new QCheckBox(this);
        entry->setText(param.name());
        lv2_vertical_layout_right->addWidget(entry);
        m_pluginParameters.append(entry);
    }
    lv2_vertical_layout_right->addStretch();
}
