/**
* @file dlgprefcontroller.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*/

#include <QtGui>
#include <QDebug>
#include <QFileInfo>

#include "controllers/dlgprefcontroller.h"
#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "configobject.h"

DlgPrefController::DlgPrefController(QWidget *parent, Controller* controller,
                                     ControllerManager* controllerManager,
                                     ConfigObject<ConfigValue> *pConfig)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pControllerManager(controllerManager),
          m_pController(controller),
          m_bDirty(false) {

    connect(m_pController, SIGNAL(presetLoaded(ControllerPresetPointer)),
            this, SLOT(slotPresetLoaded(ControllerPresetPointer)));

    //QWidget * containerWidget = new QWidget();
    //QWidget * containerWidget = new QWidget(this);
    //m_ui.setupUi(containerWidget);
    m_ui.setupUi(this);
    m_pLayout = m_ui.gridLayout_4;
    const ControllerPresetPointer pPreset = controller->getPreset();

    slotPresetLoaded(pPreset);

    //m_pVerticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    //m_pLayout->addItem(m_pVerticalSpacer, 4, 0, 1, 3);

    //QVBoxLayout* vLayout = new QVBoxLayout(this);
    //vLayout->addWidget(containerWidget);
    //setLayout(vLayout);

    m_ui.labelDeviceName->setText(m_pController->getName());
    QString category = m_pController->getCategory();
    if (category!="") {
        m_ui.labelDeviceCategory->setText(category);
    } else {
        m_ui.labelDeviceCategory->hide();
    }

    connect(m_ui.comboBoxPreset, SIGNAL(activated(int)),
            this, SLOT(slotLoadPreset(int)));
    connect(m_ui.comboBoxPreset, SIGNAL(activated(int)),
            this, SLOT(slotDirty()));

    // Connect if we wish to automatically set current mapping as autoload
    // connect(m_ui.comboBoxPreset, SIGNAL(currentIndexChanged(const QString&)),
    //        this, SLOT(slotDirty()));

    connect(this, SIGNAL(openController(Controller*)),
            m_pControllerManager, SLOT(openController(Controller*)));
    connect(this, SIGNAL(closeController(Controller*)),
            m_pControllerManager, SLOT(closeController(Controller*)));
    connect(this, SIGNAL(loadPreset(Controller*, QString, bool)),
            m_pControllerManager, SLOT(loadPreset(Controller*, QString, bool)));
}

DlgPrefController::~DlgPrefController() {
}

QString DlgPrefController::presetShortName(const ControllerPresetPointer pPreset) const {
    QString presetName = tr("None");
    if (pPreset) {
        QString name = pPreset->name();
        QString author = pPreset->author();
        if (name.length() > 0 && author.length() > 0) {
            presetName = tr("%1 by %2").arg(pPreset->name(), pPreset->author());
        } else if (name.length() > 0) {
            presetName = name;
        } else if (pPreset->filePath().length() > 0) {
            QFileInfo file(pPreset->filePath());
            presetName = file.baseName();
        }
    }
    return presetName;
}

QString DlgPrefController::presetDescription(const ControllerPresetPointer pPreset) const {
    QString description = tr("No Description");
    if (pPreset) {
        QString descr = pPreset->description();
        if (description.length() > 0)
            description = descr;
    }
    return description;
}

QString DlgPrefController::presetForumLink(const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->forumlink();
        if (link.length() > 0)
            url = "<a href=\"" + link + "\">Mixxx Forums</a>";
    }
    return url;
}

QString DlgPrefController::presetWikiLink(const ControllerPresetPointer pPreset) const {
    QString url;
    if (pPreset) {
        QString link = pPreset->wikilink();
        if (link.length() > 0)
            url = "<a href=\"" + link + "\">Mixxx Wiki</a>";
    }
    return url;
}

void DlgPrefController::addWidgetToLayout(QWidget* pWidget) {
    // Remove the vertical spacer since we're adding stuff
    //m_pLayout->removeItem(m_pVerticalSpacer);
    //m_pLayout->addWidget(pWidget);
}

void DlgPrefController::slotDirty() {
    m_bDirty = true;
}

QString nameForPreset(const PresetInfo& preset) {
    QString name = preset.getName();
    if (name.length() == 0) {
        QFileInfo file(preset.getPath());
        name = file.baseName();
    }
    return name;
}

