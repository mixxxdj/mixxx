/***************************************************************************
                          mixxx.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// #ifdef HAVE_CONFIG_H
// #include <config.h>
// #endif

#include <qaccel.h>
#include <qpushbutton.h>
#include <qtable.h>
#include "qknob.h"

#include "mixxx.h"
#include "filesave.xpm"
#include "fileopen.xpm"
#include "filenew.xpm"
#include "controlpushbutton.h"
#include "playeralsa.h"
#include "playerportaudio.h"
#include "enginepregain.h"
#include "enginefilterlbh.h"

MixxxApp::MixxxApp()
{
  qDebug("Start");

  setCaption(tr("Mixxx " VERSION));

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initActions();
  initMenuBar();
  initToolBar();
  initStatusBar();

  initDoc();
  initView();
  qDebug("Init playlist");
  // Connect play list table selection with "play new file"
  connect(view->playlist->TableList, SIGNAL(clicked(int,int,int,const QPoint &)),
          this,                      SLOT(slotChangePlay(int,int,int, const QPoint &)));

  viewToolBar->setOn(false);
  viewStatusBar->setOn(true);

  // Initialize midi:
  qDebug("Init midi...");
  midi = new MidiObject();

  // Initialize engines:

  EnginePregain *pregain = new EnginePregain(ADC7, midi);
  connect(view->channel->DialGain, SIGNAL(valueChanged(int)), pregain->pregainpot, SLOT(slotSetPosition(int)));
  connect(pregain->pregainpot, SIGNAL(recievedMidi(int)), view->channel->DialGain, SLOT(setValue(int)));
  engines.push_back(pregain);

  EngineFilterLBH *lbh_filter = new EngineFilterLBH(view->channel->DialFilterLow,
						    view->channel->DialFilterMiddle,
						    view->channel->DialFilterHigh ,midi);
  engines.push_back(lbh_filter);

  EngineClipping *clipping = new EngineClipping(view->channel->BulbClipping);
  engines.push_back(clipping);

  // Initialize player with a desired buffer size
  qDebug("Init player...");

#ifndef Q_WS_WIN
  player = new PlayerALSA(BUFFER_SIZE, &engines);
#else
  player = new PlayerPortAudio(BUFFER_SIZE, &engines);
#endif
  //player = new PlayerPortAudio(BUFFER_SIZE);

  qDebug("Init engine...");
  buffer = new EngineBuffer(view->playcontrol, view->channel, midi, view->playlist->TableList->item(0,1)->text());
  buffer->start();

  // Start audio
  qDebug("Start player...");
  player->start(buffer);
}

MixxxApp::~MixxxApp()
{
//	buffer->stop();
	delete buffer;
	player->stop();
	delete player;
}

/** initializes all QActions of the application */
void MixxxApp::initActions(){

  QPixmap openIcon, saveIcon, newIcon;
  newIcon = QPixmap(filenew);
  openIcon = QPixmap(fileopen);
  saveIcon = QPixmap(filesave);


  fileNew = new QAction(tr("New File"), newIcon, tr("&New"), QAccel::stringToKey(tr("Ctrl+N")), this);
  fileNew->setStatusTip(tr("Creates a new document"));
  fileNew->setWhatsThis(tr("New File\n\nCreates a new document"));
  connect(fileNew, SIGNAL(activated()), this, SLOT(slotFileNew()));

  fileOpen = new QAction(tr("Open File"), openIcon, tr("&Open..."), 0, this);
  fileOpen->setStatusTip(tr("Opens an existing document"));
  fileOpen->setWhatsThis(tr("Open File\n\nOpens an existing document"));
  connect(fileOpen, SIGNAL(activated()), this, SLOT(slotFileOpen()));

  fileSave = new QAction(tr("Save File"), saveIcon, tr("&Save"), QAccel::stringToKey(tr("Ctrl+S")), this);
  fileSave->setStatusTip(tr("Saves the actual document"));
  fileSave->setWhatsThis(tr("Save File.\n\nSaves the actual document"));
  connect(fileSave, SIGNAL(activated()), this, SLOT(slotFileSave()));

  fileSaveAs = new QAction(tr("Save File As"), tr("Save &as..."), 0, this);
  fileSaveAs->setStatusTip(tr("Saves the actual document under a new filename"));
  fileSaveAs->setWhatsThis(tr("Save As\n\nSaves the actual document under a new filename"));
  connect(fileSaveAs, SIGNAL(activated()), this, SLOT(slotFileSave()));

  fileClose = new QAction(tr("Close File"), tr("&Close"), QAccel::stringToKey(tr("Ctrl+W")), this);
  fileClose->setStatusTip(tr("Closes the actual document"));
  fileClose->setWhatsThis(tr("Close File\n\nCloses the actual document"));
  connect(fileClose, SIGNAL(activated()), this, SLOT(slotFileClose()));

  filePrint = new QAction(tr("Print File"), tr("&Print"), QAccel::stringToKey(tr("Ctrl+P")), this);
  filePrint->setStatusTip(tr("Prints out the actual document"));
  filePrint->setWhatsThis(tr("Print File\n\nPrints out the actual document"));
  connect(filePrint, SIGNAL(activated()), this, SLOT(slotFilePrint()));

  fileQuit = new QAction(tr("Exit"), tr("E&xit"), QAccel::stringToKey(tr("Ctrl+Q")), this);
  fileQuit->setStatusTip(tr("Quits the application"));
  fileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
  connect(fileQuit, SIGNAL(activated()), this, SLOT(slotFileQuit()));

  editCut = new QAction(tr("Cut"), tr("Cu&t"), QAccel::stringToKey(tr("Ctrl+X")), this);
  editCut->setStatusTip(tr("Cuts the selected section and puts it to the clipboard"));
  editCut->setWhatsThis(tr("Cut\n\nCuts the selected section and puts it to the clipboard"));
  connect(editCut, SIGNAL(activated()), this, SLOT(slotEditCut()));

  editCopy = new QAction(tr("Copy"), tr("&Copy"), QAccel::stringToKey(tr("Ctrl+C")), this);
  editCopy->setStatusTip(tr("Copies the selected section to the clipboard"));
  editCopy->setWhatsThis(tr("Copy\n\nCopies the selected section to the clipboard"));
  connect(editCopy, SIGNAL(activated()), this, SLOT(slotEditCopy()));

  editPaste = new QAction(tr("Paste"), tr("&Paste"), QAccel::stringToKey(tr("Ctrl+V")), this);
  editPaste->setStatusTip(tr("Pastes the clipboard contents to actual position"));
  editPaste->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to actual position"));
  connect(editPaste, SIGNAL(activated()), this, SLOT(slotEditPaste()));

  viewToolBar = new QAction(tr("Toolbar"), tr("Tool&bar"), 0, this, 0, true);
  viewToolBar->setStatusTip(tr("Enables/disables the toolbar"));
  viewToolBar->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
  connect(viewToolBar, SIGNAL(toggled(bool)), this, SLOT(slotViewToolBar(bool)));

  viewStatusBar = new QAction(tr("Statusbar"), tr("&Statusbar"), 0, this, 0, true);
  viewStatusBar->setStatusTip(tr("Enables/disables the statusbar"));
  viewStatusBar->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
  connect(viewStatusBar, SIGNAL(toggled(bool)), this, SLOT(slotViewStatusBar(bool)));

  helpAboutApp = new QAction(tr("About"), tr("&About..."), 0, this);
  helpAboutApp->setStatusTip(tr("About the application"));
  helpAboutApp->setWhatsThis(tr("About\n\nAbout the application"));
  connect(helpAboutApp, SIGNAL(activated()), this, SLOT(slotHelpAbout()));

}

