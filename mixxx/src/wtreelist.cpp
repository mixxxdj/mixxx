/***************************************************************************
                          wtreelist.cpp  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Ingo Kossyk
    email                : kossyki@cs.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wtreelist.h"

/***
Definition of Class Global constants
***/
static int* FILE_T_FOLDER = (int*)1;
static int* FILE_T_MP3 =(int*)2;
static int* FILE_T_OGG = (int*)3;
static int* FILE_T_PLAYLIST = (int*)0;
/*********************************+
	Functions and Constructor for class WTreeItem which is
	derived from QListViewItem
**********************************/
//Constructs a new WTreeList Item as child of RootItem Parent
WTreeItem::WTreeItem(QListView * parent,int *fileType)
: QListViewItem(parent),Type(fileType)
{
	  
	this->setState(false);
	listParent = parent;
	this->setType(fileType);
	
}
//Alternate Constructor for sibbling items
WTreeItem::WTreeItem(QListViewItem * parent,int *fileType)
: QListViewItem(parent)
{
	this->setState(false);
	listParent  = ((WTreeItem*)parent)->listParent;
	this->setType(fileType);
	
}
//Returns the Type as int
int * WTreeItem::getType(){
	return Type;
}
//Set Type of the Item (look for the constants in the preclass statements
void WTreeItem::setType(int * fileType){
	
	Type = fileType;
	
}
//Deconstructer for WTreeItem
WTreeItem::~WTreeItem(){
	
}

//Returns the State of the Item
bool WTreeItem::getState(){
	
	return Expanded;
	
}	
//The Item Above 
QListViewItem * WTreeItem::itemAbove(){
	
	return this->parent();
	
}
//Set state of Item
void WTreeItem::setState(bool state){
	
	Expanded = state;
	
	
}
/*********************************+
	Functions and Constructor for class WTreeList which is
	derived from QListView
**********************************/
//Constructs WTreeList object as child of parent
WTreeList::WTreeList(QWidget * parent, const char*name)
: QListView( parent, name ){
	//Connecting double click and return pressed 
	connect( this, SIGNAL( doubleClicked( QListViewItem * ) ),
             this, SLOT( slotFolderSelected( QListViewItem * ) ) );
    connect( this, SIGNAL( returnPressed(  QListViewItem * ) ),
             this, SLOT( slotFolderSelected( QListViewItem * ) ) );
	QFont f("Helvetica");
    f.setPointSize(12);
    setFont(f);
	//setMouseTracking(false);
	m_sPlaylistdir = "";
	mousePressed = false;
	setFocusPolicy(QWidget::NoFocus);
	}
