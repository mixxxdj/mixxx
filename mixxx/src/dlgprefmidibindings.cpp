/***************************************************************************
                          dlgprefmidibindings.cpp  -  description
                             -------------------
    begin                : Sat Jun 21 2008
    copyright            : (C) 2008 by Tom Care
    email                : psyc0de@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtGui>
#include <QDebug>
#include "midi/midiinputmappingtablemodel.h"
#include "midi/midioutputmappingtablemodel.h"
#include "midi/midichanneldelegate.h"
#include "midi/midistatusdelegate.h"
#include "midi/midinodelegate.h"
#include "midi/midioptiondelegate.h"
#include "controlgroupdelegate.h"
#include "controlvaluedelegate.h"
#include "dlgprefmidibindings.h"
#include "midi/mididevice.h"
#include "midi/mididevicemanager.h"
#include "configobject.h"
#include "midi/midimapping.h"

#ifdef __MIDISCRIPT__
#include "midi/midiscriptengine.h"
#endif


DlgPrefMidiBindings::DlgPrefMidiBindings(QWidget *parent, MidiDevice* midiDevice,
                                         MidiDeviceManager* midiDeviceManager,
                                         ConfigObject<ConfigValue> *pConfig)
    : QWidget(parent), Ui::DlgPrefMidiBindingsDlg()
{
    setupUi(this);
    m_pConfig = pConfig;
    m_pMidiDevice = midiDevice;
    m_pMidiDeviceManager = midiDeviceManager;

    m_pDlgMidiLearning = NULL;

    m_bDirty = false;

    labelDeviceName->setText(m_pMidiDevice->getName());

    //Tell the input mapping table widget which data model it should be viewing
    //(note that m_pInputMappingTableView is defined in the .ui file!)
    m_pInputMappingTableView->setModel((QAbstractItemModel*)m_pMidiDevice->getMidiMapping()->getMidiInputMappingTableModel());

    m_pInputMappingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pInputMappingTableView->setSelectionMode(QAbstractItemView::ContiguousSelection); //The model won't like ExtendedSelection, probably.
    m_pInputMappingTableView->verticalHeader()->hide();

    //Set up "delete" as a shortcut key to remove a row for the MIDI input table.
    m_deleteMIDIInputRowAction = new QAction(m_pInputMappingTableView);
    /*m_deleteMIDIInputRowAction->setShortcut(QKeySequence::Delete);
    m_deleteMIDIInputRowAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_deleteMIDIInputRowAction, SIGNAL(triggered()), this, SLOT(slotRemoveInputBinding()));
    */
    //The above shortcut doesn't work yet, not quite sure why. -- Albert Feb 1 / 2009

    //Set up the cool item delegates for the input mapping table
    m_pMidiChannelDelegate = new MidiChannelDelegate(m_pInputMappingTableView);
    m_pMidiStatusDelegate = new MidiStatusDelegate(m_pInputMappingTableView);
    m_pMidiNoDelegate = new MidiNoDelegate(m_pInputMappingTableView);
    m_pMidiOptionDelegate = new MidiOptionDelegate(m_pInputMappingTableView);
    m_pControlGroupDelegate = new ControlGroupDelegate(m_pInputMappingTableView);
    m_pControlValueDelegate = new ControlValueDelegate(m_pInputMappingTableView);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDISTATUS, m_pMidiStatusDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDICHANNEL, m_pMidiChannelDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDINO, m_pMidiNoDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_CONTROLOBJECTGROUP, m_pControlGroupDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_CONTROLOBJECTVALUE, m_pControlValueDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDIOPTION, m_pMidiOptionDelegate);

    //Tell the output mapping table widget which data model it should be viewing
    //(note that m_pOutputMappingTableView is defined in the .ui file!)
    m_pOutputMappingTableView->setModel((QAbstractItemModel*)m_pMidiDevice->getMidiMapping()->getMidiOutputMappingTableModel());
    m_pOutputMappingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pOutputMappingTableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_pOutputMappingTableView->verticalHeader()->hide();

    //Set up the cool item delegates for the output mapping table
    m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_MIDISTATUS, m_pMidiStatusDelegate);
    m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_MIDICHANNEL, m_pMidiChannelDelegate);
    m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_MIDINO, m_pMidiNoDelegate);
    //TODO: We need different delegates for the output table's CO group/value columns because we only list real input
    //      controls, and for output we'd want to list a different set with stuff like "VUMeter" and other output controls.
    //m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_CONTROLOBJECTGROUP, m_pControlGroupDelegate);
    //m_pOutputMappingTableView->setItemDelegateForColumn(MIDIOUTPUTTABLEINDEX_CONTROLOBJECTVALUE, m_pControlValueDelegate);

    // Connect buttons to slots
    connect(btnExportXML, SIGNAL(clicked()), this, SLOT(slotExportXML()));

    //Input bindings
    connect(btnMidiLearnWizard, SIGNAL(clicked()), this, SLOT(slotShowMidiLearnDialog()));
    connect(btnMidiLearnWizard, SIGNAL(clicked()), this, SLOT(slotDirty()));
    connect(btnClearAllInputBindings, SIGNAL(clicked()), this, SLOT(slotClearAllInputBindings()));
    connect(btnClearAllInputBindings, SIGNAL(clicked()), this, SLOT(slotDirty()));
    connect(btnRemoveInputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveInputBinding()));
    connect(btnRemoveInputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));
    connect(btnAddInputBinding, SIGNAL(clicked()), this, SLOT(slotAddInputBinding()));
    connect(btnAddInputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));

    //Output bindings
    connect(btnClearAllOutputBindings, SIGNAL(clicked()), this, SLOT(slotClearAllOutputBindings()));
    connect(btnClearAllOutputBindings, SIGNAL(clicked()), this, SLOT(slotDirty()));
    connect(btnRemoveOutputBinding, SIGNAL(clicked()), this, SLOT(slotRemoveOutputBinding()));
    connect(btnRemoveOutputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));
    connect(btnAddOutputBinding, SIGNAL(clicked()), this, SLOT(slotAddOutputBinding()));
    connect(btnAddOutputBinding, SIGNAL(clicked()), this, SLOT(slotDirty()));

    connect(comboBoxPreset, SIGNAL(activated(const QString&)), this, SLOT(slotLoadMidiMapping(const QString&)));
    connect(comboBoxPreset, SIGNAL(activated(const QString&)), this, SLOT(slotDirty()));

    connect(m_pInputMappingTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotDirty()));
    connect(m_pOutputMappingTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotDirty()));

    //Load the list of presets into the presets combobox.
    enumeratePresets();

    //Initialize the output device combobox
    enumerateOutputDevices();

}

