// legacyskinparser.cpp
// Created 9/19/2010 by RJ Ryan (rryan@mit.edu)

#include "skin/legacyskinparser.h"

#include <QDir>
#include <QGridLayout>
#include <QLabel>
#include <QMutexLocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QtDebug>
#include <QtGlobal>

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "controlobjectthreadwidget.h"

#include "mixxxkeyboard.h"
#include "playermanager.h"
#include "basetrackplayer.h"
#include "library/library.h"
#include "xmlparse.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermanager.h"

#include "skin/colorschemeparser.h"
#include "skin/propertybinder.h"

#include "widget/wwidget.h"
#include "widget/wabstractcontrol.h"
#include "widget/wknob.h"
#include "widget/wslidercomposed.h"
#include "widget/wpushbutton.h"
#include "widget/wdisplay.h"
#include "widget/wvumeter.h"
#include "widget/wstatuslight.h"
#include "widget/wlabel.h"
#include "widget/wtime.h"
#include "widget/wtracktext.h"
#include "widget/wtrackproperty.h"
#include "widget/wnumber.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberrate.h"
#include "widget/woverviewlmh.h"
#include "widget/woverviewhsv.h"
#include "widget/wspinny.h"
#include "widget/wwaveformviewer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wsearchlineedit.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wskincolor.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidgetstack.h"
#include "widget/wwidgetgroup.h"
#include "widget/wkey.h"

using mixxx::skin::SkinManifest;

QList<const char*> LegacySkinParser::s_channelStrs;
QMutex LegacySkinParser::s_safeStringMutex;

ControlObject* controlFromConfigKey(ConfigKey configKey, bool* created) {
    ControlObject* pControl = ControlObject::getControl(configKey);

    if (pControl) {
        if (created) {
            *created = false;
        }
        return pControl;
    }

    // TODO(rryan): Make this configurable by the skin.
    qWarning() << "Requested control does not exist:"
               << QString("%1,%2").arg(configKey.group, configKey.item)
               << "Creating it.";
    // Since the usual behavior here is to create a skin-defined push
    // button, actually make it a push button and set it to toggle.
    ControlPushButton* controlButton = new ControlPushButton(configKey);
    controlButton->setButtonMode(ControlPushButton::TOGGLE);
    if (created) {
        *created = true;
    }
    return controlButton;
}