//Destroys the WTreeList
WTreeList::~WTreeList(){
	
}
//General Setup of the Class
void WTreeList::setup(QDomNode node)
{

    // Position
    if (!WWidget::selectNode(node, "Pos").isNull())
    {
        QString pos = WWidget::selectNodeQString(node, "Pos");
        int x = pos.left(pos.find(",")).toInt();
        int y = pos.mid(pos.find(",")+1).toInt();
        move(x,y);
    }

    // Size
    if (!WWidget::selectNode(node, "Size").isNull())
    {
        QString size = WWidget::selectNodeQString(node, "Size");
        int x = size.left(size.find(",")).toInt();
        int y = size.mid(size.find(",")+1).toInt();
        setFixedSize(x,y);
    }

    //Create the columns
	this->addColumn( tr( "Realtive Name" ) );
	this->addColumn( tr( "Type") );
	this->addColumn( tr( "Absolute Name") );
	//Set label for the TreeList
	this->header()->setLabel( 0, tr( "Name" ) );
    this->clear();
	this->setFrameStyle(QFrame::NoFrame);

	this->setAcceptDrops( TRUE );
	viewport()->setAcceptDrops( TRUE );
	
	// Background color
    if (!WWidget::selectNode(node, "BgColor").isNull())
    {
        QColor c;
        c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
        setPaletteBackgroundColor(c);
    }

    // Foreground color
    if (!WWidget::selectNode(node, "FgColor").isNull())
    {
        QColor c;
        c.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
        setPaletteForegroundColor(c);
    }
	 // Playlist repository
    if (!WWidget::selectNode(node, "Listpath").isNull())
    {
	WTreeItem * playListItem = new WTreeItem((QListView*)this);
	playListItem->setText(0,"Playlists");
	playListItem->setText(1,"Playlist Repository");
	playListItem->setText(2,WWidget::selectNodeQString(node,"PlsRootPath"));   
    playListItem->filePath =  WWidget::selectNodeQString(node,"PlsRootPath");
	}

    // Root Directory
    if (!WWidget::selectNode(node, "DirRootPath").isNull())
    {
     WTreeItem * rootItem = new WTreeItem((QListView*)this);
	rootItem->setText(0,"Archive");
	rootItem->setText(1,"Data Repository");
	rootItem->setText(2,WWidget::selectNodeQString(node,"DirRootPath"));
	rootItem->filePath =  WWidget::selectNodeQString(node,"DirRootPath");
    }
	
    // Setup column widths
    //setLeftMargin(0);
    //hideColumn(COL_INDEX);
    setColumnWidth(0, WWidget::selectNodeInt(node, "ColWidthName"));
    setColumnWidth(1, WWidget::selectNodeInt(node, "ColWidthType"));
    setColumnWidth(2, WWidget::selectNodeInt(node, "ColWidthFullPath"));

}
//If a dragged object gets moved
void WTreeList::contentsMouseMoveEvent( QMouseEvent* e )
{
    if ( mousePressed && ( presspos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
        mousePressed = FALSE;
        QListViewItem *item = itemAt( contentsToViewport(presspos) );
        if ( item ) {
            if ( QFile::exists(((WTreeItem*)item)->filePath) || ((WTreeItem*)item)->filePath.endsWith(".xml") ) {
        
				QStrList * source = new QStrList();
				source->append(&(*QUriDrag::localFileToUri(((WTreeItem*)item)->filePath)));
				QUriDrag* ud = new QUriDrag(viewport());
                ud->setUris( (*source) );
               	ud->dragCopy();
           
				
            }
        }
    }
}
//An item from the list if being selected
void WTreeList::contentsMousePressEvent( QMouseEvent* e )
{
    QListView::contentsMousePressEvent(e);
    QPoint p( contentsToViewport( e->pos() ) );
    QListViewItem *i = itemAt( p );
    if ( i ) {
 		//Disregard decoration clicks
        if ( p.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
             treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
             p.x() < header()->cellPos( header()->mapToActual( 0 ) ) ) {
            presspos = e->pos();
            mousePressed = TRUE;
        }
    }
}
//Mouse button was released
void WTreeList::contentsMouseReleaseEvent( QMouseEvent * )
{
    mousePressed = FALSE;
}
//Slot if one Item gets double clicked or selected and return is pressed
void WTreeList::slotFolderSelected( QListViewItem * item )
{
    //Is the item allready open ? Or does it even exist ?
	if(item){
		
		if(item->childCount() == 0)
			if(item->text(0) == "Playlists"&& !(((WTreeItem*)item)->getState())){
				((WTreeItem*)item)->setState(TRUE);
		        //Make the Items children visible
		        ((WTreeItem*)item)->setOpen(TRUE);
				populatePlaylists(item);	
			}else if((item->text(1) =="Data Repository" || item->text(1) == "Folder") && !(((WTreeItem*)item)->getState())) {
		        ((WTreeItem*)item)->setState(TRUE);
		        //Make the Items children visible
		        ((WTreeItem*)item)->setOpen(TRUE);
				populateTree(((WTreeItem*)item)->filePath,item);
			}else if(item->text(1) =="Playlist"){
				emit(loadPls(item->text(2)));
				qDebug("Emited Signale loadPls");
				}
		}else{
		((WTreeItem*)item)->setState(FALSE);
		//Make the Items children hidden
		((WTreeItem*)item)->setOpen(FALSE);
		}
	
}
//Update the directories of the Root items
void WTreeList::slotSetDirs(QString root,QString playlist)
{
	setRoot(root);
	setPlaylist(playlist);
}
//Deletes the root Item and adds a new one
void WTreeList::setPlaylist(QString sPlaylist)
{
	
	if(sPlaylist){
	takeItem(findItem("Playlists",0)); //Remove first Sibling TreeItem (Root Item)
	WTreeItem * newRoot = new WTreeItem(this, FILE_T_FOLDER);
	newRoot->filePath = sPlaylist; //Set Root Path according to the String that was emited
	newRoot->setText(0, "Playlists");
	newRoot->setText(1, "Playlist Repository");
	newRoot->setText(2, sPlaylist);
	//newRoot->setState(TRUE);
	newRoot->setOpen(TRUE);
	this->populatePlaylists((QListViewItem*)newRoot);
	m_sPlaylistdir = sPlaylist;
	mousePressed = false;
	}
	
}
//Deletes the root Item and adds a new one
void WTreeList::setRoot(QString sRoot)
{
	
	if(sRoot){
	takeItem(findItem("Archive",0)); //Remove first Sibling TreeItem (Root Item)
	WTreeItem * newRoot = new WTreeItem(this, FILE_T_FOLDER);
	newRoot->filePath = sRoot; //Set Root Path according to the String that was emited
	newRoot->setText(0, "Archive");
	newRoot->setText(1, "Data Repository");
	newRoot->setText(2, sRoot);
	
	}
	
}
//returns Playlist Root Item
WTreeItem * WTreeList::getPlsRoot()
{
 	setCurrentItem(findItem("Playlists",0));
	return (WTreeItem *)currentItem();
	}
//Returns Directory Root Item
WTreeItem * WTreeList::getRoot()
{
	setCurrentItem(findItem("Archive",0));
	return (WTreeItem *)currentItem();
}
//If the Playlist item is being opened get all playlists
bool WTreeList::populatePlaylists(QListViewItem * listItem)
{
	// Initialize xml file:
    QFile opmlFile(((WTreeItem*)listItem)->filePath);
	QDomDocument domXML("Mixxx_Track_List");
    if (!domXML.setContent( &opmlFile))
    {
        qWarning(tr("Error: Cannot open file %1").arg(((WTreeItem*)listItem)->filePath));
		/**QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( ((WTreeItem*)listItem)->filePath ) );**/
        opmlFile.close();
    }
    opmlFile.close();
	
    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
    int iVersion = 0;
    //Get all Playlists in the Tracklistfile
    QDomNode node = elementRoot.firstChild();
   
	while ( !node.isNull() )
    {
	 if( node.isElement() && node.nodeName() == "Playlist")
		{
		qDebug(node.toElement().attribute( "Name" ));
		WTreeItem * tempItem = new WTreeItem(listItem, FILE_T_PLAYLIST);//Create the child	
		tempItem->filePath = (* new QString(tr(node.toElement().attribute( "Name" ))));
		tempItem->setText(2,tr(node.toElement().attribute( "Name" )));//Set pathname for child
		tempItem->setText(1,tr("Playlist"));
		tempItem->setText(0,node.toElement().attribute( "Name" ).remove(".xml"));	
		}
		node = node.nextSibling();
	}
	return true;
}
//Once the Folder was selected get all Subdirs and build the children tree
bool WTreeList::populateTree(QString dirPath, QListViewItem * listItem)
{
	//Set the working directory to the current one
	workingDir = new QDir(dirPath.latin1());
	if(!workingDir->exists()){
			
		return false;
	
	//exit if the dir doesnt exist... 
	
	}else{
		
		//workingDir->setFilter(QDir::Dirs);
		subDirList = workingDir->entryInfoList();
		
;
		
		
		if(!subDirList->count()==0){ //Are there any subdirs at all ?
		
		QFileInfoListIterator workingDir_it(*subDirList);
		workingDir_it += 2; // skip . and ..
		
		
			
		
		while((currentObject=workingDir_it.current()))//And while there are more subdirs
			{
			QStringList lst( QStringList::split( "/", currentObject->filePath() ) );
			QStringList::Iterator lstIt = lst.end();
				--lstIt;
				if((*lstIt).endsWith(".mp3")){ //Is it a mp3 file ?
					WTreeItem * tempItem = new WTreeItem(listItem, FILE_T_MP3);//Create the child	
					tempItem->setText(2,tr(currentObject->filePath()));//Set pathname for child
					tempItem->filePath = (* new QString(tr(currentObject->filePath())));
					tempItem->setText(1,tr("MP3 file"));
					tempItem->setText(0,(*lstIt));
				}else if((*lstIt).endsWith(".ogg")){ //Is it an ogg file ?
					WTreeItem * tempItem = new WTreeItem(listItem, FILE_T_OGG);//Create the child	
					tempItem->setText(2,tr(currentObject->filePath()));//Set pathname for child
					tempItem->filePath = (* new QString(tr(currentObject->filePath())));
					tempItem->setText(1,tr("OGG file"));
					tempItem->setText(0,(*lstIt));
				}else if(currentObject->isDir()){
					WTreeItem * tempItem = new WTreeItem(listItem, FILE_T_FOLDER);//Create the child	
					tempItem->setText(2,tr(currentObject->filePath()));//Set pathname for child
					tempItem->filePath = (* new QString(tr(currentObject->filePath())));
					tempItem->setText(1,tr("Folder"));
					tempItem->setText(0,(*lstIt));
				}										
			++workingDir_it;
		  }
		}
	}
		
	return true;
    
 }
