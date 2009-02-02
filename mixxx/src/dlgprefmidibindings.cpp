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
#include "midiinputmappingtablemodel.h"
#include "midichanneldelegate.h"
#include "miditypedelegate.h"
#include "dlgprefmidibindings.h"
#include "widget/wwidget.h"
#include "configobject.h"
#include "midimapping.h"

#ifdef __MIDISCRIPT__
#include "script/midiscriptengine.h"
#endif

const QStringList options = (QStringList() << "Normal" << "Script-Binding" << "Invert" << "Rot64" << "Rot64Inv"
        << "Rot64Fast" << "Diff" << "Button" << "Switch" << "HercJog"
        << "Spread64" << "SelectKnob");

QStringList controKeyOptionChoices;

const QStringList outputTypeChoices = (QStringList() << "light");

DlgPrefMidiBindings::DlgPrefMidiBindings(QWidget *parent, MidiObject &midi, ConfigObject<ConfigValue> *pConfig) :  QWidget(parent), Ui::DlgPrefMidiBindingsDlg(), m_rMidi(midi) {
    setupUi(this);
    m_pConfig = pConfig;
    singleLearning = false;
    groupLearning = false;

    //Tell the input mapping table widget which data model it should be viewing
    //(note that m_pInputMappingTableView is defined in the .ui file!)
    m_pInputMappingTableView->setModel((QAbstractItemModel*)m_rMidi.getMidiMapping()->getMidiInputMappingTableModel());

    m_pInputMappingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pInputMappingTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pInputMappingTableView->verticalHeader()->hide();

    //Set up the cool item delegates for the mapping tables
    m_pMidiChannelDelegate = new MidiChannelDelegate();
    m_pMidiTypeDelegate = new MidiTypeDelegate();
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDITYPE, m_pMidiTypeDelegate);
    m_pInputMappingTableView->setItemDelegateForColumn(MIDIINPUTTABLEINDEX_MIDICHANNEL, m_pMidiChannelDelegate);

    // Connect buttons to slots
    connect(btnSingleLearn, SIGNAL(clicked()), this, SLOT(slotSingleLearnToggle()));
    connect(btnGroupLearn, SIGNAL(clicked()), this, SLOT(slotGroupLearnToggle()));
    connect(btnImportXML, SIGNAL(clicked()), this, SLOT(slotImportXML()));
    connect(btnExportXML, SIGNAL(clicked()), this, SLOT(slotExportXML()));

    connect(btnClearBindings, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(btnRemoveBinding, SIGNAL(clicked()), this, SLOT(slotRemoveBinding()));
    connect(btnAddBinding, SIGNAL(clicked()), this, SLOT(slotAddBinding()));

    m_rMidi.disableMidiLearn();
}

DlgPrefMidiBindings::~DlgPrefMidiBindings() {
    //delete m_pMidiConfig;
    delete m_pMidiChannelDelegate;
}

/* loadPreset(QString)
 * Asks MidiMapping to load a set of MIDI bindings from an XML file
 */
void DlgPrefMidiBindings::loadPreset(QString path) {
    m_rMidi.getMidiMapping()->loadPreset(path);
}


/* slotUpdate()
 * Called when the dialog is loaded.
 */
void DlgPrefMidiBindings::slotUpdate() {

}

void DlgPrefMidiBindings::slotRemoveBinding()
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
            //qSort(selectedIndices);

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

/* slotApply()
 * Called when the OK button is pressed.
 */
void DlgPrefMidiBindings::slotApply() {
    /* User has pressed OK, so write the controls to the DOM, reload the MIDI
     * bindings, and save the default XML file. */
    m_rMidi.getMidiMapping()->savePreset();   // use default bindings path
    m_rMidi.getMidiMapping()->applyPreset();
    m_rMidi.disableMidiLearn();
}

/*
 * singleLearn(ConfigValueMidi *, QString)
 * Sets the currently row(s) to the MIDI value and Device. Typically called by
 * the MIDI device handler. Prompts the user to confirm if more than one row is
 * selected.
 */
void DlgPrefMidiBindings::singleLearn(ConfigValueMidi *value, QString device) {
    //m_pInputMappingTableView->model()->

}

/* groupLearn(ConfigValueMidi *, QString)
 * A midi learn message has been received during group learn mode. Sets the
 * current row in the group to the given MIDI message, and waits to be called
 * again for the next row. If the end of the selection is reached, group learn
 * is disabled.
 */
void DlgPrefMidiBindings::groupLearn(ConfigValueMidi *value, QString device) {

}

/* slotSingleLearnToggle()
 * The user has pressed the Single Learn toggle button. Enable or disable the
 * single learning mode and update the status accordingly.
 */
void DlgPrefMidiBindings::slotSingleLearnToggle() {
    // Toggle the status variable
    singleLearning = !singleLearning;
    if (singleLearning) {
        // Enable MIDI Hook
        m_rMidi.enableMidiLearn(this);
        labelStatus->setText("Single MIDI Learn: waiting...");
        // Can't do group learning while single learning
        btnGroupLearn->setEnabled(false);
    } else {
        // Disable MIDI Hook
        m_rMidi.disableMidiLearn();
        labelStatus->setText("Single MIDI Learn disabled");
        btnGroupLearn->setEnabled(true);
    }
}

/* slotGroupLearnToggle()
 * The user has pressed the Group Learn button. Midi Learn is enabled, and the
 * status is updated, indicating Mixxx is waiting for a Message. */
