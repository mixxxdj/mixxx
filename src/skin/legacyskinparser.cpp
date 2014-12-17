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

#include "effects/effectsmanager.h"

#include "widget/controlwidgetconnection.h"
#include "widget/wbasewidget.h"
#include "widget/wcoverart.h"
#include "widget/wwidget.h"
#include "widget/wknob.h"
#include "widget/wknobcomposed.h"
#include "widget/wslidercomposed.h"
#include "widget/wpushbutton.h"
#include "widget/weffectpushbutton.h"
#include "widget/wdisplay.h"
#include "widget/wvumeter.h"
#include "widget/wstatuslight.h"
#include "widget/wlabel.h"
#include "widget/wtime.h"
#include "widget/wtracktext.h"
#include "widget/wtrackproperty.h"
#include "widget/wstarrating.h"
#include "widget/wnumber.h"
#include "widget/wnumberdb.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberrate.h"
#include "widget/weffectchain.h"
#include "widget/weffect.h"
#include "widget/weffectparameter.h"
#include "widget/weffectbuttonparameter.h"
#include "widget/weffectparameterbase.h"
#include "widget/woverviewlmh.h"
#include "widget/woverviewhsv.h"
#include "widget/woverviewrgb.h"
#include "widget/wspinny.h"
#include "widget/wwaveformviewer.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wsearchlineedit.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wskincolor.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidgetstack.h"
#include "widget/wsizeawarestack.h"
#include "widget/wwidgetgroup.h"
#include "widget/wkey.h"
#include "widget/wcombobox.h"
#include "widget/wsplitter.h"
#include "util/valuetransformer.h"
#include "util/cmdlineargs.h"

using mixxx::skin::SkinManifest;

QList<const char*> LegacySkinParser::s_channelStrs;
QMutex LegacySkinParser::s_safeStringMutex;

static bool sDebug = false;

ControlObject* controlFromConfigKey(ConfigKey key, bool bPersist,
                                    bool* created) {
    ControlObject* pControl = ControlObject::getControl(key);

    if (pControl) {
        if (created) {
            *created = false;
        }
        return pControl;
    }

    // TODO(rryan): Make this configurable by the skin.
    qWarning() << "Requested control does not exist:"
               << QString("%1,%2").arg(key.group, key.item)
               << "Creating it.";
    // Since the usual behavior here is to create a skin-defined push
    // button, actually make it a push button and set it to toggle.
    ControlPushButton* controlButton = new ControlPushButton(key, bPersist);
    controlButton->setButtonMode(ControlPushButton::TOGGLE);
    if (created) {
        *created = true;
    }
    return controlButton;
}

ControlObject* LegacySkinParser::controlFromConfigNode(QDomElement element,
                                                       const QString& nodeName,
                                                       bool* created) {
    if (element.isNull() || !m_pContext->hasNode(element, nodeName)) {
        return NULL;
    }

    QDomElement keyElement = m_pContext->selectElement(element, nodeName);
    QString name = m_pContext->nodeToString(keyElement);
    ConfigKey key = ConfigKey::parseCommaSeparated(name);

    bool bPersist = m_pContext->selectAttributeBool(keyElement, "persist", false);

    return controlFromConfigKey(key, bPersist, created);
}

LegacySkinParser::LegacySkinParser(ConfigObject<ConfigValue>* pConfig,
                                   MixxxKeyboard* pKeyboard,
                                   PlayerManager* pPlayerManager,
                                   ControllerManager* pControllerManager,
                                   Library* pLibrary,
                                   VinylControlManager* pVCMan,
                                   EffectsManager* pEffectsManager)
        : m_pConfig(pConfig),
          m_pKeyboard(pKeyboard),
          m_pPlayerManager(pPlayerManager),
          m_pControllerManager(pControllerManager),
          m_pLibrary(pLibrary),
          m_pVCManager(pVCMan),
          m_pEffectsManager(pEffectsManager),
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

// static
Qt::MouseButton LegacySkinParser::parseButtonState(QDomNode node,
                                                   const SkinContext& context) {
    if (context.hasNode(node, "ButtonState")) {
        if (context.selectString(node, "ButtonState")
                .contains("LeftButton", Qt::CaseInsensitive)) {
            return Qt::LeftButton;
        } else if (context.selectString(node, "ButtonState")
                       .contains("RightButton", Qt::CaseInsensitive)) {
            return Qt::RightButton;
        }
    }
    return Qt::NoButton;
}