DlgPrefMidiBindings::~DlgPrefMidiBindings() {
    delete m_deleteMIDIInputRowAction;
}

void DlgPrefMidiBindings::slotDirty ()
{
    m_bDirty = true;
}

void DlgPrefMidiBindings::enumerateOutputDevices()
{
    comboBoxOutputDevice->clear();

    comboBoxOutputDevice->addItem(tr("None"));

    //For each MIDI output device, insert an item into the output device combobox.
    QList<MidiDevice*> deviceList = m_pMidiDeviceManager->getDeviceList(true, false);
    QListIterator<MidiDevice*> it(deviceList);

    while (it.hasNext())
      {
        MidiDevice* currentDevice = it.next();
        QString curDeviceName = currentDevice->getName();
        //qDebug() << "curDeviceName: " << curDeviceName;
        comboBoxOutputDevice->addItem(curDeviceName);
      }

    //Assume autopairing was done and let's just show the output device combobox with the name
    //of the input device selected for now...
    QString currentOutputMidiDeviceName = m_pMidiDevice->getName();
    comboBoxOutputDevice->setCurrentIndex(comboBoxOutputDevice->findText(currentOutputMidiDeviceName));

}

void DlgPrefMidiBindings::enumeratePresets()
{
    QList<QString> presetsList;
    comboBoxPreset->clear();

    //Insert a dummy "..." item at the top to try to make it less confusing.
    //(For example, we don't want "Akai MPD24" showing up as the default item
    // when a user has their controller plugged in)
    comboBoxPreset->addItem("...");

    // paths to search for midi presets
    QList<QString> midiDirPaths;
    midiDirPaths.append(LPRESETS_PATH);
    midiDirPaths.append(m_pConfig->getConfigPath().append("midi/"));

    QListIterator<QString> itpth(midiDirPaths);
    while (itpth.hasNext()) {
        QDirIterator it(itpth.next());
        while (it.hasNext())
        {
            it.next(); //Advance iterator. We get the filename from the next line. (It's a bit weird.)
            QString curMapping = it.fileName();
            if (curMapping.endsWith(MIDI_MAPPING_EXTENSION)) //blah, thanks for nothing Qt
            {
                curMapping.chop(QString(MIDI_MAPPING_EXTENSION).length()); //chop off the .midi.xml
                presetsList.append(curMapping);
            }
        }
    }
    //Sort in alphabetical order
    qSort(presetsList);
    comboBoxPreset->addItems(presetsList);
}



