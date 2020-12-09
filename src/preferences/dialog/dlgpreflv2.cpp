#include "dlgpreflv2.h"

#include <QCheckBox>
#include <QLabel>
#include <QPair>
#include <QPushButton>
#include <QString>
#include <QWidget>

#include "control/controlobject.h"
#include "effects/effectsmanager.h"
#include "moc_dlgpreflv2.cpp"
#include "util/math.h"

DlgPrefLV2::DlgPrefLV2(QWidget* pParent, LV2Backend* lv2Backend,
                       UserSettingsPointer pConfig,
                       EffectsManager* pEffectsManager)
        : DlgPreferencePage(pParent),
          m_pLV2Backend(lv2Backend),
          m_iCheckedParameters(0),
          m_pEffectsManager(pEffectsManager) {
    Q_UNUSED(pConfig);

    setupUi(this);

    if (!m_pLV2Backend) {
        return;
    }

    QList<QString> allPlugins = m_pLV2Backend->getDiscoveredPluginIds().values();
    // Display them alphabetically
    std::sort(allPlugins.begin(), allPlugins.end());

    for (const auto& effectId: allPlugins) {
        LV2Manifest* lv2Manifest = m_pLV2Backend->getLV2Manifest(effectId);
        EffectManifestPointer pEffectManifest = lv2Manifest->getEffectManifest();

        QPushButton* button = new QPushButton(this);
        button->setText(pEffectManifest->name());
        button->setStyleSheet("text-align:left; padding: 5px;");

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

        lv2EffectsList->addWidget(button);
        button->setProperty("id", QVariant(pEffectManifest->id()));
        connect(button, &QAbstractButton::clicked, this, &DlgPrefLV2::slotDisplayParameters);
    }
    effectNameLabel->clear();
}

DlgPrefLV2::~DlgPrefLV2() {
}

void DlgPrefLV2::slotDisplayParameters() {
    // New parameters will be displayed.
    // Reset the number of checked parameters to 0 and clear the layout.
    foreach (QCheckBox* box, m_pluginParameters) {
        delete box;
    }
    m_pluginParameters.clear();

    // isEmpty() doesn't consider spacers but count() does.
    while (lv2EffectParametersList->count() > 0) {
        QLayoutItem* item = lv2EffectParametersList->takeAt(0);
        VERIFY_OR_DEBUG_ASSERT(item) {
            continue;
        }
        delete item;
    }

    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString pluginId = button->property("id").toString();
    m_currentEffectId = pluginId;

    EffectManifestPointer pCurrentEffectManifest = m_pLV2Backend->getManifest(pluginId);
    if (pCurrentEffectManifest) {
        // Show the effect name above the parameter list
        effectNameLabel->setText(QObject::tr("Parameters of %1")
                                         .arg(pCurrentEffectManifest->name()));
        // Populate the parameters list
        const QList<EffectManifestParameterPointer>& parameterList =
                pCurrentEffectManifest->parameters();
        for (const auto& pParameter : parameterList) {
            QCheckBox* entry = new QCheckBox(this);
            entry->setText(pParameter->name());
            if (pParameter->showInParameterSlot()) {
                entry->setChecked(true);
            } else {
                entry->setChecked(false);
                entry->setEnabled(false);
            }
            lv2EffectParametersList->addWidget(entry);
            m_pluginParameters.append(entry);
            connect(entry, &QCheckBox::stateChanged, this, &DlgPrefLV2::slotUpdateOnParameterCheck);
        }
        int parameterListSize = parameterList.size();
        m_iCheckedParameters = parameterListSize < 8 ? parameterListSize : 8;
    } else {
        m_iCheckedParameters = 0;
    }
    lv2EffectParametersList->addStretch();
}

void DlgPrefLV2::slotUpdate() {
    // This preferences page will be removed in PR #2618 anyway, so we'll just
    // leave this empty for now.
}

void DlgPrefLV2::slotResetToDefaults() {
    // This preferences page will be removed in PR #2618 anyway, so we'll just
    // leave this empty for now.
}

void DlgPrefLV2::slotApply() {
    EffectManifestPointer pCurrentEffectManifest =
            m_pLV2Backend->getManifest(m_currentEffectId);
    qDebug() << "DlgPrefLV2::slotApply" << pCurrentEffectManifest.data();
    if (pCurrentEffectManifest) {
        for (int i = 0; i < m_pluginParameters.size(); i++) {
            EffectManifestParameterPointer pParameter = pCurrentEffectManifest->parameter(i);
            pParameter->setShowInParameterSlot(m_pluginParameters[i]->isChecked());
        }
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