QWidget* LegacySkinParser::parseSkin(QString skinPath, QWidget* pParent) {
    qDebug() << "LegacySkinParser loading skin:" << skinPath;

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
            ControlObject::set(configKey, value);
        }
    }

    ColorSchemeParser::setupLegacyColorSchemes(skinDocument, m_pConfig);

    QStringList skinPaths(skinPath);
    QDir::setSearchPaths("skin", skinPaths);

    // don't parent till here so the first opengl waveform doesn't screw
    // up --bkgood
    // I'm disregarding this return value because I want to return the
    // created parent so MixxxMainWindow can use it for nefarious purposes (
    // fullscreen mostly) --bkgood
    m_pParent = pParent;
    m_pContext = new SkinContext(m_pConfig, skinPath + "/skin.xml");
    m_pContext->setSkinBasePath(skinPath.append("/"));
    QList<QWidget*> widgets = parseNode(skinDocument);

    if (widgets.empty()) {
        SKIN_WARNING(skinDocument, *m_pContext) << "Skin produced no widgets!";
        return NULL;
    } else if (widgets.size() > 1) {
        SKIN_WARNING(skinDocument, *m_pContext) << "Skin produced more than 1 widget!";
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

    if (sDebug) {
        qDebug() << "BEGIN PARSE NODE" << nodeName;
    }
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
        result = wrapWidget(parseStandardWidget<WSliderComposed>(node));
    } else if (nodeName == "PushButton") {
        result = wrapWidget(parseStandardWidget<WPushButton>(node));
    } else if (nodeName == "EffectPushButton") {
        result = wrapWidget(parseEffectPushButton(node));
    } else if (nodeName == "ComboBox") {
        result = wrapWidget(parseStandardWidget<WComboBox>(node));
    } else if (nodeName == "Overview") {
        result = wrapWidget(parseOverview(node));
    } else if (nodeName == "Visual") {
        result = wrapWidget(parseVisual(node));
    } else if (nodeName == "Text") {
        result = wrapWidget(parseText(node));
    } else if (nodeName == "TrackProperty") {
        result = wrapWidget(parseTrackProperty(node));
    } else if (nodeName == "StarRating") {
        result = wrapWidget(parseStarRating(node));
    } else if (nodeName == "VuMeter") {
        result = wrapWidget(parseStandardWidget<WVuMeter>(node, true));
    } else if (nodeName == "StatusLight") {
        result = wrapWidget(parseStandardWidget<WStatusLight>(node));
    } else if (nodeName == "Display") {
        result = wrapWidget(parseStandardWidget<WDisplay>(node));
    } else if (nodeName == "NumberRate") {
        result = wrapWidget(parseNumberRate(node));
    } else if (nodeName == "NumberPos") {
        result = wrapWidget(parseNumberPos(node));
    } else if (nodeName == "Number" || nodeName == "NumberBpm") {
        // NumberBpm is deprecated, and is now the same as a Number
        result = wrapWidget(parseLabelWidget<WNumber>(node));
    } else if (nodeName == "NumberDb") {
        result = wrapWidget(parseLabelWidget<WNumberDb>(node));
    } else if (nodeName == "Label") {
        result = wrapWidget(parseLabelWidget<WLabel>(node));
    } else if (nodeName == "Knob") {
        result = wrapWidget(parseStandardWidget<WKnob>(node));
    } else if (nodeName == "KnobComposed") {
        result = wrapWidget(parseStandardWidget<WKnobComposed>(node));
    } else if (nodeName == "TableView") {
        result = wrapWidget(parseTableView(node));
    } else if (nodeName == "CoverArt") {
        result = wrapWidget(parseCoverArt(node));
    } else if (nodeName == "SearchBox") {
        result = wrapWidget(parseSearchBox(node));
    } else if (nodeName == "WidgetGroup") {
        result = wrapWidget(parseWidgetGroup(node));
    } else if (nodeName == "WidgetStack") {
        result = wrapWidget(parseWidgetStack(node));
    } else if (nodeName == "SizeAwareStack") {
        result = wrapWidget(parseSizeAwareStack(node));
    } else if (nodeName == "EffectChainName") {
        result = wrapWidget(parseEffectChainName(node));
    } else if (nodeName == "EffectName") {
        result = wrapWidget(parseEffectName(node));
    } else if (nodeName == "EffectParameterName") {
        result = wrapWidget(parseEffectParameterName(node));
    } else if (nodeName == "EffectButtonParameterName") {
        result = wrapWidget(parseEffectButtonParameterName(node));
    } else if (nodeName == "Spinny") {
        result = wrapWidget(parseSpinny(node));
    } else if (nodeName == "Time") {
        result = wrapWidget(parseLabelWidget<WTime>(node));
    } else if (nodeName == "Splitter") {
        result = wrapWidget(parseSplitter(node));
    } else if (nodeName == "LibrarySidebar") {
        result = wrapWidget(parseLibrarySidebar(node));
    } else if (nodeName == "Library") {
        result = wrapWidget(parseLibrary(node));
    } else if (nodeName == "Key") {
        result = wrapWidget(parseEngineKey(node));
    } else if (nodeName == "SetVariable") {
        m_pContext->updateVariable(node);
    } else if (nodeName == "Template") {
        result = parseTemplate(node);
    } else {
        SKIN_WARNING(node, *m_pContext) << "Invalid node name in skin:"
                                       << nodeName;
    }

    if (sDebug) {
        qDebug() << "END PARSE NODE" << nodeName;
    }
    return result;
}

