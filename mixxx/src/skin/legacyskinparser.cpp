// legacyskinparser.cpp
// Created 9/19/2010 by RJ Ryan (rryan@mit.edu)

#include <QtGlobal>
#include <QtDebug>
#include <QDir>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>

#include "controlobject.h"
#include "controlobjectthreadwidget.h"

#include "mixxxkeyboard.h"
#include "playermanager.h"
#include "basetrackplayer.h"
#include "library/library.h"
#include "waveformviewerfactory.h"
#include "xmlparse.h"

#include "skin/legacyskinparser.h"
#include "skin/colorschemeparser.h"

#include "widget/wwidget.h"
#include "widget/wabstractcontrol.h"
#include "widget/wknob.h"
#include "widget/wslidercomposed.h"
#include "widget/wpushbutton.h"
#include "widget/wdisplay.h"
#include "widget/wvumeter.h"
#include "widget/wstatuslight.h"
#include "widget/wlabel.h"
#include "widget/wtracktext.h"
#include "widget/wtrackproperty.h"
#include "widget/wnumber.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberbpm.h"
#include "widget/wnumberrate.h"
#include "widget/woverview.h"

#include "widget/wvisualsimple.h"
#include "widget/wglwaveformviewer.h"
#include "widget/wwaveformviewer.h"

#include "widget/wsearchlineedit.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

#include "widget/wskincolor.h"
#include "widget/wpixmapstore.h"



LegacySkinParser::LegacySkinParser(ConfigObject<ConfigValue>* pConfig,
                                   MixxxKeyboard* pKeyboard,
                                   PlayerManager* pPlayerManager,
                                   Library* pLibrary)
        : m_pConfig(pConfig),
          m_pKeyboard(pKeyboard),
          m_pPlayerManager(pPlayerManager),
          m_pLibrary(pLibrary) {

}

LegacySkinParser::~LegacySkinParser() {

}

bool LegacySkinParser::canParse(QString skinPath) {
    QDir skinDir(skinPath);

    if (!skinDir.exists()) {
        return false;
    }

    if (!skinDir.exists("skin.xml"))
        return false;

    // TODO check skin.xml for compliance

    return true;
}

// static
QDomElement LegacySkinParser::openSkin(QString skinPath) {
    QDir skinDir(skinPath);

    if (!skinDir.exists()) {
        return QDomElement();
    }

    QString skinXmlPath = skinDir.filePath("skin.xml");
    QFile skinXmlFile(skinXmlPath);

    if (!skinXmlFile.open(QIODevice::ReadOnly)) {
        return QDomElement();
    }

    QDomDocument skin("skin");

    if (!skin.setContent(&skinXmlFile)) {
        return QDomElement();
    }

    // TODO(XXX) legacy
    WWidget::setPixmapPath(skinPath.append("/"));

    skinXmlFile.close();
    return skin.documentElement();
}

// static
QList<QString> LegacySkinParser::getSchemeList(QString qSkinPath) {

    QDomElement docElem = openSkin(qSkinPath);
    QList<QString> schlist;

    QDomNode colsch = docElem.namedItem("Schemes");
    if (!colsch.isNull() && colsch.isElement()) {
        QDomNode sch = colsch.firstChild();

        while (!sch.isNull()) {
            QString thisname = XmlParse::selectNodeQString(sch, "Name");
            schlist.append(thisname);
            sch = sch.nextSibling();
        }
    }

    return schlist;
}

bool LegacySkinParser::compareConfigKeys(QDomNode node, QString key)
{
    QDomNode n = node;

    // Loop over each <Connection>, check if it's ConfigKey matches key
    while (!n.isNull())
    {
        n = XmlParse::selectNode(n, "Connection");
        if (!n.isNull())
        {
            if  (XmlParse::selectNodeQString(n, "ConfigKey").contains(key))
                return true;
        }
    }
    return false;
}