LegacySkinParser::LegacySkinParser(ConfigObject<ConfigValue>* pConfig,
                                   MixxxKeyboard* pKeyboard,
                                   PlayerManager* pPlayerManager,
                                   ControllerManager* pControllerManager,
                                   Library* pLibrary,
                                   VinylControlManager* pVCMan)
    : m_pConfig(pConfig),
      m_pKeyboard(pKeyboard),
      m_pPlayerManager(pPlayerManager),
      m_pControllerManager(pControllerManager),
      m_pLibrary(pLibrary),
      m_pVCManager(pVCMan),
      m_pParent(NULL) {
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
        qDebug() << "LegacySkinParser::openSkin - skin dir do not exist:" << skinPath;
        return QDomElement();
    }

    QString skinXmlPath = skinDir.filePath("skin.xml");
    QFile skinXmlFile(skinXmlPath);

    if (!skinXmlFile.open(QIODevice::ReadOnly)) {
        qDebug() << "LegacySkinParser::openSkin - can't open file:" << skinXmlPath
                 << "in directory:" << skinDir.path();
        return QDomElement();
    }

    QDomDocument skin("skin");

    QString errorMessage;
    int errorLine;
    int errorColumn;

    if (!skin.setContent(&skinXmlFile,&errorMessage,&errorLine,&errorColumn)) {
        qDebug() << "LegacySkinParser::openSkin - setContent failed see"
                 << "line:" << errorLine << "column:" << errorColumn;
        qDebug() << "LegacySkinParser::openSkin - message:" << errorMessage;
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

// static
void LegacySkinParser::freeChannelStrings() {
    QMutexLocker lock(&s_safeStringMutex);
    for (int i = 0; i < s_channelStrs.length(); ++i) {
        if (s_channelStrs[i]) {
            delete [] s_channelStrs[i];
        }
        s_channelStrs[i] = NULL;
    }
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

SkinManifest LegacySkinParser::getSkinManifest(QDomElement skinDocument) {
    QDomNode manifest_node = skinDocument.namedItem("manifest");
    SkinManifest manifest;
    if (manifest_node.isNull() || !manifest_node.isElement()) {
        return manifest;
    }
    manifest.set_title(XmlParse::selectNodeQString(manifest_node, "title").toStdString());
    manifest.set_author(XmlParse::selectNodeQString(manifest_node, "author").toStdString());
    manifest.set_version(XmlParse::selectNodeQString(manifest_node, "version").toStdString());
    manifest.set_language(XmlParse::selectNodeQString(manifest_node, "language").toStdString());
    manifest.set_description(XmlParse::selectNodeQString(manifest_node, "description").toStdString());
    manifest.set_license(XmlParse::selectNodeQString(manifest_node, "license").toStdString());

    QDomNode attributes_node = manifest_node.namedItem("attributes");
    if (!attributes_node.isNull() && attributes_node.isElement()) {
        QDomNodeList attribute_nodes = attributes_node.toElement().elementsByTagName("attribute");
        for (int i = 0; i < attribute_nodes.length(); ++i) {
            QDomNode attribute_node = attribute_nodes.item(i);
            if (attribute_node.isElement()) {
                QDomElement attribute_element = attribute_node.toElement();
                QString configKey = attribute_element.attribute("config_key");
                QString value = attribute_element.text();
                SkinManifest::Attribute* attr = manifest.add_attribute();
                attr->set_config_key(configKey.toStdString());
                attr->set_value(value.toStdString());
            }
        }
    }
    return manifest;
}

QWidget* LegacySkinParser::parseSkin(QString skinPath, QWidget* pParent) {
    if (m_pParent) {
        qDebug() << "ERROR: Somehow a parent already exists -- you are probably re-using a LegacySkinParser which is not advisable!";
    }
    QDomElement skinDocument = openSkin(skinPath);

    if (skinDocument.isNull()) {
        qDebug() << "LegacySkinParser::parseSkin - failed for skin:" << skinPath;
        return NULL;
    }

    SkinManifest manifest = getSkinManifest(skinDocument);

    // Apply SkinManifest attributes.
    for (int i = 0; i < manifest.attribute_size(); ++i) {
        const SkinManifest::Attribute& attribute = manifest.attribute(i);
        if (!attribute.has_config_key()) {
            continue;
        }
        ConfigKey configKey = ConfigKey::parseCommaSeparated(
            QString::fromStdString(attribute.config_key()));

        bool ok = false;
        double value = QString::fromStdString(attribute.value()).toDouble(&ok);
        if (ok) {
            ControlObjectThread cot(configKey);
            cot.slotSet(value);
        }
    }

    ColorSchemeParser::setupLegacyColorSchemes(skinDocument, m_pConfig);

    QStringList skinPaths(skinPath);
    QDir::setSearchPaths("skin", skinPaths);

    // don't parent till here so the first opengl waveform doesn't screw
    // up --bkgood
    // I'm disregarding this return value because I want to return the
    // created parent so MixxxApp can use it for nefarious purposes (
    // fullscreen mostly) --bkgood
    m_pParent = pParent;
    return parseNode(skinDocument);
}

QWidget* LegacySkinParser::parseNode(QDomElement node) {
    QString nodeName = node.nodeName();
    //qDebug() << "parseNode" << node.nodeName();

    // TODO(rryan) replace with a map to function pointers?

    // Root of the document
    if (nodeName == "skin") {
        // Parent all the skin widgets to an inner QWidget (this was MixxxView
        // in <=1.8, MixxxView was a subclass of QWidget), and then wrap it in
        // an outer widget. The Background parser parents the background image
        // to the inner widget but then sets the fill color of the outer widget
        // so that fullscreen will expand with the right color to fill in the
        // non-background areas. We put the inner widget in a layout inside the
        // outer widget so that it stays centered in fullscreen mode.
        QWidget* pOuterWidget = new QWidget(m_pParent);
        QWidget* pInnerWidget = new QWidget(pOuterWidget);

        QString layout = XmlParse::selectNodeQString(node, "Layout");
        QLayout* pInnerLayout = NULL;
        if (layout == "vertical") {
            pInnerLayout = new QVBoxLayout(pInnerWidget);
            pInnerLayout->setSpacing(0);
            pInnerLayout->setContentsMargins(0, 0, 0, 0);
            pInnerWidget->setLayout(pInnerLayout);
        } else if (layout == "horizontal") {
            pInnerLayout = new QHBoxLayout(pInnerWidget);
            pInnerLayout->setSpacing(0);
            pInnerLayout->setContentsMargins(0, 0, 0, 0);
            pInnerWidget->setLayout(pInnerLayout);
        }

        // Background is only valid at the top level.
        QDomElement background = XmlParse::selectElement(node, "Background");
        if (!background.isNull()) {
            parseBackground(background, pOuterWidget, pInnerWidget);
        }

        // Interpret <Size>, <SizePolicy>, <Style>, etc. tags for the root node.
        setupWidget(node, pInnerWidget, false);

        m_pParent = pInnerWidget;

        QDomNode childrenNode = XmlParse::selectNode(node, "Children");

        // For backwards compatibility, allow children to be specified outside
        // of a <Children> block.
        QDomNodeList children = childrenNode.isNull() ? node.childNodes() :
                childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (node.isElement()) {
                QWidget* pChild = parseNode(node.toElement());
                if (pChild != NULL && pInnerLayout != NULL) {
                    pInnerLayout->addWidget(pChild);
                }
            }
        }

        // Keep innerWidget centered (for fullscreen).
        pOuterWidget->setLayout(new QHBoxLayout(pOuterWidget));
        pOuterWidget->layout()->setContentsMargins(0, 0, 0, 0);
        pOuterWidget->layout()->addWidget(pInnerWidget);
        return pOuterWidget;
    } else if (nodeName == "SliderComposed") {
        return parseSliderComposed(node);
    } else if (nodeName == "PushButton") {
        return parsePushButton(node);
    } else if (nodeName == "Overview") {
        return parseOverview(node);
    } else if (nodeName == "Visual") {
        return parseVisual(node);
    } else if (nodeName == "Text") {
        return parseText(node);
    } else if (nodeName == "TrackProperty") {
        return parseTrackProperty(node);
    } else if (nodeName == "VuMeter") {
        return parseVuMeter(node);
    } else if (nodeName == "StatusLight") {
        return parseStatusLight(node);
    } else if (nodeName == "Display") {
        return parseDisplay(node);
    } else if (nodeName == "NumberRate") {
        return parseNumberRate(node);
    } else if (nodeName == "NumberPos") {
        return parseNumberPos(node);
    } else if (nodeName == "Number" || nodeName == "NumberBpm") {
        // NumberBpm is deprecated, and is now the same as a Number
        return parseNumber(node);
    } else if (nodeName == "Label") {
        return parseLabel(node);
    } else if (nodeName == "Knob") {
        return parseKnob(node);
    } else if (nodeName == "TableView") {
        return parseTableView(node);
    } else if (nodeName == "SearchBox") {
        return parseSearchBox(node);
    } else if (nodeName == "WidgetGroup") {
        return parseWidgetGroup(node);
    } else if (nodeName == "WidgetStack") {
        return parseWidgetStack(node);
    } else if (nodeName == "Spinny") {
        return parseSpinny(node);
    } else if (nodeName == "Time") {
        return parseTime(node);
    } else if (nodeName == "Splitter") {
        return parseSplitter(node);
    } else if (nodeName == "LibrarySidebar") {
        return parseLibrarySidebar(node);
    } else if (nodeName == "Library") {
        return parseLibrary(node);
    } else if (nodeName == "Key") {
        return parseKey(node);
    } else {
        qDebug() << "Invalid node name in skin:" << nodeName;
    }

    return NULL;
}

QWidget* LegacySkinParser::parseSplitter(QDomElement node) {
    QSplitter* pSplitter = new QSplitter(m_pParent);
    pSplitter->setObjectName("Splitter");
    setupWidget(node, pSplitter);

    // Default orientation is horizontal.
    if (!XmlParse::selectNode(node, "Orientation").isNull()) {
        QString layout = XmlParse::selectNodeQString(node, "Orientation");
        if (layout == "vertical") {
            pSplitter->setOrientation(Qt::Vertical);
        } else if (layout == "horizontal") {
            pSplitter->setOrientation(Qt::Horizontal);
        }
    }

    QDomNode childrenNode = XmlParse::selectNode(node, "Children");

    QWidget* pOldParent = m_pParent;
    m_pParent = pSplitter;

    if (!childrenNode.isNull()) {
        // Descend chilren
        QDomNodeList children = childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (node.isElement()) {
                QWidget* pChild = parseNode(node.toElement());

                if (pChild == NULL)
                    continue;

                pSplitter->addWidget(pChild);
            }
        }
    }

    if (!XmlParse::selectNode(node, "SplitSizes").isNull()) {
        QString sizesJoined = XmlParse::selectNodeQString(node, "SplitSizes");
        QStringList sizesSplit = sizesJoined.split(",");
        QList<int> sizes;
        bool ok = false;
        foreach (const QString& sizeStr, sizesSplit) {
            sizes.push_back(sizeStr.toInt(&ok));
            if (!ok) {
                break;
            }
        }
        if (sizes.length() != pSplitter->count()) {
            qDebug() << "<SplitSizes> for <Splitter> (" << sizesJoined
                     << ") does not match the number of children nodes:"
                     << pSplitter->count();
            ok = false;
        }
        if (ok) {
            pSplitter->setSizes(sizes);
        }

    }

    m_pParent = pOldParent;
    return pSplitter;
}