QWidget* LegacySkinParser::parseSplitter(QDomElement node) {
    WSplitter* pSplitter = new WSplitter(m_pParent, m_pConfig);
    commonWidgetSetup(node, pSplitter);

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
    pSplitter->setup(node, *m_pContext);
    pSplitter->Init();

    m_pParent = pOldParent;
    return pSplitter;
}

QWidget* LegacySkinParser::parseWidgetGroup(QDomElement node) {
    WWidgetGroup* pGroup = new WWidgetGroup(m_pParent);
    commonWidgetSetup(node, pGroup);
    pGroup->setup(node, *m_pContext);
    pGroup->Init();

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
    bool createdNext = false;
    ControlObject* pNextControl = controlFromConfigNode(
            node.toElement(), "NextControl", &createdNext);

    bool createdPrev = false;
    ControlObject* pPrevControl = controlFromConfigNode(
            node.toElement(), "PrevControl", &createdPrev);

    bool createdCurrentPage = false;
    ControlObject* pCurrentPageControl = NULL;
    QString currentpage_co = node.attribute("currentpage");
    if (currentpage_co.length() > 0) {
        ConfigKey configKey = ConfigKey::parseCommaSeparated(currentpage_co);
        QString persist_co = node.attribute("persist");
        bool persist = m_pContext->selectAttributeBool(node, "persist", false);
        pCurrentPageControl = controlFromConfigKey(configKey, persist,
                                                   &createdCurrentPage);
    }

    WWidgetStack* pStack = new WWidgetStack(m_pParent, pNextControl,
                                            pPrevControl, pCurrentPageControl);
    pStack->setObjectName("WidgetStack");
    pStack->setContentsMargins(0, 0, 0, 0);
    commonWidgetSetup(node, pStack);

    if (createdNext && pNextControl) {
        pNextControl->setParent(pStack);
    }

    if (createdPrev && pPrevControl) {
        pPrevControl->setParent(pStack);
    }

    if (createdCurrentPage) {
        pCurrentPageControl->setParent(pStack);
    }

    QWidget* pOldParent = m_pParent;
    m_pParent = pStack;

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");
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
                SKIN_WARNING(node, *m_pContext)
                        << "WidgetStack child produced no widget.";
                continue;
            }

            if (children.size() > 1) {
                SKIN_WARNING(node, *m_pContext)
                        << "WidgetStack child produced multiple widgets."
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
                pControl = controlFromConfigKey(configKey, false, &created);
                if (created) {
                    // If we created the control, parent it to the child widget so
                    // it doesn't leak.
                    pControl->setParent(pChild);
                }
            }
            pStack->addWidgetWithControl(pChild, pControl);
        }
    }

    // Init the widget last now that all the children have been created,
    // so if the current page was saved we can switch to the correct page.
    pStack->Init();
    m_pParent = pOldParent;
    return pStack;
}

