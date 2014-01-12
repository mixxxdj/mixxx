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
#include "controlobjectslave.h"

#include "mixxxkeyboard.h"
#include "playermanager.h"
#include "basetrackplayer.h"
#include "library/library.h"
#include "xmlparse.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermanager.h"

#include "skin/colorschemeparser.h"
#include "skin/skincontext.h"

#include "widget/controlwidgetconnection.h"
#include "widget/wbasewidget.h"
#include "widget/wwidget.h"
#include "widget/wknob.h"
#include "widget/wknobcomposed.h"
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
          m_pParent(NULL),
          m_pContext(NULL) {
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
        n = m_pContext->selectNode(n, "Connection");
        if (!n.isNull())
        {
            if  (m_pContext->selectString(n, "ConfigKey").contains(key))
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
        for (unsigned int i = 0; i < attribute_nodes.length(); ++i) {
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
    m_pContext = new SkinContext();
    m_pContext->setSkinBasePath(skinPath.append("/"));
    QList<QWidget*> widgets = parseNode(skinDocument);

    if (widgets.empty()) {
        qWarning() << "Skin produced no widgets!";
        return NULL;
    } else if (widgets.size() > 1) {
        qWarning() << "Skin produced more than 1 widget!";
    }
    return widgets[0];
}

QList<QWidget*> wrapWidget(QWidget* pWidget) {
    QList<QWidget*> result;
    if (pWidget != NULL) {
        result.append(pWidget);
    }
    return result;
}

QList<QWidget*> LegacySkinParser::parseNode(QDomElement node) {
    QList<QWidget*> result;
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

        // If the root widget has a layout we are loading a "new style" skin.
        QString layout = m_pContext->selectString(node, "Layout");
        bool newStyle = !layout.isEmpty();

        qDebug() << "Skin is a" << (newStyle ? ">=1.12.0" : "<1.12.0") << "style skin.";


        if (newStyle) {
            // New style skins are just a WidgetGroup at the root.
            result.append(parseWidgetGroup(node));
        } else {
            // From here on is loading for legacy skins only.
            QWidget* pOuterWidget = new QWidget(m_pParent);
            QWidget* pInnerWidget = new QWidget(pOuterWidget);

            // <Background> is only valid for old-style skins.
            QDomElement background = m_pContext->selectElement(node, "Background");
            if (!background.isNull()) {
                parseBackground(background, pOuterWidget, pInnerWidget);
            }

            // Interpret <Size>, <SizePolicy>, <Style>, etc. tags for the root node.
            setupWidget(node, pInnerWidget, false);

            m_pParent = pInnerWidget;

            // Legacy skins do not use a <Children> block.
            QDomNodeList children = node.childNodes();
            for (int i = 0; i < children.count(); ++i) {
                QDomNode node = children.at(i);
                if (node.isElement()) {
                    parseNode(node.toElement());
                }
            }

            // Keep innerWidget centered (for fullscreen).
            pOuterWidget->setLayout(new QHBoxLayout(pOuterWidget));
            pOuterWidget->layout()->setContentsMargins(0, 0, 0, 0);
            pOuterWidget->layout()->addWidget(pInnerWidget);
            result.append(pOuterWidget);
        }
    } else if (nodeName == "SliderComposed") {
        result = wrapWidget(parseSliderComposed(node));
    } else if (nodeName == "PushButton") {
        result = wrapWidget(parsePushButton(node));
    } else if (nodeName == "Overview") {
        result = wrapWidget(parseOverview(node));
    } else if (nodeName == "Visual") {
        result = wrapWidget(parseVisual(node));
    } else if (nodeName == "Text") {
        result = wrapWidget(parseText(node));
    } else if (nodeName == "TrackProperty") {
        result = wrapWidget(parseTrackProperty(node));
    } else if (nodeName == "VuMeter") {
        result = wrapWidget(parseVuMeter(node));
    } else if (nodeName == "StatusLight") {
        result = wrapWidget(parseStatusLight(node));
    } else if (nodeName == "Display") {
        result = wrapWidget(parseDisplay(node));
    } else if (nodeName == "NumberRate") {
        result = wrapWidget(parseNumberRate(node));
    } else if (nodeName == "NumberPos") {
        result = wrapWidget(parseNumberPos(node));
    } else if (nodeName == "Number" || nodeName == "NumberBpm") {
        // NumberBpm is deprecated, and is now the same as a Number
        result = wrapWidget(parseNumber(node));
    } else if (nodeName == "Label") {
        result = wrapWidget(parseLabel(node));
    } else if (nodeName == "Knob") {
        result = wrapWidget(parseKnob(node));
    } else if (nodeName == "KnobComposed") {
        result = wrapWidget(parseKnobComposed(node));
    } else if (nodeName == "TableView") {
        result = wrapWidget(parseTableView(node));
    } else if (nodeName == "SearchBox") {
        result = wrapWidget(parseSearchBox(node));
    } else if (nodeName == "WidgetGroup") {
        result = wrapWidget(parseWidgetGroup(node));
    } else if (nodeName == "WidgetStack") {
        result = wrapWidget(parseWidgetStack(node));
    } else if (nodeName == "Spinny") {
        result = wrapWidget(parseSpinny(node));
    } else if (nodeName == "Time") {
        result = wrapWidget(parseTime(node));
    } else if (nodeName == "Splitter") {
        result = wrapWidget(parseSplitter(node));
    } else if (nodeName == "LibrarySidebar") {
        result = wrapWidget(parseLibrarySidebar(node));
    } else if (nodeName == "Library") {
        result = wrapWidget(parseLibrary(node));
    } else if (nodeName == "Key") {
        result = wrapWidget(parseKey(node));
    } else if (nodeName == "SetVariable") {
        m_pContext->updateVariable(node);
    } else if (nodeName == "Template") {
        result = parseTemplate(node);
    } else {
        qDebug() << "Invalid node name in skin:" << nodeName;
    }

    return result;
}

QWidget* LegacySkinParser::parseSplitter(QDomElement node) {
    QSplitter* pSplitter = new QSplitter(m_pParent);
    pSplitter->setObjectName("Splitter");
    setupWidget(node, pSplitter);

    // Default orientation is horizontal.
    if (m_pContext->hasNode(node, "Orientation")) {
        QString layout = m_pContext->selectString(node, "Orientation");
        if (layout == "vertical") {
            pSplitter->setOrientation(Qt::Vertical);
        } else if (layout == "horizontal") {
            pSplitter->setOrientation(Qt::Horizontal);
        }
    }

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");

    QWidget* pOldParent = m_pParent;
    m_pParent = pSplitter;

    if (!childrenNode.isNull()) {
        // Descend chilren
        QDomNodeList children = childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (node.isElement()) {
                QList<QWidget*> children = parseNode(node.toElement());
                foreach (QWidget* pChild, children) {
                    if (pChild == NULL)
                        continue;
                    pSplitter->addWidget(pChild);
                }
            }
        }
    }

    if (m_pContext->hasNode(node, "SplitSizes")) {
        QString sizesJoined = m_pContext->selectString(node, "SplitSizes");
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
    setupBaseWidget(node, pGroup);
    setupWidget(node, pGroup);
    pGroup->setup(node, *m_pContext);
    setupConnections(node, pGroup);

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");

    QWidget* pOldParent = m_pParent;
    m_pParent = pGroup;

    if (!childrenNode.isNull()) {
        // Descend children
        QDomNodeList children = childrenNode.childNodes();
        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);
            if (node.isElement()) {
                QList<QWidget*> children = parseNode(node.toElement());

                foreach (QWidget* pChild, children) {
                    if (pChild == NULL) {
                        continue;
                    }
                    pGroup->addWidget(pChild);
                }
            }
        }
    }
    m_pParent = pOldParent;
    return pGroup;
}