QWidget* LegacySkinParser::parseWidgetGroup(QDomElement node) {
    WWidgetGroup* pGroup = new WWidgetGroup(m_pParent);
    setupWidget(node, pGroup);
    pGroup->setup(node);
    setupConnections(node, pGroup);

    QDomNode childrenNode = XmlParse::selectNode(node, "Children");

    QWidget* pOldParent = m_pParent;
    m_pParent = pGroup;

    if (!childrenNode.isNull()) {
        // Descend children
        QDomNodeList children = childrenNode.childNodes();
        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);
            if (node.isElement()) {
                QWidget* pChild = parseNode(node.toElement());

                if (pChild == NULL) {
                    continue;
                }

                pGroup->addWidget(pChild);
            }
        }
    }
    m_pParent = pOldParent;
    return pGroup;
}

QWidget* LegacySkinParser::parseWidgetStack(QDomElement node) {
    ControlObject* pNextControl = NULL;
    QString nextControl = XmlParse::selectNodeQString(node, "NextControl");
    bool createdNext = false;
    if (nextControl.length() > 0) {
        ConfigKey nextConfigKey = ConfigKey::parseCommaSeparated(nextControl);
        pNextControl = controlFromConfigKey(nextConfigKey, &createdNext);
    }

    ControlObject* pPrevControl = NULL;
    bool createdPrev = false;
    QString prevControl = XmlParse::selectNodeQString(node, "PrevControl");
    if (prevControl.length() > 0) {
        ConfigKey prevConfigKey = ConfigKey::parseCommaSeparated(prevControl);
        pPrevControl = controlFromConfigKey(prevConfigKey, &createdPrev);
    }

    WWidgetStack* pStack = new WWidgetStack(m_pParent, pNextControl, pPrevControl);
    pStack->setObjectName("WidgetStack");
    pStack->setContentsMargins(0, 0, 0, 0);
    setupWidget(node, pStack);
    setupConnections(node, pStack);

    if (createdNext && pNextControl) {
        pNextControl->setParent(pStack);
    }

    if (createdPrev && pPrevControl) {
        pPrevControl->setParent(pStack);
    }

    QDomNode childrenNode = XmlParse::selectNode(node, "Children");

    QWidget* pOldParent = m_pParent;
    m_pParent = pStack;

    if (!childrenNode.isNull()) {
        // Descend chilren
        QDomNodeList children = childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (!node.isElement()) {
                continue;
            }
            QDomElement element = node.toElement();

            QWidget* pChild = parseNode(element);
            if (pChild == NULL)
                continue;

            ControlObject* pControl = NULL;
            QString trigger_configkey = element.attribute("trigger");
            if (trigger_configkey.length() > 0) {
                ConfigKey configKey = ConfigKey::parseCommaSeparated(trigger_configkey);
                bool created;
                pControl = controlFromConfigKey(configKey, &created);
                if (created) {
                    // If we created the control, parent it to the child widget so
                    // it doesn't leak.
                    pControl->setParent(pChild);
                }
            }
            pStack->addWidgetWithControl(pChild, pControl);
        }
    }
    m_pParent = pOldParent;
    return pStack;
}