void DlgPrefController::enumeratePresets() {
    m_ui.comboBoxPreset->clear();

    // qDebug() << "Enumerating presets for controller" << m_pController->getName();

    //Insert a dummy "..." item at the top to try to make it less confusing.
    //(We don't want the first found file showing up as the default item
    // when a user has their controller plugged in)
    m_ui.comboBoxPreset->addItem("...");

    m_ui.comboBoxPreset->setInsertPolicy(QComboBox::InsertAlphabetically);
    // Ask the controller manager for a list of applicable presets
    PresetInfoEnumerator* pie =  m_pControllerManager->getPresetInfoManager();
    QList<PresetInfo> presets = pie->getPresets(m_pController->presetExtension());

    PresetInfo match;
    foreach (PresetInfo preset, presets) {
        m_ui.comboBoxPreset->addItem(nameForPreset(preset), preset.getPath());
        if (m_pController->matchPreset(preset)) {
            match = preset;
            break;
        }
    }

    // Jump to matching device in list if it was found.
    if (match.isValid()) {
        int index = m_ui.comboBoxPreset->findText(nameForPreset(match));
        if (index != -1)
            m_ui.comboBoxPreset->setCurrentIndex(index);
    }
}

void DlgPrefController::slotUpdate() {
    // Check if the device that this dialog is for is already enabled...
    bool deviceOpen = m_pController->isOpen();
    // Check/uncheck the "Enabled" box
    m_ui.chkEnabledDevice->setCheckState(deviceOpen ? Qt::Checked : Qt::Unchecked);
    // Enable/disable presets group box.
    m_ui.groupBoxPresets->setEnabled(deviceOpen);
    // Connect the "Enabled" checkbox after the checkbox state is set
    connect(m_ui.chkEnabledDevice, SIGNAL(stateChanged(int)),
            this, SLOT(slotDeviceState(int)));
    connect(m_ui.chkEnabledDevice, SIGNAL(stateChanged(int)),
            this, SLOT(slotDirty()));
}

void DlgPrefController::slotApply() {
    /* User has pressed OK, so if anything has been changed, enable or disable
     * the device, write the controls to the DOM, and reload the presets.
     */
    if (m_bDirty) {
        if (m_ui.chkEnabledDevice->isChecked()) {
            enableDevice();
        } else {
            disableDevice();
        }

        //Select the "..." item again in the combobox.
        m_ui.comboBoxPreset->setCurrentIndex(0);
    }
    m_bDirty = false;
}

void DlgPrefController::slotLoadPreset(int chosenIndex) {
    if (chosenIndex == 0) {
        // User picked ...
        return;
    }

    QString presetPath = m_ui.comboBoxPreset->itemData(chosenIndex).toString();

    // Applied on prefs close
    emit(loadPreset(m_pController, presetPath, true));
}

void DlgPrefController::slotPresetLoaded(ControllerPresetPointer preset) {
    m_ui.labelLoadedPreset->setText(presetShortName(preset));
    m_ui.labelLoadedPresetDescription->setText(presetDescription(preset));
    QStringList supportLinks;
    QString forumLink = presetForumLink(preset);
    if (forumLink.length() > 0) {
        supportLinks << forumLink;
    }
    QString wikiLink = presetWikiLink(preset);
    if (wikiLink.length() > 0) {
        supportLinks << wikiLink;
    }
    QString support = supportLinks.join("&nbsp;");
    if (support.length() == 0) {
        //: Shown when a MIDI controller has no links to support pages (e.g. Mixxx wiki or forums).
        support = tr("No support available.");
    }
    m_ui.labelLoadedPresetSupportLinks->setText(support);
}

void DlgPrefController::slotDeviceState(int state) {
    bool checked = state == Qt::Checked;
    // Enable/disable presets group box.
    m_ui.groupBoxPresets->setEnabled(checked);
    // Set tree item text to normal/bold
    emit(deviceStateChanged(this, checked));
}

void DlgPrefController::enableDevice() {
    emit(openController(m_pController));
    //TODO: Should probably check if open() actually succeeded.
}

void DlgPrefController::disableDevice() {
    emit(closeController(m_pController));
    //TODO: Should probably check if close() actually succeeded.
}