void DlgPrefMidiBindings::slotGroupLearnToggle() {
    groupLearning = !groupLearning;
    if (groupLearning) {
        // Enable MIDI Hook
        m_rMidi.enableMidiLearn(this);
        labelStatus->setText("Group MIDI Learn: waiting...");
        // Can't do group learning while single learning
        btnSingleLearn->setEnabled(false);
    } else {
        // Disable MIDI Hook
        m_rMidi.disableMidiLearn();
        labelStatus->setText("Group MIDI Learn disabled");
        btnSingleLearn->setEnabled(true);
    }
}

/* slotImportXML()
 * Prompts the user for an XML preset and loads it.
 */
void DlgPrefMidiBindings::slotImportXML() {
    QString fileName = QFileDialog::getOpenFileName(this,
            "Import Mixxx MIDI Preset", m_pConfig->getConfigPath().append("midi/"),
            "Preset Files (*.xml)");
    if (!fileName.isNull()) {
        loadPreset(fileName);
        m_rMidi.getMidiMapping()->applyPreset();
    }
    m_pInputMappingTableView->update();
}

/* slotExportXML()
 * Prompts the user for an XML preset and saves it.
 */
void DlgPrefMidiBindings::slotExportXML() {
    QString fileName = QFileDialog::getSaveFileName(this,
            "Export Mixxx MIDI Preset", m_pConfig->getConfigPath().append("midi/"),
            "Preset Files (*.xml)");
    if (!fileName.isNull()) m_rMidi.getMidiMapping()->savePreset(fileName);
}

/* slotClear()
 * Clears the table and DOM of all bindings.
 */
void DlgPrefMidiBindings::slotClear() {
    if (QMessageBox::warning(this, "Clear Input Bindings",
            "Are you sure you want to clear all bindings?",
            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;

    //Remove all the rows from the data model (ie. the MIDI mapping).
    MidiInputMappingTableModel* tableModel = dynamic_cast<MidiInputMappingTableModel*>(m_pInputMappingTableView->model());
    if (tableModel) {
        tableModel->removeRows(0, tableModel->rowCount());
    }

}


/* slotAddBinding()
 * Adds a binding to the table, with a minimum of the control and device info.
 */
void DlgPrefMidiBindings::slotAddBinding() {
    // TODO: This function is going to be totally broken.

    bool ok = true;

    QStringList selectionList;

    QString device = QInputDialog::getItem(this, tr("1/6 Select Device"), tr("Select a device to control"), selectionList, 0, true, &ok);
    if (!ok) return;

    QString controlKey = QInputDialog::getItem(this, tr("2/6 Select Control Group + Key"), tr("Select Control Group + Key"), getControlKeyList(), 0, false,  &ok);
    if (!ok) return;
    QString group = controlKey.trimmed().split(" ").at(0).trimmed();
    QString key = controlKey.trimmed().split(" ").at(1).trimmed();

    selectionList = QStringList();
    selectionList << "Ctrl" << "Key" << "Pitch";
    QString miditype = QInputDialog::getItem(this, tr("3/6 Select MIDI Type"), tr("Select MIDI Type"), selectionList, 0, false,  &ok);
    if (!ok) return;

    QString midino = QString::number(QInputDialog::getInteger(this, tr("4/6 Select Midi No."), tr("Type in the MIDI number value"), 1, 0, 127, 1, &ok));
    if (!ok) return;

    QString midichan = QString::number(QInputDialog::getInteger(this, tr("5/6 Select Midi Channel"), tr("Type in the MIDI channel value (ch)"), 1, 0, 127, 1, &ok));
    if (!ok) return;

    //    selectionList = QStringList(getControlTypeList());
    //    if (selectionList.size() == 0 && tblBindings->rowCount() > 0) { selectionList.append(tblBindings->item(tblBindings->rowCount() - 1,3)->text()); }
    //    QString controltype = QInputDialog::getItem(this, tr("7/8 Select Control"), tr("Select a function of Mixxx to control"), selectionList, 0, true, &ok);
    //        if (!ok) return;
    QString controltype = "";

    selectionList = QStringList(options);
    QString option = QInputDialog::getItem(this, tr("6/6 Select Option"), tr("Select an optional behaviour modifier"), selectionList, 0, true, &ok);
    if (!ok) return;

    // At this stage we have enough information to create a blank, learnable binding
    m_rMidi.getMidiMapping()->addInputControl((MidiType)miditype.toInt(), midino.toInt(), midichan.toInt(),
                                              group, key, (MidiOption)option.toInt());
                        //FUCK! The "option" thing above will be garbage when converted to an int, since it's
                        //        some string describing the midi option in words, not a string number like "2".

    //tblBindings->selectRow(tblBindings->rowCount() - 1); // Focus the row just added
}

/* getControlKeyList()
 * Gets the list of control objects
 */
QStringList DlgPrefMidiBindings::getControlKeyList() {
    // midi/BindableConfigKeys.txt = grep "ConfigKey" *.cpp | grep \\[ | sed -e 's/.*ConfigKey("//g' | cut -d \) -f1 | sed -e 's/[",]/ /g' -e 's/ \+/ /g' | egrep -ve '[+:>]' | sort -u | egrep -e "\[Channel[12]\]|\[Master\]|\[Playlist\]" > midi/BindableConfigKeys.txt
    if (controKeyOptionChoices.count() == 0) {
        QFile input(m_pConfig->getConfigPath() + "midi/BindableConfigKeys.txt");

        if (!input.open(QIODevice::ReadOnly)) return QStringList();

        while (!input.atEnd()) {
            QString line = input.readLine().trimmed();
            if (line.isEmpty() || line.indexOf('#') == 0) continue; // ignore # hashed out comments
            if (!line.isNull()) controKeyOptionChoices.append(line);
        }
        input.close();
    }
    return controKeyOptionChoices;
}