QWidget* LegacySkinParser::parseBackground(QDomElement node,
                                           QWidget* pOuterWidget,
                                           QWidget* pInnerWidget) {
    QLabel* bg = new QLabel(pInnerWidget);

    QString filename = XmlParse::selectNodeQString(node, "Path");
    QPixmap* background = WPixmapStore::getPixmapNoCache(WWidget::getPath(filename));

    bg->move(0, 0);
    if (background != NULL && !background->isNull()) {
        bg->setPixmap(*background);
    }

    bg->lower();

    pInnerWidget->move(0,0);
    if (background != NULL && !background->isNull()) {
        pInnerWidget->setFixedSize(background->width(), background->height());
        pOuterWidget->setMinimumSize(background->width(), background->height());
    }

    // Default background color is now black, if people want to do <invert/>
    // filters they'll have to figure something out for this.
    QColor c(0,0,0);
    if (!XmlParse::selectNode(node, "BgColor").isNull()) {
        c.setNamedColor(XmlParse::selectNodeQString(node, "BgColor"));
    }

    QPalette palette;
    palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
    pOuterWidget->setBackgroundRole(QPalette::Window);
    pOuterWidget->setPalette(palette);
    pOuterWidget->setAutoFillBackground(true);

    // WPixmapStore::getPixmapNoCache() allocated background and gave us
    // ownership. QLabel::setPixmap makes a copy, so we have to delete this.
    delete background;

    return bg;
}