QWidget* LegacySkinParser::parseSizeAwareStack(QDomElement node) {
    WSizeAwareStack* pStack = new WSizeAwareStack(m_pParent);
    pStack->setObjectName("SizeAwareStack");
    pStack->setContentsMargins(0, 0, 0, 0);
    commonWidgetSetup(node, pStack);

    QWidget* pOldParent = m_pParent;
    m_pParent = pStack;

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");
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
                SKIN_WARNING(node, *m_pContext)
                        << "SizeAwareStack child produced no widget.";
                continue;
            }

            if (children.size() > 1) {
                SKIN_WARNING(node, *m_pContext)
                        << "SizeAwareStack child produced multiple widgets."
                        << "All but the first are ignored.";
            }
            QWidget* pChild = children[0];

            if (pChild == NULL) {
                continue;
            }

            pStack->addWidget(pChild);
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

template <class T>
QWidget* LegacySkinParser::parseStandardWidget(QDomElement element,
                                               bool timerListener) {
    T* pWidget = new T(m_pParent);
    if (timerListener) {
        WaveformWidgetFactory::instance()->addTimerListener(pWidget);
    }
    commonWidgetSetup(element, pWidget);
    pWidget->setup(element, *m_pContext);
    pWidget->installEventFilter(m_pKeyboard);
    pWidget->installEventFilter(
            m_pControllerManager->getControllerLearningEventFilter());
    pWidget->Init();
    return pWidget;
}

template <class T>
QWidget* LegacySkinParser::parseLabelWidget(QDomElement element) {
    T* pLabel = new T(m_pParent);
    setupLabelWidget(element, pLabel);
    return pLabel;
}

void LegacySkinParser::setupLabelWidget(QDomElement element, WLabel* pLabel) {
    // NOTE(rryan): To support color schemes, the WWidget::setup() call must
    // come first. This is because WLabel derivatives change the palette based
    // on the node and setupWidget() will set the widget style. If the style is
    // set before the palette is set then the custom palette will not take
    // effect which breaks color scheme support.
    pLabel->setup(element, *m_pContext);
    commonWidgetSetup(element, pLabel);
    pLabel->installEventFilter(m_pKeyboard);
    pLabel->installEventFilter(
            m_pControllerManager->getControllerLearningEventFilter());
    pLabel->Init();
}

QWidget* LegacySkinParser::parseOverview(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (pPlayer == NULL)
        return NULL;

    WOverview* overviewWidget = NULL;

    // "RGB" = "2", "HSV" = "1" or "Filtered" = "0" (LMH) waveform overview type
    int type = m_pConfig->getValueString(ConfigKey("[Waveform]","WaveformOverviewType"), "0").toInt();
    if (type == 0) {
        overviewWidget = new WOverviewLMH(pSafeChannelStr, m_pConfig, m_pParent);
    } else if (type == 1) {
        overviewWidget = new WOverviewHSV(pSafeChannelStr, m_pConfig, m_pParent);
    } else {
        overviewWidget = new WOverviewRGB(pSafeChannelStr, m_pConfig, m_pParent);
    }

    connect(overviewWidget, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    commonWidgetSetup(node, overviewWidget);
    overviewWidget->setup(node, *m_pContext);
    overviewWidget->installEventFilter(m_pKeyboard);
    overviewWidget->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    overviewWidget->Init();

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
    commonWidgetSetup(node, viewer);
    viewer->Init();

    // connect display with loading/unloading of tracks
    QObject::connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                     viewer, SLOT(onTrackLoaded(TrackPointer)));
    QObject::connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
                     viewer, SLOT(onTrackUnloaded(TrackPointer)));

    connect(viewer, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    // if any already loaded
    viewer->onTrackLoaded(pPlayer->getLoadedTrack());

    return viewer;
}

QWidget* LegacySkinParser::parseText(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    const char* pSafeChannelStr = safeChannelString(channelStr);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WTrackText* p = new WTrackText(pSafeChannelStr, m_pConfig, m_pParent);
    setupLabelWidget(node, p);

    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            p, SLOT(slotTrackLoaded(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            p, SLOT(slotTrackUnloaded(TrackPointer)));
    connect(p, SIGNAL(trackDropped(QString,QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString,QString)));

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        p->slotTrackLoaded(pTrack);
    }

    return p;
}

QWidget* LegacySkinParser::parseTrackProperty(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    const char* pSafeChannelStr = safeChannelString(channelStr);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WTrackProperty* p = new WTrackProperty(pSafeChannelStr, m_pConfig, m_pParent);
    setupLabelWidget(node, p);

    connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
            p, SLOT(slotTrackLoaded(TrackPointer)));
    connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
            p, SLOT(slotTrackUnloaded(TrackPointer)));
    connect(p, SIGNAL(trackDropped(QString,QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString,QString)));

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        p->slotTrackLoaded(pTrack);
    }

    return p;
}