/* slotUpdate()
 * Called when the dialog is displayed.
 */
void DlgPrefMidiBindings::slotUpdate() {

    //Check if the device that this dialog is for is already enabled...
    if (m_pMidiDevice->isOpen())
    {
        chkEnabledDevice->setCheckState(Qt::Checked); //Check the "Enabled" box
        toolBox->setEnabled(true); //Enable MIDI in/out toolbox.
        groupBoxPresets->setEnabled(true); //Enable presets group box.
    }
    else {
        chkEnabledDevice->setCheckState(Qt::Unchecked); //Uncheck the "Enabled" box
        toolBox->setEnabled(false); //Disable MIDI in/out toolbox.
        groupBoxPresets->setEnabled(false); //Disable presets group box.
    }

    //Connect the "Enabled" checkbox after the checkbox state is set
    connect(chkEnabledDevice, SIGNAL(stateChanged(int)), this, SLOT(slotDeviceState(int)));
    connect(chkEnabledDevice, SIGNAL(stateChanged(int)), this, SLOT(slotDirty()));
}

/* slotApply()
 * Called when the OK button is pressed.
 */
void DlgPrefMidiBindings::slotApply() {
    /* User has pressed OK, so enable or disable the device, write the
     * controls to the DOM, and reload the MIDI bindings.  FIXED: only
     * do this if the user has changed the preferences.
     */
    if (m_bDirty)
    {
        m_pMidiDevice->disableMidiLearn();
        if (chkEnabledDevice->isChecked()) {
            //Enable the device.
            enableDevice();

            //Disable processing of MIDI messages received from the device in order to
            //prevent a race condition while we modify the MIDI mapping.
            m_pMidiDevice->setReceiveInhibit(true);
            m_pMidiDevice->getMidiMapping()->applyPreset();
            m_pMidiDevice->setReceiveInhibit(false);

            //FIXME: We need some logic like this to make changing the output device work.
            //       See MidiDeviceManager::associateInputAndOutputDevices() for more info...
            /*
              if (comboBoxOutputDevice->currentText() != tr("None"))
              m_pMidiDeviceManager->associateInputAndOutputDevices(m_pMidiDevice, comboBoxOutputDevice->currentText());
            */
        }
        else disableDevice();
    }
    m_bDirty = false;
}

void DlgPrefMidiBindings::slotShowMidiLearnDialog() {

    //If the user has checked the "Enabled" checkbox but they haven't
    //hit OK to apply it yet, prompt them to apply the settings before we open
    //the MIDI learning dialog. If we don't apply the settings first and open the device,
    //MIDI learn won't react to MIDI messages.
    if (chkEnabledDevice->isChecked() && !m_pMidiDevice->isOpen())
    {
        QMessageBox::StandardButton result = QMessageBox::question(this,
                    tr("Apply MIDI device settings?"),
                    tr("Your settings must be applied before starting the MIDI learning wizard.\n"
                        "Apply settings and continue?"));
        if (result == QMessageBox::Cancel)
        {
            return;
        }
        else
        {
            slotApply();
        }
    }

    //Note that DlgMidiLearning is set to delete itself on
    //close using the Qt::WA_DeleteOnClose attribute (so this "new" doesn't leak memory)
    m_pDlgMidiLearning = new DlgMidiLearning(this, m_pMidiDevice->getMidiMapping());
    m_pDlgMidiLearning->show();
}

/* slotImportXML()
 * Prompts the user for an XML preset and loads it.
 */
