/***************************************************************************
                          dlgprefmididevice.cpp  -  description
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
#include <QtCore>
#include "dlgprefmididevice.h"
#include "midiledhandler.h"
#include "widget/wwidget.h"

#define DEVICE_CONFIG_PATH QDir::homePath().append("/").append(".MixxxMIDIDevices")

static QString toHex(QString numberStr) {
    return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

DlgPrefMidiDevice::DlgPrefMidiDevice(QWidget *parent, MidiObject *midi, ConfigObject<ConfigValue> *pConfig) :  QWidget(parent), Ui::DlgPrefMidiDeviceDlg() {
    setupUi(this);

    m_pConfig = pConfig;

    // TODO: use #define for filename
    m_pDeviceSettings = new ConfigObject<ConfigValue>(DEVICE_CONFIG_PATH);

    // Open midi
    m_pMidi = midi;

    // Store default midi device
    //m_pConfig->set(ConfigKey("[Midi]","Device"), ConfigValue(m_pMidi->getOpenDevice()));

    // Midi devices
    listMidiDevices->clear();
    QStringList * midiDeviceList = m_pMidi->getDeviceList();
    listMidiDevices->addItems(*midiDeviceList);

    // Set up send/receive enable/disable options - disabled initially
    comboMidiRxEnable->addItem("Disabled", false);
    comboMidiTxEnable->addItem("Disabled", false);
    comboMidiRxEnable->addItem("Enabled", true);
    comboMidiTxEnable->addItem("Enabled", true);

    setupDevices();
	
    // Enable update on list item change
    connect(listMidiDevices, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(slotGetInfo(QListWidgetItem*, QListWidgetItem*)));
    // Enable update of device status upon combo box changes
    connect(comboMidiRxEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateEnabled()));
    connect(comboMidiTxEnable, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateEnabled()));
    // Connect load preset button
    //connect(btnLoadDefaults, SIGNAL(clicked()), this, SLOT(slotLoadPreset())); //slot doesn't exist yet so shut up Mixxx at boot time with a comment
			
    tblDebug->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    // Select the first MIDI device, if there is one.
    if (listMidiDevices->count()) {
#if QT_VERSION >= 0x040400
        listMidiDevices->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
#else
        //Ensures backwards compatiblity with Qt 4.3, but at the expense of possibly introducing a bug.
        listMidiDevices->setCurrentRow(0);
#endif
		slotGetInfo(listMidiDevices->currentItem(), 0);
    }

    // Install event handler to handle hiding...
    installEventFilter(this);
}

bool DlgPrefMidiDevice::eventFilter(QObject * o, QEvent * e)
{
    if (e->type() == QEvent::Hide) tblDebug->setRowCount(0);
	return false; //Let the event be handled further
}

DlgPrefMidiDevice::~DlgPrefMidiDevice() {

}

void DlgPrefMidiDevice::slotDebug(ConfigValueMidi *event, char value, QString device) {
// TODO: it would be really cool if we could have the rows go in descending order, so we don't have to refocus with each row...
	int rowNumber = tblDebug->rowCount();
	tblDebug->insertRow(rowNumber);
	tblDebug->setItem(rowNumber, 0, new QTableWidgetItem(device));
	tblDebug->setItem(rowNumber, 1, new QTableWidgetItem(QString("%1").arg(event->miditype)));
	tblDebug->setItem(rowNumber, 2, new QTableWidgetItem(QString("%1").arg(event->midichannel)));
	tblDebug->setItem(rowNumber, 3, new QTableWidgetItem(QString("%1").arg(event->midino)));
	tblDebug->setItem(rowNumber, 4, new QTableWidgetItem(QString("%1").arg(toHex(QString::number(value)))));
        tblDebug->selectRow(rowNumber);
}

void DlgPrefMidiDevice::slotUpdate() {
	m_pMidi->enableDebug(this);
}

void DlgPrefMidiDevice::slotApply() {
	//Save device settings (we may not have changed devices, so force save)
	if (labelDeviceName->text() != "") {
		saveSettings(labelDeviceName->text(), comboMidiRxEnable->currentText(), comboMidiTxEnable->currentText());
		m_pDeviceSettings->Save();
	}
	m_pMidi->disableDebug();
}

// Called when a MIDI device is selected
void DlgPrefMidiDevice::slotGetInfo(QListWidgetItem * current, QListWidgetItem * previous) {
	// Moving away from another device, save the settings (unless nothing was selected)
	if (previous)
		saveSettings(previous->text(), comboMidiRxEnable->currentText(), comboMidiTxEnable->currentText());

	//Update the GUI for the new selected device
    comboMidiRxEnable->setEnabled(true);
    comboMidiTxEnable->setEnabled(true);
	labelDeviceName->setText(current->text());
	readSettings(); // Do we already have settings for this device?
	slotUpdateEnabled(); // Force update of device status for first selection
	// See if we have a preset available for this device and update the GUI
	presetAvailable(listMidiDevices->currentItem()->text());
}

void DlgPrefMidiDevice::slotUpdateEnabled() {
	if (deviceEnabled()) { // Is the device enabled or disabled
		labelDeviceStatus->setText("Enabled");
		m_pMidi->devOpen(labelDeviceName->text());
	} else {
		labelDeviceStatus->setText("Disabled");
		m_pMidi->devClose(labelDeviceName->text());
	}
	// Update device rx/tx status
	if (comboMidiRxEnable->currentText() == "Enabled")
	    m_pMidi->setRxStatus(labelDeviceName->text(), true);
	else
	    m_pMidi->setRxStatus(labelDeviceName->text(), false);
	if (comboMidiTxEnable->currentText() == "Enabled")
	    m_pMidi->setTxStatus(labelDeviceName->text(), true);
	else
	    m_pMidi->setTxStatus(labelDeviceName->text(), false);
	
}

void DlgPrefMidiDevice::saveSettings(QString device, QString Rx, QString Tx) {
	// Save current settings to the configobject
	m_pDeviceSettings->set(ConfigKey("[" + device + "]","RxEnable"), Rx);
	m_pDeviceSettings->set(ConfigKey("[" + device + "]","TxEnable"), Tx);
}

void DlgPrefMidiDevice::readSettings() {
	// Read in settings from the configobject for the current device
        if (m_pDeviceSettings->getValueString(ConfigKey("[" + labelDeviceName->text() + "]","RxEnable")) == "Enabled") {
        	comboMidiRxEnable->setCurrentIndex(1);
        	m_pMidi->setRxStatus(labelDeviceName->text(), true);
        } else {
        	comboMidiRxEnable->setCurrentIndex(0);
        	m_pMidi->setRxStatus(labelDeviceName->text(), false);
        }
        if (m_pDeviceSettings->getValueString(ConfigKey("[" + labelDeviceName->text() + "]","TxEnable")) == "Enabled") {
        	comboMidiTxEnable->setCurrentIndex(1);
        	m_pMidi->setTxStatus(labelDeviceName->text(), true);
        } else {
        	comboMidiTxEnable->setCurrentIndex(0);
        	m_pMidi->setTxStatus(labelDeviceName->text(), false);
        }
}

bool DlgPrefMidiDevice::deviceEnabled() {
	// Return true if either send or receive are enabled for the current device
	return comboMidiRxEnable->currentText() == "Enabled"
			|| comboMidiTxEnable->currentText() == "Enabled";
}

void DlgPrefMidiDevice::presetAvailable(QString deviceName) {
	// Is there a preset available
    QString qConfigPath = m_pConfig->getConfigPath();
	QStringList * midiConfigList = m_pMidi->getConfigList(qConfigPath.append("midi/"));

	for (QStringList::Iterator it = midiConfigList->begin(); it != midiConfigList->end(); ++it ) {
		// For each xml preset, open the preset
		QString presetPath = m_pConfig->getValueString(ConfigKey("[Config]","Path")).append("midi/").append(*it);
		QDomElement doc = WWidget::openXMLFile(presetPath, "MixxxMIDIPreset");
		//Grab the first deviceid tag
		QDomNode n = doc.firstChild().firstChild().nextSiblingElement("deviceid");
		while(!n.isNull()) {
		// While there are deviceid tags left, convert to elements and compare
			QDomElement e = n.toElement();
			if(!e.isNull()) {
				if (deviceName == e.text()) {
					// We found a match, update the label and button, and stop
					labelPresetAvailable->setText(*it);
					btnLoadDefaults->setEnabled(true);
					return;
				}
			}
			// Next deviceid
			n = n.nextSiblingElement("deviceid");
		}
	}
	// 2. Try keywords
		// TBD
	btnLoadDefaults->setEnabled(false);
}

// Opens devices that are enabled and sets up Rx/Tx for each device
void DlgPrefMidiDevice::setupDevices() {
	// For each device
	for (int i = 0; i < listMidiDevices->count(); i++) {
		// Simulate device selection
		listMidiDevices->setCurrentRow(i);
		slotGetInfo(listMidiDevices->item(i), 0);
	}
	// Clear the selection to make the dialog appear normal again
	listMidiDevices->clearSelection();
	labelDeviceName->setText("");
	labelPresetAvailable->setText("");
    comboMidiRxEnable->setEnabled(false);
    comboMidiTxEnable->setEnabled(false);
}

/* TODO:
 * - Make the device list static, not instanced.
 * Functions for:
 *   - Determining if there is a preset available given a device name
 *     - bool isPresetAvailable(string)  - find a preset by breaking apart device name
 *       - getPreset (find preset based on keyword)
 *     - Device names reported by the MIDI drivers may not be the same across
 *       OSs due to different implementations. Possible solutions: 'similar
 *       search' or multiple device names in presets?
 *     - Use device IDs instead? Can we get these regardless of OS?
 * - Make the dialog open the devices (if needed)
 * - Move all midiobject stuff to mididevicehandler and pass pointer to midi.
 */