QWidget* LegacySkinParser::parseStarRating(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    const char* pSafeChannelStr = safeChannelString(channelStr);

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);

    if (!pPlayer)
        return NULL;

    WStarRating* p = new WStarRating(pSafeChannelStr, m_pParent);
    p->setup(node, *m_pContext);

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
    setupLabelWidget(node, p);

    // TODO(rryan): Let's look at removing this palette change in 1.12.0. I
    // don't think it's needed anymore.
    p->setPalette(palette);

    return p;
}

QWidget* LegacySkinParser::parseNumberPos(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);

    const char* pSafeChannelStr = safeChannelString(channelStr);

    WNumberPos* p = new WNumberPos(pSafeChannelStr, m_pParent);
    setupLabelWidget(node, p);
    return p;
}

QWidget* LegacySkinParser::parseEngineKey(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    const char* pSafeChannelStr = safeChannelString(channelStr);
    WKey* pEngineKey = new WKey(pSafeChannelStr, m_pParent);
    setupLabelWidget(node, pEngineKey);
    return pEngineKey;
}

QWidget* LegacySkinParser::parseSpinny(QDomElement node) {
    QString channelStr = lookupNodeGroup(node);
    if (CmdlineArgs::Instance().getSafeMode()) {
        WLabel* dummy = new WLabel(m_pParent);
        //: Shown when Mixxx is running in safe mode.
        dummy->setText(tr("Safe Mode Enabled"));
        return dummy;
    }
    WSpinny* spinny = new WSpinny(m_pParent, channelStr, m_pConfig,
                                  m_pVCManager);
    if (!spinny->isValid()) {
        delete spinny;
        WLabel* dummy = new WLabel(m_pParent);
        //: Shown when Spinny can not be displayed. Please keep \n unchanged
        dummy->setText(tr("No OpenGL\nsupport."));
        return dummy;
    }
    commonWidgetSetup(node, spinny);

    WaveformWidgetFactory::instance()->addTimerListener(spinny);
    connect(spinny, SIGNAL(trackDropped(QString, QString)),
            m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channelStr);
    if (pPlayer != NULL) {
        connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                spinny, SLOT(slotLoadTrack(TrackPointer)));
        connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
                spinny, SLOT(slotReset()));
        // just in case a track is already loaded
        spinny->slotLoadTrack(pPlayer->getLoadedTrack());
    }

    spinny->setup(node, *m_pContext);
    spinny->installEventFilter(m_pKeyboard);
    spinny->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    spinny->Init();
    return spinny;
}