void DlgPrefMidiBindings::slotLoadMidiMapping(const QString &name) {

    if (name == "...")
        return;

    //Ask for confirmation if the MIDI tables aren't empty...
    MidiMapping* mapping = m_pMidiDevice->getMidiMapping();
    if (mapping->numInputMidiMessages() > 0 ||
        mapping->numOutputMixxxControls() > 0)
    {
         QMessageBox::StandardButton result = QMessageBox::question(
             this,
             tr("Overwrite existing mapping?"),
             tr("Are you sure you'd like to load the %1 mapping?\n"
                "This will overwrite your existing MIDI mapping.").arg(name),
                   QMessageBox::Yes | QMessageBox::No);

         if (result == QMessageBox::No) {
            //Select the "..." item again in the combobox.
            comboBoxPreset->setCurrentIndex(0);
            return;
         }
    }

    QString filename = LPRESETS_PATH + name + MIDI_MAPPING_EXTENSION;
    QFile ftest(filename);
    if ( !ftest.exists() ) filename = m_pConfig->getConfigPath().append("midi/") + name + MIDI_MAPPING_EXTENSION;

    if (!filename.isNull()) m_pMidiDevice->getMidiMapping()->loadPreset(filename, true);    // It's applied on prefs close
    m_pInputMappingTableView->update();

    //Select the "..." item again in the combobox.
    comboBoxPreset->setCurrentIndex(0);
}

/* slotExportXML()
 * Prompts the user for an XML preset and saves it.
 */
void DlgPrefMidiBindings::slotExportXML() {
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Export Mixxx MIDI Bindings"), m_pConfig->getConfigPath().append("midi/"),
            tr("Preset Files (*.midi.xml)"));
    if (!fileName.isNull()) m_pMidiDevice->getMidiMapping()->savePreset(fileName);
}

void DlgPrefMidiBindings::slotDeviceState(int state) {
  if (state == Qt::Checked) {
      toolBox->setEnabled(true);    //Enable MIDI in/out toolbox.
      groupBoxPresets->setEnabled(true);    //Enable presets group box.
      emit deviceStateChanged(this,true);  // Set tree item text to bold
  }
  else {
      toolBox->setEnabled(false);   //Disable MIDI in/out toolbox.
      groupBoxPresets->setEnabled(false);   //Disable presets group box.
      emit deviceStateChanged(this,false);  // Set tree item text to not bold
  }
}

void DlgPrefMidiBindings::enableDevice()
{
    m_pMidiDevice->close();
    m_pMidiDevice->open();
    m_pConfig->set(ConfigKey("[Midi]", m_pMidiDevice->getName().replace(" ", "_")), 1);

    //TODO: Should probably check if open() actually succeeded.
}

void DlgPrefMidiBindings::disableDevice()
{
    m_pMidiDevice->close();
    m_pConfig->set(ConfigKey("[Midi]", m_pMidiDevice->getName().replace(" ", "_")), 0);

    //TODO: Should probably check if close() actually succeeded.
}

void DlgPrefMidiBindings::slotAddInputBinding()
{
    bool ok = true;
    QString controlGroup = QInputDialog::getItem(this, tr("Select Control Group"), tr("Select Control Group"),
                                                ControlGroupDelegate::getControlGroups(), 0, false,  &ok);
    if (!ok) return;

    QStringList controlValues;
    if (controlGroup == CONTROLGROUP_CHANNEL1_STRING ||
        controlGroup == CONTROLGROUP_CHANNEL2_STRING) {
        controlValues = ControlValueDelegate::getChannelControlValues();
    }
    else if (controlGroup == CONTROLGROUP_MASTER_STRING)
    {
        controlValues = ControlValueDelegate::getMasterControlValues();
    }
    else if (controlGroup == CONTROLGROUP_PLAYLIST_STRING)
    {
        controlValues = ControlValueDelegate::getPlaylistControlValues();
    }
    else if (controlGroup == CONTROLGROUP_FX_STRING)
    {
    	controlValues = ControlValueDelegate::getFXControlValues();
    }
    else if (controlGroup == CONTROLGROUP_FLANGER_STRING)
    {
        controlValues = ControlValueDelegate::getFlangerControlValues();
    }
    else if (controlGroup == CONTROLGROUP_MICROPHONE_STRING)
    {
        controlValues = ControlValueDelegate::getMicrophoneControlValues();
    }
    else
    {
        qDebug() << "Unhandled ControlGroup in " << __FILE__;
    }


    QString controlValue = QInputDialog::getItem(this, tr("Select Control"), tr("Select Control"),
                                                 controlValues, 0, false,  &ok);
    if (!ok) return;


    MixxxControl mixxxControl(controlGroup, controlValue);
    MidiMessage message;

    while (m_pMidiDevice->getMidiMapping()->isMidiMessageMapped(message))
    {
        message.setMidiNo(message.getMidiNo() + 1);
        if (message.getMidiNo() >= 127) //If the table is full, then overwrite something...
            break;
    }
    m_pMidiDevice->getMidiMapping()->setInputMidiMapping(message, mixxxControl);
}

