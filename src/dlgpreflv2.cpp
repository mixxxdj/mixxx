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
    while ((item = lv2_vertical_layout_params->takeAt(1)) != 0) {
        lv2_vertical_layout_params->removeWidget(item->widget());
        delete item->widget();
    }

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString pluginId = button->property("id").toString();
    m_currentEffectId = pluginId;
    EffectManifest& currentEffectManifest = m_pLV2Backend->getManifestReference(pluginId);
    // Need to do the remapping here
    foreach (EffectManifestParameter param, currentEffectManifest.parameters()) {
        QCheckBox* entry = new QCheckBox(this);
        entry->setText(param.name());
        lv2_vertical_layout_params->addWidget(entry);
        m_pluginParameters.append(entry);
        connect(entry, SIGNAL(stateChanged(int)),
                this, SLOT(slotUpdateOnParameterCheck(int)));
    }
    lv2_vertical_layout_params->addStretch();
}

void DlgPrefLV2::slotApply() {
    EffectManifest& currentEffectManifest = m_pLV2Backend->getManifestReference(m_currentEffectId);
    // It displays the first 8 checked parameters
    int visible = 0;
    int hidden = m_iCheckedParameters;
    for (int i = 0; i < m_pluginParameters.size(); i++) {
        if (m_pluginParameters[i]->isChecked()) {
            currentEffectManifest.setActiveParameter(visible, i);
            visible++;
        } else {
            currentEffectManifest.setActiveParameter(hidden, i);
            hidden++;
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

void DlgPrefLV2::slotDisplayButtonParameters() {
    // Set the number of checked parameters to 0 because new parameters are
    // displayed
    m_iCheckedButtonParameters = 0;

    // Clear the right vertical layout
    foreach (QCheckBox* box, m_pluginButtonParameters) {
        delete box;
    }
    m_pluginButtonParameters.clear();

    QLayoutItem* item;
    while ((item = lv2_vertical_layout_button_params->takeAt(1)) != 0) {
        lv2_vertical_layout_button_params->removeWidget(item->widget());
        delete item->widget();
    }

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString pluginId = button->property("id").toString();
    m_currentEffectId = pluginId;
    EffectManifest& currentEffectManifest = m_pLV2Backend->getManifestReference(pluginId);
    // Need to do the remapping here
    foreach (EffectManifestParameter param, currentEffectManifest.buttonParameters()) {
        QCheckBox* entry = new QCheckBox(this);
        entry->setText(param.name());
        lv2_vertical_layout_button_params->addWidget(entry);
        m_pluginButtonParameters.append(entry);
        connect(entry, SIGNAL(stateChanged(int)),
                this, SLOT(slotUpdateOnButtonParameterCheck(int)));
    }
    lv2_vertical_layout_button_params->addStretch();
}

void DlgPrefLV2::slotUpdateOnButtonParameterCheck(int state) {
    if (state == Qt::Checked) {
        m_iCheckedButtonParameters++;
    } else {
        m_iCheckedButtonParameters--;
    }

    // If 8 parameters are already checked, disable all other checkboxes
    if (m_iCheckedButtonParameters >= 8) {
        foreach (QCheckBox* box, m_pluginButtonParameters) {
            if (!box->isChecked()) {
                box->setEnabled(false);
            }
        }
    } else {
        foreach (QCheckBox* box, m_pluginButtonParameters) {
            if (!box->isChecked()) {
                box->setEnabled(true);
            }
        }
    }
}