QWidget* LegacySkinParser::parsePushButton(QDomElement node) {
    WPushButton* p = new WPushButton(m_pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseSliderComposed(QDomElement node) {
    WSliderComposed* p = new WSliderComposed(m_pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseOverview(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (pPlayer == NULL)
        return NULL;

    WOverview* overviewWidget = NULL;

    // HSV = "1" or "Filtered" = "0" (LMH) waveform overview type
    if (m_pConfig->getValueString(ConfigKey("[Waveform]","WaveformOverviewType"),
            "0").toInt() == 0) {
        overviewWidget = new WOverviewLMH(pSafeChannelStr, m_pConfig, m_pParent);
    } else {
        overviewWidget = new WOverviewHSV(pSafeChannelStr, m_pConfig, m_pParent);
    }

    connect(overviewWidget, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    setupWidget(node, overviewWidget);
    overviewWidget->setup(node);
    setupConnections(node, overviewWidget);
    overviewWidget->installEventFilter(m_pKeyboard);
    overviewWidget->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());

    // Connect the player's load and unload signals to the overview widget.
    connect(pPlayer, SIGNAL(loadTrack(TrackPointer)),
            overviewWidget, SLOT(slotLoadNewTrack(TrackPointer)));
    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            overviewWidget, SLOT(slotTrackLoaded(TrackPointer)));
    connect(pPlayer, SIGNAL(loadTrackFailed(TrackPointer)),
               overviewWidget, SLOT(slotUnloadTrack(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            overviewWidget, SLOT(slotUnloadTrack(TrackPointer)));

    //just in case track already loaded
    overviewWidget->slotLoadNewTrack(pPlayer->getLoadedTrack());

    return overviewWidget;
}

QWidget* LegacySkinParser::parseVisual(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    if (pPlayer == NULL)
        return NULL;

    WWaveformViewer* viewer = new WWaveformViewer(pSafeChannelStr, m_pConfig, m_pParent);
    viewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    factory->setWaveformWidget(viewer, node);

    //qDebug() << "::parseVisual: parent" << m_pParent << m_pParent->size();
    //qDebug() << "::parseVisual: viewer" << viewer << viewer->size();

    viewer->installEventFilter(m_pKeyboard);
    viewer->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());

    // Connect control proxy to widget, so delete can be handled by the QT object tree
    ControlObjectThreadWidget * p = new ControlObjectThreadWidget(
            channelStr, "wheel", viewer);

    p->setWidget((QWidget *)viewer, true, false,
                 ControlObjectThreadWidget::EMIT_ON_PRESS, Qt::RightButton);

    setupWidget(node, viewer);

    // connect display with loading/unloading of tracks
    QObject::connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                     viewer, SLOT(onTrackLoaded(TrackPointer)));
    QObject::connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
                     viewer, SLOT(onTrackUnloaded(TrackPointer)));

    setupConnections(node, viewer);

    connect(viewer, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    // if any already loaded
    viewer->onTrackLoaded(pPlayer->getLoadedTrack());

    return viewer;
}

QWidget* LegacySkinParser::parseText(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WTrackText* p = new WTrackText(m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());

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

QWidget* LegacySkinParser::parseTrackProperty(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);


    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WTrackProperty* p = new WTrackProperty(m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());

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

QWidget* LegacySkinParser::parseVuMeter(QDomElement node) {
    WVuMeter * p = new WVuMeter(m_pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseStatusLight(QDomElement node) {
    WStatusLight * p = new WStatusLight(m_pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseDisplay(QDomElement node) {
    WDisplay * p = new WDisplay(m_pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseNumberRate(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    QColor c(255,255,255);
    if (!XmlParse::selectNode(node, "BgColor").isNull()) {
        c.setNamedColor(XmlParse::selectNodeQString(node, "BgColor"));
    }

    QPalette palette;
    //palette.setBrush(QPalette::Background, WSkinColor::getCorrectColor(c));
    palette.setBrush(QPalette::Button, Qt::NoBrush);

    WNumberRate * p = new WNumberRate(pSafeChannelStr, m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    p->setPalette(palette);

    return p;
}

QWidget* LegacySkinParser::parseNumberPos(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    WNumberPos* p = new WNumberPos(pSafeChannelStr, m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseNumber(QDomElement node) {
    WNumber* p = new WNumber(m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseLabel(QDomElement node) {
    WLabel * p = new WLabel(m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseTime(QDomElement node) {
    WTime *p = new WTime(m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseKnob(QDomElement node) {
    WKnob * p = new WKnob(m_pParent);
    setupWidget(node, p);
    p->setup(node);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseSpinny(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    const char* pSafeChannelStr = safeChannelString(channelStr);
    WSpinny* spinny = new WSpinny(m_pParent, m_pVCManager);
    if (!spinny->isValid()) {
        delete spinny;
        QLabel* dummy = new QLabel(m_pParent);
        //: Shown when Spinny can not be displayd. Please keep \n unchanged
        dummy->setText(tr("No OpenGL\nsupport."));
        return dummy;
    }
    setupWidget(node, spinny);

    WaveformWidgetFactory::instance()->addTimerListener(spinny);
    connect(spinny, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    spinny->setup(node, pSafeChannelStr);
    setupConnections(node, spinny);
    spinny->installEventFilter(m_pKeyboard);
    spinny->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return spinny;
}

QWidget* LegacySkinParser::parseSearchBox(QDomElement node) {
    WSearchLineEdit* pLineEditSearch = new WSearchLineEdit(m_pConfig, m_pParent);
    setupWidget(node, pLineEditSearch);
    pLineEditSearch->setup(node);

    // Connect search box signals to the library
    connect(pLineEditSearch, SIGNAL(search(const QString&)),
            m_pLibrary, SIGNAL(search(const QString&)));
    connect(pLineEditSearch, SIGNAL(searchCleared()),
            m_pLibrary, SIGNAL(searchCleared()));
    connect(pLineEditSearch, SIGNAL(searchStarting()),
            m_pLibrary, SIGNAL(searchStarting()));
    connect(m_pLibrary, SIGNAL(restoreSearch(const QString&)),
            pLineEditSearch, SLOT(restoreSearch(const QString&)));

    return pLineEditSearch;
}

QWidget* LegacySkinParser::parseLibrary(QDomElement node) {
    WLibrary* pLibraryWidget = new WLibrary(m_pParent);
    pLibraryWidget->installEventFilter(m_pKeyboard);
    pLibraryWidget->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());

    // Connect Library search signals to the WLibrary
    connect(m_pLibrary, SIGNAL(search(const QString&)),
            pLibraryWidget, SLOT(search(const QString&)));

    m_pLibrary->bindWidget(pLibraryWidget, m_pKeyboard);

    // This must come after the bindWidget or we will not style any of the
    // LibraryView's because they have not been added yet.
    setupWidget(node, pLibraryWidget);

    return pLibraryWidget;
}

QWidget* LegacySkinParser::parseLibrarySidebar(QDomElement node) {
    WLibrarySidebar* pLibrarySidebar = new WLibrarySidebar(m_pParent);
    pLibrarySidebar->installEventFilter(m_pKeyboard);
    pLibrarySidebar->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    m_pLibrary->bindSidebarWidget(pLibrarySidebar);
    setupWidget(node, pLibrarySidebar);
    return pLibrarySidebar;
}

QWidget* LegacySkinParser::parseTableView(QDomElement node) {
    QStackedWidget* pTabWidget = new QStackedWidget(m_pParent);

    setupPosition(node, pTabWidget);
    setupSize(node, pTabWidget);

    // set maximum width to prevent growing to qSplitter->sizeHint()
    // Note: sizeHint() may be greater in skins for tiny screens
    int width = pTabWidget->minimumWidth();
    if (width == 0) {
        width = m_pParent->minimumWidth();
    }
    pTabWidget->setMaximumWidth(width);

    QWidget* pLibraryPage = new QWidget(pTabWidget);

    QGridLayout* pLibraryPageLayout = new QGridLayout(pLibraryPage);
    pLibraryPageLayout->setContentsMargins(0, 0, 0, 0);
    pLibraryPage->setLayout(pLibraryPageLayout);

    QSplitter* pSplitter = new QSplitter(pLibraryPage);

    QWidget* oldParent = m_pParent;

    m_pParent = pSplitter;
    QWidget* pLibraryWidget = parseLibrary(node);

    QWidget* pLibrarySidebarPage = new QWidget(pSplitter);
    m_pParent = pLibrarySidebarPage;
    QWidget* pLibrarySidebar = parseLibrarySidebar(node);
    QWidget* pLineEditSearch = parseSearchBox(node);
    m_pParent = oldParent;

    QVBoxLayout* vl = new QVBoxLayout(pLibrarySidebarPage);
    vl->setContentsMargins(0,0,0,0); //Fill entire space
    vl->addWidget(pLineEditSearch);
    vl->addWidget(pLibrarySidebar);
    pLibrarySidebarPage->setLayout(vl);

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
    pTabWidget->setStyleSheet(getLibraryStyle(node));

    return pTabWidget;
}

QString LegacySkinParser::getLibraryStyle(QDomNode node) {
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

    // Style the library preview button with a default image.
    styleHack = (
        "#LibraryPreviewButton { background: transparent; border: 0; }"
        "#LibraryPreviewButton:checked {"
        "  image: url(:/images/library/ic_library_preview_pause.png);"
        "}"
        "#LibraryPreviewButton:!checked {"
        "  image: url(:/images/library/ic_library_preview_play.png);"
        "}");
    // Style the library BPM Button with a default image
    styleHack.append(QString(
        "QPushButton#LibraryBPMButton { background: transparent; border: 0; }"
        "QPushButton#LibraryBPMButton:checked {image: url(:/images/library/ic_library_checked.png);}"
        "QPushButton#LibraryBPMButton:!checked {image: url(:/images/library/ic_library_unchecked.png);}"));

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
        styleHack.append(QString("QSpinBox { color: %1; }\n ").arg(color.name()));
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
        styleHack.append(QString("QSpinBox {  background-color: %1; }\n ").arg(color.name()));
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
    return style;
}

QWidget* LegacySkinParser::parseKey(QDomElement node) {
    WKey* p = new WKey(m_pParent);
    setupWidget(node, p);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node);
    if (p->getComposedWidget()) {
        setupWidget(node, p->getComposedWidget(), false);
    }
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}


QString LegacySkinParser::lookupNodeGroup(QDomElement node) {
    QString group = XmlParse::selectNodeQString(node, "Group");

    // If the group is not present, then check for a Channel, since legacy skins
    // will specify the channel as either 1 or 2.
    if (group.size() == 0) {
        int channel = XmlParse::selectNodeInt(node, "Channel");
        // groupForDeck is 0-indexed
        group = PlayerManager::groupForDeck(channel - 1);
    }

    return group;
}

// static
const char* LegacySkinParser::safeChannelString(QString channelStr) {
    QMutexLocker lock(&s_safeStringMutex);
    foreach (const char *s, s_channelStrs) {
        if (channelStr == s) { // calls QString::operator==(const char*)
            return s;
        }
    }
    QByteArray qba(channelStr.toAscii());
    char *safe = new char[qba.size() + 1]; // +1 for \0
    int i = 0;
    // Copy string
    while ((safe[i] = qba[i])) ++i;
    s_channelStrs.append(safe);
    return safe;
}

void LegacySkinParser::setupPosition(QDomNode node, QWidget* pWidget) {
    if (!XmlParse::selectNode(node, "Pos").isNull()) {
        QString pos = XmlParse::selectNodeQString(node, "Pos");
        int x = pos.left(pos.indexOf(",")).toInt();
        int y = pos.mid(pos.indexOf(",")+1).toInt();
        pWidget->move(x,y);
    }
}

bool parseSizePolicy(QString* input, QSizePolicy::Policy* policy) {
    if (input->endsWith("me")) {
        *policy = QSizePolicy::MinimumExpanding;
        *input = input->left(input->size()-2);
    } else if (input->endsWith("e")) {
        *policy = QSizePolicy::Expanding;
        *input = input->left(input->size()-1);
    } else if (input->endsWith("i")) {
        *policy = QSizePolicy::Ignored;
        *input = input->left(input->size()-1);
    } else if (input->endsWith("min")) {
        *policy = QSizePolicy::Minimum;
        *input = input->left(input->size()-3);
    } else if (input->endsWith("max")) {
        *policy = QSizePolicy::Maximum;
        *input = input->left(input->size()-3);
    } else if (input->endsWith("p")) {
        *policy = QSizePolicy::Preferred;
        *input = input->left(input->size()-1);
    } else if (input->endsWith("f")) {
        *policy = QSizePolicy::Fixed;
        *input = input->left(input->size()-1);
    } else {
        return false;
    }
    return true;
}

void LegacySkinParser::setupSize(QDomNode node, QWidget* pWidget) {
    if (!XmlParse::selectNode(node, "MinimumSize").isNull()) {
        QString size = XmlParse::selectNodeQString(node, "MinimumSize");
        int comma = size.indexOf(",");
        QString xs = size.left(comma);
        QString ys = size.mid(comma+1);

        bool widthOk = false;
        int x = xs.toInt(&widthOk);

        bool heightOk = false;
        int y = ys.toInt(&heightOk);

        // -1 means do not set.
        if (widthOk && heightOk && x >= 0 && y >= 0) {
            pWidget->setMinimumSize(x, y);
        } else if (widthOk && x >= 0) {
            pWidget->setMinimumWidth(x);
        } else if (heightOk && y >= 0) {
            pWidget->setMinimumHeight(y);
        } else if (!widthOk && !heightOk) {
            qDebug() << "Could not parse widget MinimumSize:" << size;
        }
    }

    if (!XmlParse::selectNode(node, "MaximumSize").isNull()) {
        QString size = XmlParse::selectNodeQString(node, "MaximumSize");
        int comma = size.indexOf(",");
        QString xs = size.left(comma);
        QString ys = size.mid(comma+1);

        bool widthOk = false;
        int x = xs.toInt(&widthOk);

        bool heightOk = false;
        int y = ys.toInt(&heightOk);

        // -1 means do not set.
        if (widthOk && heightOk && x >= 0 && y >= 0) {
            pWidget->setMaximumSize(x, y);
        } else if (widthOk && x >= 0) {
            pWidget->setMaximumWidth(x);
        } else if (heightOk && y >= 0) {
            pWidget->setMaximumHeight(y);
        } else if (!widthOk && !heightOk) {
            qDebug() << "Could not parse widget MaximumSize:" << size;
        }
    }

    bool hasSizePolicyNode = false;
    if (!XmlParse::selectNode(node, "SizePolicy").isNull()) {
        QString size = XmlParse::selectNodeQString(node, "SizePolicy");
        int comma = size.indexOf(",");
        QString xs = size.left(comma);
        QString ys = size.mid(comma+1);

        QSizePolicy sizePolicy = pWidget->sizePolicy();

        QSizePolicy::Policy horizontalPolicy;
        if (parseSizePolicy(&xs, &horizontalPolicy)) {
            sizePolicy.setHorizontalPolicy(horizontalPolicy);
        } else {
            qDebug() << "Could not parse horizontal size policy:" << xs;
        }

        QSizePolicy::Policy verticalPolicy;
        if (parseSizePolicy(&ys, &verticalPolicy)) {
            sizePolicy.setVerticalPolicy(verticalPolicy);
        } else {
            qDebug() << "Could not parse vertical size policy:" << ys;
        }

        hasSizePolicyNode = true;
        pWidget->setSizePolicy(sizePolicy);
    }

    if (!XmlParse::selectNode(node, "Size").isNull()) {
        QString size = XmlParse::selectNodeQString(node, "Size");
        int comma = size.indexOf(",");
        QString xs = size.left(comma);
        QString ys = size.mid(comma+1);

        QSizePolicy sizePolicy = pWidget->sizePolicy();

        bool hasHorizontalPolicy = false;
        QSizePolicy::Policy horizontalPolicy;
        if (parseSizePolicy(&xs, &horizontalPolicy)) {
            sizePolicy.setHorizontalPolicy(horizontalPolicy);
            hasHorizontalPolicy = true;
        }

        bool hasVerticalPolicy = false;
        QSizePolicy::Policy verticalPolicy;
        if (parseSizePolicy(&ys, &verticalPolicy)) {
            sizePolicy.setVerticalPolicy(verticalPolicy);
            hasVerticalPolicy = true;
        }

        bool widthOk = false;
        int x = xs.toInt(&widthOk);
        if (widthOk) {
            if (hasHorizontalPolicy &&
                    sizePolicy.horizontalPolicy() == QSizePolicy::Fixed) {
                //qDebug() << "setting width fixed to" << x;
                pWidget->setFixedWidth(x);
            } else {
                //qDebug() << "setting width to" << x;
                pWidget->setMinimumWidth(x);
            }
        }

        bool heightOk = false;
        int y = ys.toInt(&heightOk);
        if (heightOk) {
            if (hasVerticalPolicy &&
                    sizePolicy.verticalPolicy() == QSizePolicy::Fixed) {
                //qDebug() << "setting height fixed to" << x;
                pWidget->setFixedHeight(y);
            } else {
                //qDebug() << "setting height to" << y;
                pWidget->setMinimumHeight(y);
            }
        }

        if (!hasSizePolicyNode) {
            pWidget->setSizePolicy(sizePolicy);
        }
    }
}

void LegacySkinParser::setupWidget(QDomNode node, QWidget* pWidget, bool setPosition) {
    // Override the widget object name.
    QString objectName = XmlParse::selectNodeQString(node, "ObjectName");
    if (!objectName.isEmpty()) {
        pWidget->setObjectName(objectName);
    }

    if (setPosition) {
        setupPosition(node, pWidget);
    }
    setupSize(node, pWidget);

    // Tooltip
    if (!XmlParse::selectNode(node, "Tooltip").isNull()) {
        QString toolTip = XmlParse::selectNodeQString(node, "Tooltip");
        pWidget->setToolTip(toolTip);
    } else if (!XmlParse::selectNode(node, "TooltipId").isNull()) {
        QString toolTipId = XmlParse::selectNodeQString(node, "TooltipId");
        QString toolTip = m_tooltips.tooltipForId(toolTipId);

        if (toolTipId.length() > 0) {
            pWidget->setToolTip(toolTip);
        } else {
            qDebug() << "Invalid <TooltipId> in skin.xml:" << toolTipId;
        }
    }

    QString style = XmlParse::selectNodeQString(node, "Style");
    // Check if we should apply legacy library styling to this node.
    if (XmlParse::selectNodeQString(node, "LegacyTableViewStyle")
        .contains("true", Qt::CaseInsensitive)) {
        style = getLibraryStyle(node);
    }
    if (style != "") {
        pWidget->setStyleSheet(style);
    }
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
        bool created = false;
        ControlObject * control = controlFromConfigKey(configKey, &created);

        QString property = XmlParse::selectNodeQString(con, "BindProperty");
        if (property != "") {
            qDebug() << "Making property binder for" << property;
            // Bind this control to a property. Not leaked because it is
            // parented to the widget and so it dies with it.
            PropertyBinder* pBinder = new PropertyBinder(
                pWidget, property, control, m_pConfig);
            // If we created this control, bind it to the PropertyBinder so that
            // it is deleted when the binder is deleted.
            if (created) {
                control->setParent(pBinder);
            }
        } else if (!XmlParse::selectNode(con, "OnOff").isNull() &&
                   XmlParse::selectNodeQString(con, "OnOff")=="true") {
            // Connect control proxy to widget. Parented to pWidget so it is not
            // leaked. OnOff controls do not use the value of the widget at all
            // so we do not give this control's info to the
            // ControllerLearningEventFilter.
            (new ControlObjectThreadWidget(control->getKey(), pWidget))->setWidgetOnOff(pWidget);
        } else {
            // Default to emit on press
            ControlObjectThreadWidget::EmitOption emitOption = ControlObjectThreadWidget::EMIT_ON_PRESS;

            // Get properties from XML, or use defaults
            if (XmlParse::selectNodeQString(con, "EmitOnPressAndRelease").contains("true", Qt::CaseInsensitive))
                emitOption = ControlObjectThreadWidget::EMIT_ON_PRESS_AND_RELEASE;

            if (XmlParse::selectNodeQString(con, "EmitOnDownPress").contains("false", Qt::CaseInsensitive))
                emitOption = ControlObjectThreadWidget::EMIT_ON_RELEASE;

            bool connectValueFromWidget = true;
            if (XmlParse::selectNodeQString(con, "ConnectValueFromWidget").contains("false", Qt::CaseInsensitive))
                connectValueFromWidget = false;

            bool connectValueToWidget = true;
            if (XmlParse::selectNodeQString(con, "ConnectValueToWidget").contains("false", Qt::CaseInsensitive))
                connectValueToWidget = false;

            Qt::MouseButton state = Qt::NoButton;
            if (!XmlParse::selectNode(con, "ButtonState").isNull()) {
                if (XmlParse::selectNodeQString(con, "ButtonState").contains("LeftButton", Qt::CaseInsensitive)) {
                    state = Qt::LeftButton;
                } else if (XmlParse::selectNodeQString(con, "ButtonState").contains("RightButton", Qt::CaseInsensitive)) {
                    state = Qt::RightButton;
                }
            }

            // Connect control proxy to widget. Parented to pWidget so it is not
            // leaked.
            (new ControlObjectThreadWidget(control->getKey(), pWidget))->setWidget(
                        pWidget, connectValueFromWidget, connectValueToWidget,
                        emitOption, state);

            // We only add info for controls that this widget affects, not
            // controls that only affect the widget.
            if (connectValueFromWidget) {
                m_pControllerManager->getControllerLearningEventFilter()
                        ->addWidgetClickInfo(pWidget, state, control, emitOption);
            }

            // Add keyboard shortcut info to tooltip string
            if (connectValueFromWidget) {
                // do not add Shortcut string for feedback connections
                QString shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(configKey);
                addShortcutToToolTip(pWidget, shortcut, QString(""));

                const WSliderComposed* pSlider;

                if (qobject_cast<const  WPushButton*>(pWidget)) {
                    // check for "_activate", "_toggle"
                    ConfigKey subkey;
                    QString shortcut;

                    subkey = configKey;
                    subkey.item += "_activate";
                    shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                    addShortcutToToolTip(pWidget, shortcut, tr("activate"));

                    subkey = configKey;
                    subkey.item += "_toggle";
                    shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                    addShortcutToToolTip(pWidget, shortcut, tr("toggle"));
                } else if ((pSlider = qobject_cast<const WSliderComposed*>(pWidget))) {
                    // check for "_up", "_down", "_up_small", "_down_small"
                    ConfigKey subkey;
                    QString shortcut;

                    if (pSlider->isHorizontal()) {
                        subkey = configKey;
                        subkey.item += "_up";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("right"));

                        subkey = configKey;
                        subkey.item += "_down";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("left"));

                        subkey = configKey;
                        subkey.item += "_up_small";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("right small"));

                        subkey = configKey;
                        subkey.item += "_down_small";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("left small"));
                    } else {
                        subkey = configKey;
                        subkey.item += "_up";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("up"));

                        subkey = configKey;
                        subkey.item += "_down";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("down"));

                        subkey = configKey;
                        subkey.item += "_up_small";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("up small"));

                        subkey = configKey;
                        subkey.item += "_down_small";
                        shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(subkey);
                        addShortcutToToolTip(pWidget, shortcut, tr("down small"));
                    }
                }
            }
        }
        con = con.nextSibling();
    }
}

void LegacySkinParser::addShortcutToToolTip(QWidget* pWidget, const QString& shortcut, const QString& cmd) {
    if (shortcut.isEmpty()) {
        return;
    }

    QString tooltip = pWidget->toolTip();

    // translate shortcut to native text
#if QT_VERSION >= 0x040700
    QString nativeShortcut = QKeySequence(shortcut, QKeySequence::PortableText).toString(QKeySequence::NativeText);
#else
    QKeySequence keySec = QKeySequence::fromString(shortcut, QKeySequence::PortableText);
    QString nativeShortcut = keySec.toString(QKeySequence::NativeText);
#endif

    tooltip += "\n";
    tooltip += tr("Shortcut");
    if (!cmd.isEmpty()) {
        tooltip += " ";
        tooltip += cmd;
    }
    tooltip += ": ";
    tooltip += nativeShortcut;
    pWidget->setToolTip(tooltip);
}