void DlgPrefMidiBindings::slotRemoveInputBinding()
{
    QModelIndexList selectedIndices = m_pInputMappingTableView->selectionModel()->selectedRows();
    if (selectedIndices.size() > 0)
    {
        MidiInputMappingTableModel* tableModel = dynamic_cast<MidiInputMappingTableModel*>(m_pInputMappingTableView->model());
        if (tableModel) {

            QModelIndex curIndex;
            //The model indices are sorted so that we remove the rows from the table
            //in ascending order. This is necessary because if row A is above row B in
            //the table, and you remove row A, the model index for row B will change.
            //Sorting the indices first means we don't have to worry about this.
            qSort(selectedIndices);

            //Going through the model indices in descending order (see above comment for explanation).
            QListIterator<QModelIndex> it(selectedIndices);
            it.toBack();
            while (it.hasPrevious())
            {
                curIndex = it.previous();
                tableModel->removeRow(curIndex.row());
            }
        }
    }
}

void DlgPrefMidiBindings::slotClearAllInputBindings() {
    if (QMessageBox::warning(this, tr("Clear Input Bindings"),
            tr("Are you sure you want to clear all bindings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    //Remove all the rows from the data model (ie. the MIDI mapping).
    MidiInputMappingTableModel* tableModel = dynamic_cast<MidiInputMappingTableModel*>(m_pInputMappingTableView->model());
    if (tableModel) {
        tableModel->removeRows(0, tableModel->rowCount());
    }
}


void DlgPrefMidiBindings::slotAddOutputBinding() {
    qDebug() << "STUB: DlgPrefMidiBindings::slotAddOutputBinding()";

    m_pMidiDevice->getMidiMapping()->setOutputMidiMapping(MixxxControl(), MidiMessage());
}

void DlgPrefMidiBindings::slotRemoveOutputBinding()
{
    QModelIndexList selectedIndices = m_pOutputMappingTableView->selectionModel()->selectedRows();
    if (selectedIndices.size() > 0)
    {
        MidiOutputMappingTableModel* tableModel =
                            dynamic_cast<MidiOutputMappingTableModel*>(m_pOutputMappingTableView->model());
        if (tableModel) {
            QModelIndex curIndex;
            //The model indices are sorted so that we remove the rows from the table
            //in ascending order. This is necessary because if row A is above row B in
            //the table, and you remove row A, the model index for row B will change.
            //Sorting the indices first means we don't have to worry about this.
            //qSort(selectedIndices);

            //Going through the model indices in descending order (see above comment for explanation).
            QListIterator<QModelIndex> it(selectedIndices);
            it.toBack();
            while (it.hasPrevious())
            {
                curIndex = it.previous();
                qDebug() << "Dlg: removing row" << curIndex.row();
                tableModel->removeRow(curIndex.row());
            }
        }
    }
}

void DlgPrefMidiBindings::slotClearAllOutputBindings() {
    if (QMessageBox::warning(this, tr("Clear Output Bindings"),
            tr("Are you sure you want to clear all output bindings?"),
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    //Remove all the rows from the data model (ie. the MIDI mapping).
    MidiOutputMappingTableModel* tableModel = dynamic_cast<MidiOutputMappingTableModel*>(m_pOutputMappingTableView->model());
    if (tableModel) {
        tableModel->removeRows(0, tableModel->rowCount());
    }
}
