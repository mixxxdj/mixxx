#include <QWidget>
#include <QString>
#include <QPair>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

#include "preferences/dialog/dlgpreflv2.h"
#include "engine/enginefilterbessel4.h"
#include "control/controlobject.h"
#include "util/math.h"

DlgPrefLV2::DlgPrefLV2(QWidget* pParent,
                       UserSettingsPointer pConfig,
                       std::shared_ptr<EffectsManager> pEffectsManager)
        : DlgPreferencePage(pParent),
          m_iCheckedParameters(0),
          m_pEffectsManager(pEffectsManager) {
    Q_UNUSED(pConfig);
    setupUi(this);

    m_pLV2Backend = dynamic_cast<LV2Backend*>(
            pEffectsManager->getEffectsBackendByType(EffectBackendType::LV2));
    if (!m_pLV2Backend) {
        return;
    }

    QList<QString> allPlugins = m_pLV2Backend->getDiscoveredPluginIds().toList();
    // Display them alphabetically
    qSort(allPlugins.begin(), allPlugins.end());

    for (const auto& effectId: allPlugins) {
        LV2Manifest* lv2Manifest = m_pLV2Backend->getLV2Manifest(effectId);
        EffectManifestPointer pEffectManifest = lv2Manifest->getEffectManifest();

        QPushButton* button = new QPushButton(this);
        button->setText(pEffectManifest->name());

        if (!m_pLV2Backend->canInstantiateEffect(effectId)) {
            // Tooltip displaying why this effect is disabled
            LV2Manifest::Status status = lv2Manifest->getStatus();
            switch (status) {
                case LV2Manifest::IO_NOT_STEREO:
                    button->setToolTip(QObject::tr("This plugin does not support "
                                                   "stereo samples as input/output"));
                    break;
                case LV2Manifest::HAS_REQUIRED_FEATURES:
                    button->setToolTip(QObject::tr("This plugin has features "
                                                   "which are not yet supported"));
                    break;
                default:
                    button->setToolTip(QObject::tr("Unknown status"));
            }
            button->setDisabled(true);
        } else {
            button->setDisabled(false);
        }

        lv2_vertical_layout_left->addWidget(button);
        button->setProperty("id", QVariant(pEffectManifest->id()));
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

    EffectManifestPointer pCurrentEffectManifest = m_pLV2Backend->getManifest(pluginId);
    const QList<EffectManifestParameterPointer>& parameterList =
            pCurrentEffectManifest->parameters();
    for (const auto& pPrameter: parameterList) {
        QCheckBox* entry = new QCheckBox(this);
        entry->setText(pPrameter->name());
        if (pPrameter->showInParameterSlot()) {
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

    int parameterListSize = parameterList.size();
    m_iCheckedParameters = parameterListSize < 8 ? parameterListSize : 8;
}

void DlgPrefLV2::slotApply() {
    EffectManifestPointer pCurrentEffectManifest =
            m_pLV2Backend->getManifest(m_currentEffectId);
    qDebug() << "DlgPrefLV2::slotApply" << pCurrentEffectManifest.data();
    for (int i = 0; i < m_pluginParameters.size(); i++) {
        EffectManifestParameterPointer pParameter = pCurrentEffectManifest->parameter(i);
        pParameter->setShowInParameterSlot(m_pluginParameters[i]->isChecked());
    }
    m_pEffectsManager->refeshAllRacks();
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