QWidget* LegacySkinParser::parseSearchBox(QDomElement node) {
    WSearchLineEdit* pLineEditSearch = new WSearchLineEdit(m_pParent);
    commonWidgetSetup(node, pLineEditSearch, false);
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

QWidget* LegacySkinParser::parseCoverArt(QDomElement node) {
    QString channel = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(channel);

    WCoverArt* pCoverArt = new WCoverArt(m_pParent, m_pConfig, channel);
    commonWidgetSetup(node, pCoverArt);
    pCoverArt->setup(node, *m_pContext);

    // If no group was provided, hook the widget up to the Library.
    if (channel.isEmpty()) {
        // Connect cover art signals to the library
        connect(m_pLibrary, SIGNAL(switchToView(const QString&)),
                pCoverArt, SLOT(slotReset()));
        connect(m_pLibrary, SIGNAL(enableCoverArtDisplay(bool)),
                pCoverArt, SLOT(slotEnable(bool)));
        connect(m_pLibrary, SIGNAL(trackSelected(TrackPointer)),
                pCoverArt, SLOT(slotLoadTrack(TrackPointer)));
    } else if (pPlayer != NULL) {
        connect(pPlayer, SIGNAL(newTrackLoaded(TrackPointer)),
                pCoverArt, SLOT(slotLoadTrack(TrackPointer)));
        connect(pPlayer, SIGNAL(unloadingTrack(TrackPointer)),
                pCoverArt, SLOT(slotReset()));
        connect(pCoverArt, SIGNAL(trackDropped(QString, QString)),
                m_pPlayerManager, SLOT(slotLoadToPlayer(QString, QString)));

        // just in case a track is already loaded
        pCoverArt->slotLoadTrack(pPlayer->getLoadedTrack());
    }

    return pCoverArt;
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
    commonWidgetSetup(node, pLibraryWidget, false);

    return pLibraryWidget;
}

QWidget* LegacySkinParser::parseLibrarySidebar(QDomElement node) {
    WLibrarySidebar* pLibrarySidebar = new WLibrarySidebar(m_pParent);
    pLibrarySidebar->installEventFilter(m_pKeyboard);
    pLibrarySidebar->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    m_pLibrary->bindSidebarWidget(pLibrarySidebar);
    commonWidgetSetup(node, pLibrarySidebar, false);
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

    QWidget* pCoverArt = parseCoverArt(node);

    QVBoxLayout* vl = new QVBoxLayout(pLibrarySidebarPage);
    vl->setContentsMargins(0,0,0,0); //Fill entire space
    vl->addWidget(pLineEditSearch);
    vl->addWidget(pLibrarySidebar);
    vl->addWidget(pCoverArt);
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

    // Style the library preview button with a default image.
    QString styleHack = (
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
        qWarning() << "LegacySkinParser::loadTemplate - setContent failed see"
                   << absolutePath << "line:" << errorLine << "column:" << errorColumn;
        qWarning() << "LegacySkinParser::loadTemplate - message:" << errorMessage;
        return QDomElement();
    }

    m_templateCache[absolutePath] = tmpl.documentElement();
    return tmpl.documentElement();
}

QList<QWidget*> LegacySkinParser::parseTemplate(QDomElement node) {
    if (!node.hasAttribute("src")) {
        SKIN_WARNING(node, *m_pContext)
                << "Template instantiation without src attribute:"
                << node.text();
        return QList<QWidget*>();
    }

    QString path = node.attribute("src");

    QDomElement templateNode = loadTemplate(path);

    if (templateNode.isNull()) {
        SKIN_WARNING(node, *m_pContext) << "Template instantiation for template failed:" << path;
        return QList<QWidget*>();
    }

    if (sDebug) {
        qDebug() << "BEGIN TEMPLATE" << path;
    }

    SkinContext* pOldContext = m_pContext;
    m_pContext = new SkinContext(*pOldContext);
    // Take any <SetVariable> elements from this node and update the context
    // with them.
    m_pContext->updateVariables(node);
    m_pContext->setXmlPath(path);

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

    if (sDebug) {
        qDebug() << "END TEMPLATE" << path;
    }
    return widgets;
}

QString LegacySkinParser::lookupNodeGroup(QDomElement node) {
    QString group = m_pContext->selectString(node, "Group");

    // If the group is not present, then check for a Channel, since legacy skins
    // will specify the channel as either 1 or 2.
    if (group.size() == 0) {
        int channel = m_pContext->selectInt(node, "Channel");
        if (channel > 0) {
            // groupForDeck is 0-indexed
            group = PlayerManager::groupForDeck(channel - 1);
        }
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

QWidget* LegacySkinParser::parseEffectChainName(QDomElement node) {
    WEffectChain* pEffectChain = new WEffectChain(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffectChain);
    return pEffectChain;
}

QWidget* LegacySkinParser::parseEffectName(QDomElement node) {
    WEffect* pEffect = new WEffect(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffect);
    return pEffect;
}

QWidget* LegacySkinParser::parseEffectPushButton(QDomElement element) {
    WEffectPushButton* pWidget = new WEffectPushButton(m_pParent, m_pEffectsManager);
    commonWidgetSetup(element, pWidget);
    pWidget->setup(element, *m_pContext);
    pWidget->installEventFilter(m_pKeyboard);
    pWidget->installEventFilter(
            m_pControllerManager->getControllerLearningEventFilter());
    pWidget->Init();
    return pWidget;
}

QWidget* LegacySkinParser::parseEffectParameterName(QDomElement node) {
    WEffectParameterBase* pEffectParameter = new WEffectParameter(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffectParameter);
    return pEffectParameter;
}

QWidget* LegacySkinParser::parseEffectButtonParameterName(QDomElement node) {
    WEffectParameterBase* pEffectButtonParameter = new WEffectButtonParameter(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffectButtonParameter);
    return pEffectButtonParameter;
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
            SKIN_WARNING(node, *m_pContext)
                    << "Could not parse widget MinimumSize:" << size;
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
            SKIN_WARNING(node, *m_pContext)
                    << "Could not parse widget MaximumSize:" << size;
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
        } else if (!xs.isEmpty()) {
            SKIN_WARNING(node, *m_pContext)
                    << "Could not parse horizontal size policy:" << xs;
        }

        QSizePolicy::Policy verticalPolicy;
        if (parseSizePolicy(&ys, &verticalPolicy)) {
            sizePolicy.setVerticalPolicy(verticalPolicy);
        } else if (!ys.isEmpty()) {
            SKIN_WARNING(node, *m_pContext)
                    << "Could not parse vertical size policy:" << ys;
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

void LegacySkinParser::commonWidgetSetup(QDomNode node,
                                         WBaseWidget* pBaseWidget,
                                         bool allowConnections) {
    setupBaseWidget(node, pBaseWidget);
    setupWidget(node, pBaseWidget->toQWidget());
    // NOTE(rryan): setupConnections should come after setupBaseWidget and
    // setupWidget since a BindProperty connection to the display parameter can
    // cause the widget to be polished (i.e. style computed) before it is
    // ready. The most common case is that the object name has not yet been set
    // in setupWidget. See Bug #1285836.
    if (allowConnections) {
        setupConnections(node, pBaseWidget);
    }
}

void LegacySkinParser::setupBaseWidget(QDomNode node,
                                       WBaseWidget* pBaseWidget) {
    // Tooltip
    if (m_pContext->hasNode(node, "Tooltip")) {
        QString toolTip = m_pContext->selectString(node, "Tooltip");
        pBaseWidget->prependBaseTooltip(toolTip);
    } else if (m_pContext->hasNode(node, "TooltipId")) {
        QString toolTipId = m_pContext->selectString(node, "TooltipId");
        QString toolTip = m_tooltips.tooltipForId(toolTipId);

        if (!toolTip.isEmpty()) {
            pBaseWidget->prependBaseTooltip(toolTip);
        } else if (!toolTipId.isEmpty()) {
            // Only warn if there was a tooltip ID specified and no tooltip for
            // that ID.
            SKIN_WARNING(node, *m_pContext)
                    << "Invalid <TooltipId> in skin.xml:" << toolTipId;
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
    if (!style.isEmpty()) {
        pWidget->setStyleSheet(style);
    }
}

void LegacySkinParser::setupConnections(QDomNode node, WBaseWidget* pWidget) {
    // For each connection
    QDomNode con = m_pContext->selectNode(node, "Connection");

    ControlParameterWidgetConnection* pLastLeftOrNoButtonConnection = NULL;

    for (QDomNode con = m_pContext->selectNode(node, "Connection");
            !con.isNull();
            con = con.nextSibling()) {
        // Check that the control exists
        bool created = false;
        ControlObject* control = controlFromConfigNode(
                con.toElement(), "ConfigKey", &created);

        if (!control) {
            continue;
        }

        ValueTransformer* pTransformer = NULL;
        if (m_pContext->hasNode(con, "Transform")) {
            QDomElement element = m_pContext->selectElement(con, "Transform");
            pTransformer = ValueTransformer::parseFromXml(element, *m_pContext);
        }

        if (m_pContext->hasNode(con, "BindProperty")) {
            QString property = m_pContext->selectString(con, "BindProperty");
            //qDebug() << "Making property connection for" << property;

            ControlObjectSlave* pControlWidget =
                    new ControlObjectSlave(control->getKey(),
                                           pWidget->toQWidget());

            ControlWidgetPropertyConnection* pConnection =
                    new ControlWidgetPropertyConnection(pWidget, pControlWidget,
                                                        pTransformer, property);
            pWidget->addPropertyConnection(pConnection);

            // If we created this control, bind it to the
            // ControlWidgetConnection so that it is deleted when the connection
            // is deleted.
            if (created) {
                control->setParent(pConnection);
            }
        } else {
            bool nodeValue;
            Qt::MouseButton state = parseButtonState(con, *m_pContext);

            bool directionOptionSet = false;
            int directionOption = ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET;
            if(m_pContext->hasNodeSelectBool(
                    con, "ConnectValueFromWidget", &nodeValue)) {
                if (nodeValue) {
                    directionOption = directionOption | ControlParameterWidgetConnection::DIR_FROM_WIDGET;
                } else {
                    directionOption = directionOption & ~ControlParameterWidgetConnection::DIR_FROM_WIDGET;
                }
                directionOptionSet = true;
            }

            if(m_pContext->hasNodeSelectBool(
                    con, "ConnectValueToWidget", &nodeValue)) {
                if (nodeValue) {
                    directionOption = directionOption | ControlParameterWidgetConnection::DIR_TO_WIDGET;
                } else {
                    directionOption = directionOption & ~ControlParameterWidgetConnection::DIR_TO_WIDGET;
                }
                directionOptionSet = true;
            }

            if (!directionOptionSet) {
                // default:
                // no direction option is explicite set
                // Set default flag to allow the widget to change this during setup
                directionOption |= ControlParameterWidgetConnection::DIR_DEFAULT;
            }

            int emitOption =
                    ControlParameterWidgetConnection::EMIT_ON_PRESS;
            if(m_pContext->hasNodeSelectBool(
                    con, "EmitOnDownPress", &nodeValue)) {
                if (nodeValue) {
                    emitOption = ControlParameterWidgetConnection::EMIT_ON_PRESS;
                } else {
                    emitOption = ControlParameterWidgetConnection::EMIT_ON_RELEASE;
                }
            } else if(m_pContext->hasNodeSelectBool(
                    con, "EmitOnPressAndRelease", &nodeValue)) {
                if (nodeValue) {
                    emitOption = ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE;
                } else {
                    SKIN_WARNING(con, *m_pContext)
                            << "LegacySkinParser::setupConnections(): EmitOnPressAndRelease must not set false";
                }
            } else {
                // default:
                // no emit option is set
                // Allow to change the emitOption from Widget
                emitOption |= ControlParameterWidgetConnection::EMIT_DEFAULT;
            }

            // Connect control proxy to widget. Parented to pWidget so it is not
            // leaked.
            ControlObjectSlave* pControlWidget = new ControlObjectSlave(
                    control->getKey(), pWidget->toQWidget());
            ControlParameterWidgetConnection* pConnection = new ControlParameterWidgetConnection(
                    pWidget, pControlWidget, pTransformer,
                    static_cast<ControlParameterWidgetConnection::DirectionOption>(directionOption),
                    static_cast<ControlParameterWidgetConnection::EmitOption>(emitOption));

            // If we created this control, bind it to the
            // ControlWidgetConnection so that it is deleted when the connection
            // is deleted.
            if (created) {
                control->setParent(pConnection);
            }

            switch (state) {
            case Qt::NoButton:
                pWidget->addConnection(pConnection);
                if (directionOption & ControlParameterWidgetConnection::DIR_TO_WIDGET) {
                    pLastLeftOrNoButtonConnection = pConnection;
                }
                break;
            case Qt::LeftButton:
                pWidget->addLeftConnection(pConnection);
                if (directionOption & ControlParameterWidgetConnection::DIR_TO_WIDGET) {
                    pLastLeftOrNoButtonConnection = pConnection;
                }
                break;
            case Qt::RightButton:
                pWidget->addRightConnection(pConnection);
                break;
            default:
                break;
            }

            // We only add info for controls that this widget affects, not
            // controls that only affect the widget.
            if (directionOption & ControlParameterWidgetConnection::DIR_FROM_WIDGET) {
                m_pControllerManager->getControllerLearningEventFilter()
                        ->addWidgetClickInfo(pWidget->toQWidget(), state, control,
                                static_cast<ControlParameterWidgetConnection::EmitOption>(emitOption));

                // Add keyboard shortcut info to tooltip string
                QString key = m_pContext->selectString(con, "ConfigKey");
                ConfigKey configKey = ConfigKey::parseCommaSeparated(key);

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
    }

    // Legacy behavior: The last left-button or no-button connection with
    // connectValueToWidget is the display connection. If no left-button or
    // no-button connection exists, use the last right-button connection as the
    // display connection.
    if (pLastLeftOrNoButtonConnection != NULL) {
        pWidget->setDisplayConnection(pLastLeftOrNoButtonConnection);
    }
}

void LegacySkinParser::addShortcutToToolTip(WBaseWidget* pWidget, const QString& shortcut, const QString& cmd) {
    if (shortcut.isEmpty()) {
        return;
    }

    QString tooltip;

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
    pWidget->appendBaseTooltip(tooltip);
}