void LegacySkinParser::setControlDefaults(QDomNode node, WAbstractControl* pControl) {
    if (compareConfigKeys(node, "[Master],headMix"))
    {
        pControl->setDefaultValue(0.);
    }
    else if (compareConfigKeys(node, "],volume") && // Matches [ChannelX],volume
             !compareConfigKeys(node, "[Master],volume"))
    {
        pControl->setDefaultValue(127.);
    }
}

QWidget* LegacySkinParser::parseSkin(QString skinPath, QWidget* pParent) {
    QDomElement skinDocument = openSkin(skinPath);

    if (skinDocument.isNull()) {
        // TODO error message
        qDebug() << "Could not load skin.";
        return NULL;
    }

    ColorSchemeParser::setupLegacyColorSchemes(skinDocument, m_pConfig);

    QStringList skinPaths(skinPath);
    QDir::setSearchPaths("skin", skinPaths);

    return parseNode(skinDocument, pParent);
}


QWidget* LegacySkinParser::parseNode(QDomElement node, QWidget* pParent) {
    QString nodeName = node.nodeName();
    //qDebug() << "parseNode" << node.nodeName();

    if (nodeName == "skin") {
        // Root of the document
    }

    // TODO(rryan) replace with a map to function pointers?
    if (nodeName == "Background") {
        return parseBackground(node, pParent);
    } else if (nodeName == "SliderComposed") {
        return parseSliderComposed(node, pParent);
    } else if (nodeName == "PushButton") {
        return parsePushButton(node, pParent);
    } else if (nodeName == "Overview") {
        return parseOverview(node, pParent);
    } else if (nodeName == "Visual") {
        return parseVisual(node, pParent);
    } else if (nodeName == "Text") {
        return parseText(node, pParent);
    } else if (nodeName == "TrackProperty") {
        return parseTrackProperty(node, pParent);
    } else if (nodeName == "VuMeter") {
        return parseVuMeter(node, pParent);
    } else if (nodeName == "StatusLight") {
        return parseStatusLight(node, pParent);
    } else if (nodeName == "Display") {
        return parseDisplay(node, pParent);
    } else if (nodeName == "NumberRate") {
        return parseNumberRate(node, pParent);
    } else if (nodeName == "NumberPos") {
        return parseNumberPos(node, pParent);
    } else if (nodeName == "NumberBpm") {
        return parseNumberBpm(node, pParent);
    } else if (nodeName == "Number") {
        return parseNumber(node, pParent);
    } else if (nodeName == "Label") {
        return parseLabel(node, pParent);
    } else if (nodeName == "Knob") {
        return parseKnob(node, pParent);
    } else if (nodeName == "TableView") {
        return parseTableView(node, pParent);
    } else if (nodeName == "WidgetGroup") {
        return parseWidgetGroup(node, pParent);
    } else if (nodeName == "Style") {
        return parseStyle(node, pParent);
    }

    // Descend chilren, should only happen for the root node
    QDomNodeList children = node.childNodes();

    for (int i = 0; i < children.count(); ++i) {
        QDomNode node = children.at(i);

        if (node.isElement()) {
            parseNode(node.toElement(), pParent);
        }
    }

    return pParent;
}

QWidget* LegacySkinParser::parseWidgetGroup(QDomElement node, QWidget* pParent) {
    QWidget* pGroup = new QGroupBox(pParent);

    setupWidget(node, pGroup);

    if (!XmlParse::selectNode(node, "Layout").isNull()) {
        QString layout = XmlParse::selectNodeQString(node, "Layout");
        if (layout == "vertical") {

        } else if (layout == "horizontal") {

        }
    }

    QDomNode childrenNode = XmlParse::selectNode(node, "Children");
    if (!childrenNode.isNull()) {
        // Descend chilren
        QDomNodeList children = childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (node.isElement()) {
                parseNode(node.toElement(), pGroup);
            }
        }
    }

    return pGroup;
}