void MixxxApp::initMenuBar()
{
  ///////////////////////////////////////////////////////////////////
  // MENUBAR

  ///////////////////////////////////////////////////////////////////
  // menuBar entry fileMenu
  fileMenu=new QPopupMenu();
  fileNew->addTo(fileMenu);
  fileOpen->addTo(fileMenu);
  fileClose->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileSave->addTo(fileMenu);
  fileSaveAs->addTo(fileMenu);
  fileMenu->insertSeparator();
  filePrint->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileQuit->addTo(fileMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry editMenu
  editMenu=new QPopupMenu();
  editCut->addTo(editMenu);
  editCopy->addTo(editMenu);
  editPaste->addTo(editMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry viewMenu
  viewMenu=new QPopupMenu();
  viewMenu->setCheckable(true);
  viewToolBar->addTo(viewMenu);
  viewStatusBar->addTo(viewMenu);
  ///////////////////////////////////////////////////////////////////
  // EDIT YOUR APPLICATION SPECIFIC MENUENTRIES HERE

  ///////////////////////////////////////////////////////////////////
  // menuBar entry helpMenu
  helpMenu=new QPopupMenu();
  helpAboutApp->addTo(helpMenu);

  ///////////////////////////////////////////////////////////////////
  // MENUBAR CONFIGURATION
  menuBar()->insertItem(tr("&File"), fileMenu);
  menuBar()->insertItem(tr("&Edit"), editMenu);
  menuBar()->insertItem(tr("&View"), viewMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem(tr("&Help"), helpMenu);

}

void MixxxApp::initToolBar()
{
  ///////////////////////////////////////////////////////////////////
  // TOOLBAR
  fileToolbar = new QToolBar(this, "file operations");
  fileNew->addTo(fileToolbar);
  fileOpen->addTo(fileToolbar);
  fileSave->addTo(fileToolbar);
  fileToolbar->addSeparator();
  QWhatsThis::whatsThisButton(fileToolbar);

}

void MixxxApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  //STATUSBAR
  statusBar()->message(tr("Ready."), 2000);
}

void MixxxApp::initDoc()
{
   doc=new MixxxDoc();
}

void MixxxApp::initView()
{
  ////////////////////////////////////////////////////////////////////
  // set the main widget here
  view=new MixxxView(this, doc);
  setCentralWidget(view);
}

bool MixxxApp::queryExit()
{
  int exit=QMessageBox::information(this, tr("Quit..."),
                                    tr("Do your really want to quit?"),
                                    QMessageBox::Ok, QMessageBox::Cancel);

  if (exit==1)
  {

  }
  else
  {

  };

  return (exit==1);
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


void MixxxApp::slotFileNew()
{
  statusBar()->message(tr("Creating new file..."));
  doc->newDoc();
  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileOpen()
{
  statusBar()->message(tr("Opening file..."));

  QString fileName = QFileDialog::getOpenFileName(0,0,this);
  if (!fileName.isEmpty())
  {
    doc->load(fileName);
    setCaption(fileName);
    QString message=tr("Loaded document: ")+fileName;
    statusBar()->message(message, 2000);
  }
  else
  {
    statusBar()->message(tr("Opening aborted"), 2000);
  }
}


void MixxxApp::slotFileSave()
{
  statusBar()->message(tr("Saving file..."));
  doc->save();
  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileSaveAs()
{
  statusBar()->message(tr("Saving file under new filename..."));
  QString fn = QFileDialog::getSaveFileName(0, 0, this);
  if (!fn.isEmpty())
  {
    doc->saveAs(fn);
  }
  else
  {
    statusBar()->message(tr("Saving aborted"), 2000);
  }

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileClose()
{
  statusBar()->message(tr("Closing file..."));

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFilePrint()
{
  statusBar()->message(tr("Printing..."));
  QPrinter printer;
  if (printer.setup(this))
  {
    QPainter painter;
    painter.begin(&printer);

    ///////////////////////////////////////////////////////////////////
    // TODO: Define printing by using the QPainter methods here

    painter.end();
  };

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotFileQuit()
{
  statusBar()->message(tr("Exiting application..."));
  ///////////////////////////////////////////////////////////////////
  // exits the Application
  if(doc->isModified())
  {
    if(queryExit())
    {
      qApp->quit();
    }
    else
    {

    };
  }
  else
  {
    qApp->quit();
  };

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotEditCut()
{
  statusBar()->message(tr("Cutting selection..."));

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotEditCopy()
{
  statusBar()->message(tr("Copying selection to clipboard..."));

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotEditPaste()
{
  statusBar()->message(tr("Inserting clipboard contents..."));

  statusBar()->message(tr("Ready."));
}


void MixxxApp::slotViewToolBar(bool toggle)
{
  statusBar()->message(tr("Toggle toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off

  if (toggle== false)
  {
    fileToolbar->hide();
  }
  else
  {
    fileToolbar->show();
  };

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotViewStatusBar(bool toggle)
{
  statusBar()->message(tr("Toggle statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off

  if (toggle == false)
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  statusBar()->message(tr("Ready."));
}

void MixxxApp::slotHelpAbout()
{
  QMessageBox::about(this,tr("About..."),
                      tr("Mixxx\nVersion " VERSION "\n(c) 2002 by Tue and Ken Haste Andersen") );
}

void MixxxApp::slotChangePlay(int row,int col,int button, const QPoint &)
{
  // stop playback and deallocate buffer
  player->stop();

  // Tell the buffer to get a new file:
  buffer->newtrack(view->playlist->TableList->item(row,1)->text());
  // Start buffer and playback
  player->start(buffer);
}