QWidget* LegacySkinParser::parseWidgetStack(QDomElement node) {
    ControlObject* pNextControl = NULL;
    QString nextControl = m_pContext->selectString(node, "NextControl");
    bool createdNext = false;
    if (nextControl.length() > 0) {
        ConfigKey nextConfigKey = ConfigKey::parseCommaSeparated(nextControl);
        pNextControl = controlFromConfigKey(nextConfigKey, &createdNext);
    }

    ControlObject* pPrevControl = NULL;
    bool createdPrev = false;
    QString prevControl = m_pContext->selectString(node, "PrevControl");
    if (prevControl.length() > 0) {
        ConfigKey prevConfigKey = ConfigKey::parseCommaSeparated(prevControl);
        pPrevControl = controlFromConfigKey(prevConfigKey, &createdPrev);
    }

    WWidgetStack* pStack = new WWidgetStack(m_pParent, pNextControl, pPrevControl);
    pStack->setObjectName("WidgetStack");
    pStack->setContentsMargins(0, 0, 0, 0);
    setupBaseWidget(node, pStack);
    setupWidget(node, pStack);
    setupConnections(node, pStack);

    if (createdNext && pNextControl) {
        pNextControl->setParent(pStack);
    }

    if (createdPrev && pPrevControl) {
        pPrevControl->setParent(pStack);
    }

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");

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

            QList<QWidget*> children = parseNode(element);

            if (children.empty()) {
                qWarning() << "WidgetStack child produced no widget.";
                continue;
            }

            if (children.size() > 1) {
                qWarning() << "WidgetStack child produced multiple widgets."
                           << "All but the first are ignored.";
            }
            QWidget* pChild = children[0];

            if (pChild == NULL) {
                continue;
            }

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

    QString filename = m_pContext->selectString(node, "Path");
    QPixmap* background = WPixmapStore::getPixmapNoCache(
        m_pContext->getSkinPath(filename));

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
    if (m_pContext->hasNode(node, "BgColor")) {
        c.setNamedColor(m_pContext->selectString(node, "BgColor"));
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
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseSliderComposed(QDomElement node) {
    WSliderComposed* p = new WSliderComposed(m_pParent);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
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

    setupBaseWidget(node, overviewWidget);
    setupWidget(node, overviewWidget);
    overviewWidget->setup(node, *m_pContext);
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
    factory->setWaveformWidget(viewer, node, *m_pContext);

    //qDebug() << "::parseVisual: parent" << m_pParent << m_pParent->size();
    //qDebug() << "::parseVisual: viewer" << viewer << viewer->size();

    viewer->installEventFilter(m_pKeyboard);
    viewer->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());

    // Connect control proxy to widget, so delete can be handled by the QT object tree
    ControlObjectSlave* p = new ControlObjectSlave(
            channelStr, "wheel", viewer);
    ControlWidgetConnection* pConnection = new ControlParameterWidgetConnection(
        viewer, p, true, false, ControlWidgetConnection::EMIT_ON_PRESS);
    viewer->addRightConnection(pConnection);

    setupBaseWidget(node, viewer);
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
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
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
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
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
    WVuMeter* p = new WVuMeter(m_pParent);
    WaveformWidgetFactory::instance()->addTimerListener(p);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseStatusLight(QDomElement node) {
    WStatusLight * p = new WStatusLight(m_pParent);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseDisplay(QDomElement node) {
    WDisplay * p = new WDisplay(m_pParent);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseNumberRate(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    QColor c(255,255,255);
    if (m_pContext->hasNode(node, "BgColor")) {
        c.setNamedColor(m_pContext->selectString(node, "BgColor"));
    }

    QPalette palette;
    //palette.setBrush(QPalette::Background, WSkinColor::getCorrectColor(c));
    palette.setBrush(QPalette::Button, Qt::NoBrush);

    WNumberRate * p = new WNumberRate(pSafeChannelStr, m_pParent);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
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
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseNumber(QDomElement node) {
    WNumber* p = new WNumber(m_pParent);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseLabel(QDomElement node) {
    WLabel * p = new WLabel(m_pParent);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseTime(QDomElement node) {
    WTime *p = new WTime(m_pParent);
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseKnob(QDomElement node) {
    WKnob * p = new WKnob(m_pParent);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseKnobComposed(QDomElement node) {
    WKnobComposed* p = new WKnobComposed(m_pParent);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
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
        WLabel* dummy = new WLabel(m_pParent);
        //: Shown when Spinny can not be displayd. Please keep \n unchanged
        dummy->setText(tr("No OpenGL\nsupport."));
        return dummy;
    }
    setupBaseWidget(node, spinny);
    setupWidget(node, spinny);

    WaveformWidgetFactory::instance()->addTimerListener(spinny);
    connect(spinny, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    spinny->setup(node, *m_pContext, pSafeChannelStr);
    setupConnections(node, spinny);
    spinny->installEventFilter(m_pKeyboard);
    spinny->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return spinny;
}

QWidget* LegacySkinParser::parseSearchBox(QDomElement node) {
    WSearchLineEdit* pLineEditSearch = new WSearchLineEdit(m_pConfig, m_pParent);
    setupBaseWidget(node, pLineEditSearch);
    setupWidget(node, pLineEditSearch);
    pLineEditSearch->setup(node, *m_pContext);

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
    setupBaseWidget(node, pLibraryWidget);
    setupWidget(node, pLibraryWidget);

    return pLibraryWidget;
}

QWidget* LegacySkinParser::parseLibrarySidebar(QDomElement node) {
    WLibrarySidebar* pLibrarySidebar = new WLibrarySidebar(m_pParent);
    pLibrarySidebar->installEventFilter(m_pKeyboard);
    pLibrarySidebar->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    m_pLibrary->bindSidebarWidget(pLibrarySidebar);
    setupBaseWidget(node, pLibrarySidebar);
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
    QString style = getStyleFromNode(node);

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

    if (m_pContext->hasNode(node, "FgColor")) {
        color.setNamedColor(m_pContext->selectString(node, "FgColor"));
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

    if (m_pContext->hasNode(node, "BgColor")) {
        color.setNamedColor(m_pContext->selectString(node, "BgColor"));
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

    if (m_pContext->hasNode(node, "BgColorRowEven")) {
        color.setNamedColor(m_pContext->selectString(node, "BgColorRowEven"));
        color = WSkinColor::getCorrectColor(color);

        if (hasQtKickedUsInTheNuts) {
            styleHack.append(QString("QTableView::item:!selected { background-color: %1; }\n ").arg(color.name()));
        } else {
            styleHack.append(QString("WLibraryTableView { background: %1; }\n ").arg(color.name()));
        }
    }

    if (m_pContext->hasNode(node, "BgColorRowUneven")) {
        color.setNamedColor(m_pContext->selectString(node, "BgColorRowUneven"));
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
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WNumber/WLabel both change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    p->setup(node, *m_pContext);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QDomElement LegacySkinParser::loadTemplate(const QString& path) {
    QFileInfo templateFileInfo(path);

    QString absolutePath = templateFileInfo.absoluteFilePath();

    QHash<QString, QDomElement>::const_iterator it =
            m_templateCache.find(absolutePath);
    if (it != m_templateCache.end()) {
        return it.value();
    }

    QFile templateFile(absolutePath);

    if (!templateFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open template file:" << absolutePath;
    }

    QDomDocument tmpl("template");
    QString errorMessage;
    int errorLine;
    int errorColumn;

    if (!tmpl.setContent(&templateFile, &errorMessage,
                         &errorLine, &errorColumn)) {
        qDebug() << "LegacySkinParser::loadTemplate - setContent failed see"
                 << "line:" << errorLine << "column:" << errorColumn;
        qDebug() << "LegacySkinParser::loadTemplate - message:" << errorMessage;
        return QDomElement();
    }

    m_templateCache[absolutePath] = tmpl.documentElement();
    return tmpl.documentElement();
}

QList<QWidget*> LegacySkinParser::parseTemplate(QDomElement node) {
    if (!node.hasAttribute("src")) {
        qDebug() << "Template instantiation without src attribute:" << node.text();
        return QList<QWidget*>();
    }

    QString path = node.attribute("src");

    QDomElement templateNode = loadTemplate(path);

    if (templateNode.isNull()) {
        qDebug() << "Template instantiation for template failed:"
                 << path;
        return QList<QWidget*>();
    }

    SkinContext* pOldContext = m_pContext;
    m_pContext = new SkinContext(*pOldContext);
    // Take any <SetVariable> elements from this node and update the context
    // with them.
    m_pContext->updateVariables(node);

    QList<QWidget*> widgets;

    QDomNode child = templateNode.firstChild();
    while (!child.isNull()) {
        if (child.isElement()) {
            QList<QWidget*> childWidgets = parseNode(child.toElement());
            widgets.append(childWidgets);
        }
        child = child.nextSibling();
    }

    delete m_pContext;
    m_pContext = pOldContext;
    return widgets;
}

QString LegacySkinParser::lookupNodeGroup(QDomElement node) {
    QString group = m_pContext->selectString(node, "Group");

    // If the group is not present, then check for a Channel, since legacy skins
    // will specify the channel as either 1 or 2.
    if (group.size() == 0) {
        int channel = m_pContext->selectInt(node, "Channel");
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
    if (m_pContext->hasNode(node, "Pos")) {
        QString pos = m_pContext->selectString(node, "Pos");
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
    if (m_pContext->hasNode(node, "MinimumSize")) {
        QString size = m_pContext->selectString(node, "MinimumSize");
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

    if (m_pContext->hasNode(node, "MaximumSize")) {
        QString size = m_pContext->selectString(node, "MaximumSize");
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
    if (m_pContext->hasNode(node, "SizePolicy")) {
        QString size = m_pContext->selectString(node, "SizePolicy");
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

    if (m_pContext->hasNode(node, "Size")) {
        QString size = m_pContext->selectString(node, "Size");
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

QString LegacySkinParser::getStyleFromNode(QDomNode node) {
    QDomElement styleElement = m_pContext->selectElement(node, "Style");

    if (styleElement.isNull()) {
        return QString();
    }

    QString style;
    if (styleElement.hasAttribute("src")) {
        QString styleSrc = styleElement.attribute("src");

        QFile file(styleSrc);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileBytes = file.readAll();

            style = QString::fromLocal8Bit(fileBytes.constData(),
                                           fileBytes.length());
        }
    } else {
        // If no src attribute, use the node data as text.
        style = styleElement.text();
    }

    // Legacy fixes: In Mixxx <1.12.0 we used QGroupBox for WWidgetGroup. Some
    // skin writers used QGroupBox for styling. In 1.12.0 onwards, we have
    // switched to QFrame and there should be no reason we would ever use a
    // QGroupBox in a skin. To support legacy skins, we rewrite QGroupBox
    // selectors to use WWidgetGroup directly.
    style = style.replace("QGroupBox", "WWidgetGroup");

    return style;
}

void LegacySkinParser::setupBaseWidget(QDomNode node,
                                       WBaseWidget* pBaseWidget) {
    // Tooltip
    if (m_pContext->hasNode(node, "Tooltip")) {
        QString toolTip = m_pContext->selectString(node, "Tooltip");
        pBaseWidget->setBaseTooltip(toolTip);
    } else if (m_pContext->hasNode(node, "TooltipId")) {
        QString toolTipId = m_pContext->selectString(node, "TooltipId");
        QString toolTip = m_tooltips.tooltipForId(toolTipId);

        if (toolTipId.length() > 0) {
            pBaseWidget->setBaseTooltip(toolTip);
        } else {
            qDebug() << "Invalid <TooltipId> in skin.xml:" << toolTipId;
        }
    }
}

void LegacySkinParser::setupWidget(QDomNode node,
                                   QWidget* pWidget,
                                   bool setPosition) {
    // Override the widget object name.
    QString objectName = m_pContext->selectString(node, "ObjectName");
    if (!objectName.isEmpty()) {
        pWidget->setObjectName(objectName);
    }

    if (setPosition) {
        setupPosition(node, pWidget);
    }
    setupSize(node, pWidget);

    QString style = getStyleFromNode(node);
    // Check if we should apply legacy library styling to this node.
    if (m_pContext->selectBool(node, "LegacyTableViewStyle", false)) {
        style = getLibraryStyle(node);
    }
    if (style != "") {
        pWidget->setStyleSheet(style);
    }
}

void LegacySkinParser::setupConnections(QDomNode node, WBaseWidget* pWidget) {
    // For each connection
    QDomNode con = m_pContext->selectNode(node, "Connection");

    while (!con.isNull())
    {
        // Get ConfigKey
        QString key = m_pContext->selectString(con, "ConfigKey");

        ConfigKey configKey = ConfigKey::parseCommaSeparated(key);

        // Check that the control exists
        bool created = false;
        ControlObject* control = controlFromConfigKey(configKey, &created);

        if (m_pContext->hasNode(con, "BindProperty")) {
            QString property = m_pContext->selectString(con, "BindProperty");
            qDebug() << "Making property connection for" << property;

            ControlObjectSlave* pControlWidget =
                    new ControlObjectSlave(control->getKey(),
                                           pWidget->toQWidget());

            ControlWidgetConnection* pConnection =
                    new ControlWidgetPropertyConnection(pWidget, pControlWidget,
                                                        m_pConfig, property);
            pWidget->addConnection(pConnection);

            // If we created this control, bind it to the
            // ControlWidgetConnection so that it is deleted when the connection
            // is deleted.
            if (created) {
                control->setParent(pConnection);
            }
        } else {
            // Default to emit on press
            ControlWidgetConnection::EmitOption emitOption =
                    ControlWidgetConnection::EMIT_ON_PRESS;

            // Get properties from XML, or use defaults
            if (m_pContext->selectBool(con, "EmitOnPressAndRelease", false))
                emitOption = ControlWidgetConnection::EMIT_ON_PRESS_AND_RELEASE;

            if (!m_pContext->selectBool(con, "EmitOnDownPress", true))
                emitOption = ControlWidgetConnection::EMIT_ON_RELEASE;

            bool connectValueFromWidget = m_pContext->selectBool(con, "ConnectValueFromWidget", true);
            bool connectValueToWidget = m_pContext->selectBool(con, "ConnectValueToWidget", true);

            Qt::MouseButton state = Qt::NoButton;
            if (m_pContext->hasNode(con, "ButtonState")) {
                if (m_pContext->selectString(con, "ButtonState").contains("LeftButton", Qt::CaseInsensitive)) {
                    state = Qt::LeftButton;
                } else if (m_pContext->selectString(con, "ButtonState").contains("RightButton", Qt::CaseInsensitive)) {
                    state = Qt::RightButton;
                }
            }

            // Connect control proxy to widget. Parented to pWidget so it is not
            // leaked.
            ControlObjectSlave* pControlWidget = new ControlObjectSlave(
                control->getKey(), pWidget->toQWidget());
            ControlWidgetConnection* pConnection = new ControlParameterWidgetConnection(
                pWidget, pControlWidget, connectValueFromWidget,
                connectValueToWidget, emitOption);

            // If we created this control, bind it to the
            // ControlWidgetConnection so that it is deleted when the connection
            // is deleted.
            if (created) {
                control->setParent(pConnection);
            }

            switch (state) {
                case Qt::NoButton:
                    pWidget->addConnection(pConnection);
                    break;
                case Qt::LeftButton:
                    pWidget->addLeftConnection(pConnection);
                    break;
                case Qt::RightButton:
                    pWidget->addRightConnection(pConnection);
                    break;
                default:
                    break;
            }

            // Legacy behavior, the last widget that is marked
            // connectValueToWidget is the display connection.
            if (connectValueToWidget) {
                pWidget->setDisplayConnection(pConnection);
            }

            // We only add info for controls that this widget affects, not
            // controls that only affect the widget.
            if (connectValueFromWidget) {
                m_pControllerManager->getControllerLearningEventFilter()
                        ->addWidgetClickInfo(pWidget->toQWidget(), state, control, emitOption);
            }

            // Add keyboard shortcut info to tooltip string
            if (connectValueFromWidget) {
                // do not add Shortcut string for feedback connections
                QString shortcut = m_pKeyboard->getKeyboardConfig()->getValueString(configKey);
                addShortcutToToolTip(pWidget, shortcut, QString(""));

                const WSliderComposed* pSlider;

                if (qobject_cast<const  WPushButton*>(pWidget->toQWidget())) {
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
                } else if ((pSlider = qobject_cast<const WSliderComposed*>(pWidget->toQWidget()))) {
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

void LegacySkinParser::addShortcutToToolTip(WBaseWidget* pWidget, const QString& shortcut, const QString& cmd) {
    if (shortcut.isEmpty()) {
        return;
    }

    QString tooltip = pWidget->baseTooltip();

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
    pWidget->setBaseTooltip(tooltip);
}