QWidget* LegacySkinParser::parseBackground(QDomElement node, QWidget* pParent) {
    QLabel* bg = new QLabel(pParent);

    QString filename = XmlParse::selectNodeQString(node, "Path");
    QPixmap* background = WPixmapStore::getPixmapNoCache(WWidget::getPath(filename));

    bg->move(0, 0);
    if (background != NULL)
        bg->setPixmap(*background);
    bg->lower();

    pParent->move(0,0);
    pParent->setFixedSize(background->width(), background->height());
		pParent->setMinimumSize(background->width(), background->height());

    QColor c(0,0,0); // Default background color is now black, if people want to do <invert/> filters they'll have to figure something out for this.
    if (!XmlParse::selectNode(node, "BgColor").isNull()) {
        c.setNamedColor(XmlParse::selectNodeQString(node, "BgColor"));
    }

    QPalette palette;
    palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
    pParent->setBackgroundRole(QPalette::Window);
    pParent->setPalette(palette);
    pParent->setAutoFillBackground(true);

    return bg;
}

QWidget* LegacySkinParser::parsePushButton(QDomElement node, QWidget* pParent) {
    WPushButton* p = new WPushButton(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    return p;
}

QWidget* LegacySkinParser::parseSliderComposed(QDomElement node, QWidget* pParent) {
    WSliderComposed* p = new WSliderComposed(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    setControlDefaults(node, p);
    return p;
}

QWidget* LegacySkinParser::parseOverview(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);

    // TODO(XXX) This is a memory leak, but it's tiny. We have to do this for
    // now while everything expects groups as const char* because otherwise
    // we'll be providing a pointer to QString's internal memory, which might
    // get free'd at some point.
    const char* pSafeChannelStr = strdup(channelStr.toAscii().constData());

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (pPlayer == NULL)
        return NULL;

    WOverview* p = new WOverview(pSafeChannelStr, pParent);

    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);

    // Connect the player's load and unload signals to the overview widget.
    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            p, SLOT(slotLoadNewWaveform(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            p, SLOT(slotUnloadTrack(TrackPointer)));

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        p->slotLoadNewWaveform(pTrack);
    }

    return p;
}

QWidget* LegacySkinParser::parseVisual(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    // TODO(XXX) This is a memory leak, but it's tiny. We have to do this for
    // now while everything expects groups as const char* because otherwise
    // we'll be providing a pointer to QString's internal memory, which might
    // get free'd at some point.
    const char* pSafeChannelStr = strdup(channelStr.toAscii().constData());

    if (pPlayer == NULL)
        return NULL;

    WaveformRenderer* pWaveformRenderer = pPlayer->getWaveformRenderer();

    WaveformViewerType type;
    QWidget* widget = NULL;
    type = WaveformViewerFactory::createWaveformViewer(pSafeChannelStr, pParent,
                                                       m_pConfig, &widget, pWaveformRenderer);
    widget->installEventFilter(m_pKeyboard);

    // Hook up the wheel Control Object to the Visual Controller
    ControlObjectThreadWidget * p = new ControlObjectThreadWidget(
        ControlObject::getControl(ConfigKey(channelStr, "wheel")));

    p->setWidget((QWidget *)widget, true, true, true, Qt::LeftButton);

    setupWidget(node, widget);
    if (type == WAVEFORM_GL) {
        ((WGLWaveformViewer*)widget)->setup(node);
    } else if (type == WAVEFORM_WIDGET) {
        ((WWaveformViewer *)widget)->setup(node);
    } else if (type == WAVEFORM_SIMPLE) {
        ((WVisualSimple*)widget)->setup(node);
    }
    setupConnections(node, widget);

    connect(widget, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    return widget;
}

QWidget* LegacySkinParser::parseText(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WTrackText* p = new WTrackText(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);

    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            p, SLOT(slotTrackLoaded(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            p, SLOT(slotTrackUnloaded(TrackPointer)));

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        p->slotTrackLoaded(pTrack);
    }

    return p;
}

QWidget* LegacySkinParser::parseTrackProperty(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);


    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WTrackProperty* p = new WTrackProperty(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);

    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            p, SLOT(slotTrackLoaded(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            p, SLOT(slotTrackUnloaded(TrackPointer)));

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        p->slotTrackLoaded(pTrack);
    }

    return p;
}

