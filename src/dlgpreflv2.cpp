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
            // Tooltip with info why is this disabled
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

    // Clear the right vertical layout
    foreach (QCheckBox* box, m_pluginParameters) {
        delete box;
    }
    m_pluginParameters.clear();

    QLayoutItem* item;
    while ((item = lv2_vertical_layout_params->takeAt(1)) != 0) {
        lv2_vertical_layout_params->removeWidget(item->widget());
        delete item->widget();
    }

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString pluginId = button->property("id").toString();
    m_currentEffectId = pluginId;

    EffectManifest& currentEffectManifest = m_pLV2Backend->getManifestReference(pluginId);
    QList<EffectManifestParameter> parameterList = currentEffectManifest.parameters();
    int parameterListSize = parameterList.size();

    for (int i = 0; i < parameterListSize; i++) {
        QCheckBox* entry = new QCheckBox(this);
        entry->setText(parameterList[i].name());
        if (parameterList[i].showInParameterSlot()) {
            entry->setChecked(true);
        } else {
            entry->setChecked(false);
            entry->setEnabled(false);
        }
        lv2_vertical_layout_params->addWidget(entry);
        m_pluginParameters.append(entry);
        connect(entry, SIGNAL(stateChanged(int)),
                this, SLOT(slotUpdateOnParameterCheck(int)));
    }
    lv2_vertical_layout_params->addStretch();

    m_iCheckedParameters = parameterListSize < 8 ? parameterListSize : 8;
}

void DlgPrefLV2::slotApply() {
    EffectManifest& currentEffectManifest = m_pLV2Backend->getManifestReference(m_currentEffectId);
    for (int i = 0; i < m_pluginParameters.size(); i++) {       
	if (m_pluginParameters[i]->isChecked()) {
            currentEffectManifest.parameter(i)->setShowInParameterSlot(true);
        } else {
            currentEffectManifest.parameter(i)->setShowInParameterSlot(false);
        }
    }
}

void DlgPrefLV2::slotUpdateOnParameterCheck(int state) {
    if (state == Qt::Checked) {
        m_iCheckedParameters++;
    } else {
        m_iCheckedParameters--;
    }

    // If 8 parameters are already checked, disable all other checkboxes
    if (m_iCheckedParameters >= 8) {
        foreach (QCheckBox* box, m_pluginParameters) {
            if (!box->isChecked()) {
                box->setEnabled(false);
            }
        }
    } else {
        foreach (QCheckBox* box, m_pluginParameters) {
            if (!box->isChecked()) {
                box->setEnabled(true);
            }
        }
    }
}