QWidget* LegacySkinParser::parseVuMeter(QDomElement node, QWidget* pParent) {
    WVuMeter * p = new WVuMeter(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    return p;
}

QWidget* LegacySkinParser::parseStatusLight(QDomElement node, QWidget* pParent) {
    WStatusLight * p = new WStatusLight(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    return p;
}

QWidget* LegacySkinParser::parseDisplay(QDomElement node, QWidget* pParent) {
    WDisplay * p = new WDisplay(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    return p;
}

QWidget* LegacySkinParser::parseNumberRate(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);

    // TODO(XXX) This is a memory leak, but it's tiny. We have to do this for
    // now while everything expects groups as const char* because otherwise
    // we'll be providing a pointer to QString's internal memory, which might
    // get free'd at some point.
    const char* pSafeChannelStr = strdup(channelStr.toAscii().constData());

    QColor c(255,255,255);
    if (!XmlParse::selectNode(node, "BgColor").isNull()) {
        c.setNamedColor(XmlParse::selectNodeQString(node, "BgColor"));
    }

    QPalette palette;
    //palette.setBrush(QPalette::Background, WSkinColor::getCorrectColor(c));
    palette.setBrush(QPalette::Button, Qt::NoBrush);


    WNumberRate * p = new WNumberRate(pSafeChannelStr, pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->setPalette(palette);

    return p;
}

QWidget* LegacySkinParser::parseNumberPos(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);

    // TODO(XXX) This is a memory leak, but it's tiny. We have to do this for
    // now while everything expects groups as const char* because otherwise
    // we'll be providing a pointer to QString's internal memory, which might
    // get free'd at some point.
    const char* pSafeChannelStr = strdup(channelStr.toAscii().constData());

    WNumberPos* p = new WNumberPos(pSafeChannelStr, pParent);
    p->installEventFilter(m_pKeyboard);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    return p;
}

QWidget* LegacySkinParser::parseNumberBpm(QDomElement node, QWidget* pParent) {
    QString channelStr = lookupNodeGroup(node);

    // TODO(XXX) This is a memory leak, but it's tiny. We have to do this for
    // now while everything expects groups as const char* because otherwise
    // we'll be providing a pointer to QString's internal memory, which might
    // get free'd at some point.
    const char* pSafeChannelStr = strdup(channelStr.toAscii().constData());

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WNumberBpm * p = new WNumberBpm(pSafeChannelStr, pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);

    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            p, SLOT(slotTrackLoaded(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            p, SLOT(slotTrackUnloaded(TrackPointer)));

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        p->slotTrackLoaded(pTrack);
    }

    return p;
}

QWidget* LegacySkinParser::parseNumber(QDomElement node, QWidget* pParent) {
    WNumber* p = new WNumber(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    return p;
}

QWidget* LegacySkinParser::parseLabel(QDomElement node, QWidget* pParent) {
    WLabel * p = new WLabel(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    return p;
}

QWidget* LegacySkinParser::parseKnob(QDomElement node, QWidget* pParent) {
    WKnob * p = new WKnob(pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    setControlDefaults(node, p);
    return p;
}

QWidget* LegacySkinParser::parseTableView(QDomElement node, QWidget* pParent) {
    QStackedWidget* pTabWidget = new QStackedWidget(pParent);

    // Position
    if (!XmlParse::selectNode(node, "Pos").isNull())
    {
        QString pos = XmlParse::selectNodeQString(node, "Pos");
        int x = pos.left(pos.indexOf(",")).toInt();
        int y = pos.mid(pos.indexOf(",")+1).toInt();
        pTabWidget->move(x,y);
    }

    // Size
    if (!XmlParse::selectNode(node, "Size").isNull())
    {
        QString size = XmlParse::selectNodeQString(node, "Size");
        int x = size.left(size.indexOf(",")).toInt();
        int y = size.mid(size.indexOf(",")+1).toInt();
        pTabWidget->setFixedSize(x,y);
    }

    QWidget* pLibraryPage = new QWidget(pTabWidget);

    QGridLayout* pLibraryPageLayout = new QGridLayout(pLibraryPage);
    pLibraryPageLayout->setContentsMargins(0, 0, 0, 0);
    pLibraryPage->setLayout(pLibraryPageLayout);

    QSplitter* pSplitter = new QSplitter(pLibraryPage);

    WLibrary* pLibraryWidget = new WLibrary(pSplitter);
    pLibraryWidget->installEventFilter(m_pKeyboard);

    QWidget* pLibrarySidebarPage = new QWidget(pSplitter);

    WLibrarySidebar* pLibrarySidebar = new WLibrarySidebar(pLibrarySidebarPage);
    pLibrarySidebar->installEventFilter(m_pKeyboard);

    WSearchLineEdit* pLineEditSearch = new WSearchLineEdit(m_pConfig,
                                                           pLibrarySidebarPage);
    pLineEditSearch->setup(node);

    QVBoxLayout* vl = new QVBoxLayout(pLibrarySidebarPage);
    vl->setContentsMargins(0,0,0,0); //Fill entire space
    vl->addWidget(pLineEditSearch);
    vl->addWidget(pLibrarySidebar);
    pLibrarySidebarPage->setLayout(vl);

    // Connect search box signals to the library
    connect(pLineEditSearch, SIGNAL(search(const QString&)),
            pLibraryWidget, SLOT(search(const QString&)));
    connect(pLineEditSearch, SIGNAL(searchCleared()),
            pLibraryWidget, SLOT(searchCleared()));
    connect(pLineEditSearch, SIGNAL(searchStarting()),
            pLibraryWidget, SLOT(searchStarting()));
    connect(m_pLibrary, SIGNAL(restoreSearch(const QString&)),
            pLineEditSearch, SLOT(restoreSearch(const QString&)));

    m_pLibrary->bindWidget(pLibrarySidebar,
                           pLibraryWidget,
                           m_pKeyboard);
    // This must come after the bindWidget or we will not style any of the
    // LibraryView's because they have not been added yet.
    pLibraryWidget->setup(node);

    pSplitter->addWidget(pLibrarySidebarPage);
    pSplitter->addWidget(pLibraryWidget);

    // TODO(rryan) can we make this more elegant?
    QList<int> splitterSizes;
    splitterSizes.push_back(50);
    splitterSizes.push_back(500);
    pSplitter->setSizes(splitterSizes);

    // Add the splitter to the library page's layout, so it's
    // positioned/sized automatically
    pLibraryPageLayout->addWidget(pSplitter,
                                    1, 0, //From row 1, col 0,
                                    1,    //Span 1 row
                                    3,    //Span 3 cols
                                    0);   //Default alignment

    pTabWidget->addWidget(pLibraryPage);

    QString style = XmlParse::selectNodeQString(node, "Style");

    // Workaround to support legacy color styling
    QColor color(0,0,0);


    // Qt 4.7.0's GTK style is broken.
    bool hasQtKickedUsInTheNuts = false;

#ifdef __LINUX__
#define ohyesithas true
    QString QtVersion = qVersion();
    if (QtVersion == "4.7.0") {
        hasQtKickedUsInTheNuts = ohyesithas;
    }
#undef ohyesithas
#endif

    QString styleHack = "";

    if (!XmlParse::selectNode(node, "FgColor").isNull()) {
        color.setNamedColor(XmlParse::selectNodeQString(node, "FgColor"));
        color = WSkinColor::getCorrectColor(color);

        if (hasQtKickedUsInTheNuts) {
            styleHack.append(QString("QTreeView { color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("QTableView { color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("QTableView::item:!selected { color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("QTreeView::item:!selected { color: %1; }\n ").arg(color.name()));
        } else {
            styleHack.append(QString("WLibraryTableView { color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("WLibrarySidebar { color: %1; }\n ").arg(color.name()));
        }
        styleHack.append(QString("WSearchLineEdit { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QTextBrowser { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QLabel { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QRadioButton { color: %1; }\n ").arg(color.name()));
    }

    if (!XmlParse::selectNode(node, "BgColor").isNull()) {
        color.setNamedColor(XmlParse::selectNodeQString(node, "BgColor"));
        color = WSkinColor::getCorrectColor(color);
        if (hasQtKickedUsInTheNuts) {
            styleHack.append(QString("QTreeView {  background-color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("QTableView {  background-color: %1; }\n ").arg(color.name()));

            // Required for styling the item backgrounds, need to pick !selected
            styleHack.append(QString("QTreeView::item:!selected {  background-color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("QTableView::item:!selected {  background-color: %1; }\n ").arg(color.name()));

            // Styles the sidebar triangle area where there is no triangle
            styleHack.append(QString("QTreeView::branch:!has-children {  background-color: %1; }\n ").arg(color.name()));

            // We can't style the triangle portions because the triangle
            // disappears when we do background-color. I suspect they use
            // background-image instead of border-image, against their own
            // documentation's recommendation.
            // EDIT: Un-commented next line cause it works if we use custom images as triangle,
            // see https://bugs.launchpad.net/mixxx/+bug/690280 --jus 12/2010
            styleHack.append(QString("QTreeView::branch:has-children {  background-color: %1; }\n ").arg(color.name()));
        } else {
            styleHack.append(QString("WLibraryTableView {  background-color: %1; }\n ").arg(color.name()));
            styleHack.append(QString("WLibrarySidebar {  background-color: %1; }\n ").arg(color.name()));
        }

        styleHack.append(QString("WSearchLineEdit {  background-color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QTextBrowser {  background-color: %1; }\n ").arg(color.name()));
    }

    if (!XmlParse::selectNode(node, "BgColorRowEven").isNull()) {
        color.setNamedColor(XmlParse::selectNodeQString(node, "BgColorRowEven"));
        color = WSkinColor::getCorrectColor(color);

        if (hasQtKickedUsInTheNuts) {
            styleHack.append(QString("QTableView::item:!selected { background-color: %1; }\n ").arg(color.name()));
        } else {
            styleHack.append(QString("WLibraryTableView { background: %1; }\n ").arg(color.name()));
        }
    }

    if (!XmlParse::selectNode(node, "BgColorRowUneven").isNull()) {
        color.setNamedColor(XmlParse::selectNodeQString(node, "BgColorRowUneven"));
        color = WSkinColor::getCorrectColor(color);

        if (hasQtKickedUsInTheNuts) {
            styleHack.append(QString("QTableView::item:alternate:!selected { background-color: %1; }\n ").arg(color.name()));
        } else {
            styleHack.append(QString("WLibraryTableView { alternate-background-color: %1; }\n ").arg(color.name()));
        }
    }

    style.prepend(styleHack);

    pTabWidget->setStyleSheet(style);

    return pTabWidget;
}

QString LegacySkinParser::lookupNodeGroup(QDomElement node) {
    QString group = XmlParse::selectNodeQString(node, "Group");

    // If the group is not present, then check for a Channel, since legacy skins
    // will specify the channel as either 1 or 2.
    if (group.size() == 0) {
        int channel = XmlParse::selectNodeInt(node, "Channel");
        group = QString("[Channel%1]").arg(channel);
    }

    return group;
}

QWidget* LegacySkinParser::parseStyle(QDomElement node, QWidget* pParent) {
    QString style = node.text();
    pParent->setStyleSheet(style);
    return pParent;
}

void LegacySkinParser::setupWidget(QDomNode node, QWidget* pWidget) {
    // Position
    if (!XmlParse::selectNode(node, "Pos").isNull())
    {
        QString pos = XmlParse::selectNodeQString(node, "Pos");
        int x = pos.left(pos.indexOf(",")).toInt();
        int y = pos.mid(pos.indexOf(",")+1).toInt();
        pWidget->move(x,y);
    }

    // Size
    if (!XmlParse::selectNode(node, "Size").isNull())
    {
        QString size = XmlParse::selectNodeQString(node, "Size");
        int x = size.left(size.indexOf(",")).toInt();
        int y = size.mid(size.indexOf(",")+1).toInt();
        pWidget->setFixedSize(x,y);
    }

    // Tooltip
    if (!XmlParse::selectNode(node, "Tooltip").isNull()) {
        QString toolTip = XmlParse::selectNodeQString(node, "Tooltip");
        pWidget->setToolTip(toolTip);
    }

    QString style = XmlParse::selectNodeQString(node, "Style");
    if (style != "")
        pWidget->setStyleSheet(style);

}

void LegacySkinParser::setupConnections(QDomNode node, QWidget* pWidget) {
    // For each connection
    QDomNode con = XmlParse::selectNode(node, "Connection");

    while (!con.isNull())
    {
        // Get ConfigKey
        QString key = XmlParse::selectNodeQString(con, "ConfigKey");

        ConfigKey configKey = ConfigKey::parseCommaSeparated(key);

        // Check that the control exists
        ControlObject * control = ControlObject::getControl(configKey);

        if (control == NULL) {
            qWarning() << "Requested control does not exist:" << key;
            con = con.nextSibling();
            continue;
        }

        if (!XmlParse::selectNode(con, "OnOff").isNull() &&
            XmlParse::selectNodeQString(con, "OnOff")=="true")
        {
            // Connect control proxy to widget
            (new ControlObjectThreadWidget(control))->setWidgetOnOff(pWidget);
        }
        else
        {
            // Get properties from XML, or use defaults
            bool bEmitOnDownPress = true;
            if (XmlParse::selectNodeQString(con, "EmitOnDownPress").contains("false", Qt::CaseInsensitive))
                bEmitOnDownPress = false;

            bool connectValueFromWidget = true;
            if (XmlParse::selectNodeQString(con, "ConnectValueFromWidget").contains("false", Qt::CaseInsensitive))
                connectValueFromWidget = false;

            bool connectValueToWidget = true;
            if (XmlParse::selectNodeQString(con, "ConnectValueToWidget").contains("false", Qt::CaseInsensitive))
                connectValueToWidget = false;

            Qt::MouseButton state = Qt::NoButton;
            if (!XmlParse::selectNode(con, "ButtonState").isNull())
            {
                if (XmlParse::selectNodeQString(con, "ButtonState").contains("LeftButton", Qt::CaseInsensitive))
                    state = Qt::LeftButton;
                else if (XmlParse::selectNodeQString(con, "ButtonState").contains("RightButton", Qt::CaseInsensitive))
                    state = Qt::RightButton;
            }

            // Connect control proxy to widget
            (new ControlObjectThreadWidget(control))->setWidget(
                pWidget, connectValueFromWidget, connectValueToWidget,
                bEmitOnDownPress, state);

            // Add keyboard shortcut info to tooltip string
            QString tooltip = pWidget->toolTip();
            QString shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(configKey);
            if (!shortcut.isEmpty() && !tooltip.contains(shortcut, Qt::CaseInsensitive)) {
                tooltip.append(QString("\nShortcut: %1").arg(shortcut));
                pWidget->setToolTip(tooltip);
            }
        }
        con = con.nextSibling();
    }
}
