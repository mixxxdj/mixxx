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

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermanager.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "effects/effectsmanager.h"
#include "library/library.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_legacyskinparser.cpp"
#include "recording/recordingmanager.h"
#include "skin/colorschemeparser.h"
#include "skin/launchimage.h"
#include "skin/skincontext.h"
#include "util/cmdlineargs.h"
#include "util/timer.h"
#include "util/valuetransformer.h"
#include "util/xml.h"
#include "waveform/vsyncthread.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/controlwidgetconnection.h"
#include "widget/wbasewidget.h"
#include "widget/wbattery.h"
#include "widget/wbeatspinbox.h"
#include "widget/wcombobox.h"
#include "widget/wcoverart.h"
#include "widget/wdisplay.h"
#include "widget/weffect.h"
#include "widget/weffectbuttonparameter.h"
#include "widget/weffectchain.h"
#include "widget/weffectparameter.h"
#include "widget/weffectparameterbase.h"
#include "widget/weffectparameterknob.h"
#include "widget/weffectparameterknobcomposed.h"
#include "widget/weffectpushbutton.h"
#include "widget/weffectselector.h"
#include "widget/whotcuebutton.h"
#include "widget/wkey.h"
#include "widget/wknob.h"
#include "widget/wknobcomposed.h"
#include "widget/wlabel.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wnumber.h"
#include "widget/wnumberdb.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberrate.h"
#include "widget/woverviewhsv.h"
#include "widget/woverviewlmh.h"
#include "widget/woverviewrgb.h"
#include "widget/wpixmapstore.h"
#include "widget/wpushbutton.h"
#include "widget/wrecordingduration.h"
#include "widget/wsearchlineedit.h"
#include "widget/wsingletoncontainer.h"
#include "widget/wsizeawarestack.h"
#include "widget/wskincolor.h"
#include "widget/wslidercomposed.h"
#include "widget/wspinny.h"
#include "widget/wsplitter.h"
#include "widget/wstarrating.h"
#include "widget/wstatuslight.h"
#include "widget/wtime.h"
#include "widget/wtrackproperty.h"
#include "widget/wtracktext.h"
#include "widget/wtrackwidgetgroup.h"
#include "widget/wvumeter.h"
#include "widget/wwaveformviewer.h"
#include "widget/wwidget.h"
#include "widget/wwidgetgroup.h"
#include "widget/wwidgetstack.h"

using mixxx::skin::SkinManifest;

/// This QSet allows to make use of the implicit sharing
/// of QString instead of every widget keeping its own copy.
QSet<QString> LegacySkinParser::s_sharedGroupStrings;

static bool sDebug = false;

ControlObject* LegacySkinParser::controlFromConfigKey(
        const ConfigKey& key, bool bPersist, bool* pCreated) {
    if (!key.isValid()) {
        return nullptr;
    }
    // Don't warn if the control doesn't exist. Skins use this to create
    // controls.
    ControlObject* pControl = ControlObject::getControl(key, ControlFlag::NoWarnIfMissing);

    if (pControl) {
        if (pCreated) {
            *pCreated = false;
        }
        return pControl;
    }

    // TODO(rryan): Make this configurable by the skin.
    if (CmdlineArgs::Instance().getDeveloper()) {
        qWarning() << "Requested control does not exist:"
                   << QString("%1,%2").arg(key.group, key.item)
                   << "Creating it.";
    }
    // Since the usual behavior here is to create a skin-defined push
    // button, actually make it a push button and set it to toggle.
    ControlPushButton* controlButton = new ControlPushButton(key, bPersist);
    controlButton->setButtonMode(ControlPushButton::TOGGLE);

    if (pCreated) {
        *pCreated = true;
    }

    // If we created this control, add it to the set of skin-created
    // controls, so that it can be deleted when the MixxxMainWindow is
    // destroyed.
    VERIFY_OR_DEBUG_ASSERT(m_pSkinCreatedControls) {
        qWarning() << "Can't add skin-created control" << key << "to set, control will be leaked!";
        return controlButton;
    }
    DEBUG_ASSERT(!m_pSkinCreatedControls->contains(controlButton));

    m_pSkinCreatedControls->insert(controlButton);

    return controlButton;
}

ControlObject* LegacySkinParser::controlFromConfigNode(const QDomElement& element,
        const QString& nodeName,
        bool* pCreated) {
    QDomElement keyElement = m_pContext->selectElement(element, nodeName);
    if (keyElement.isNull()) {
        return nullptr;
    }

    QString name = m_pContext->nodeToString(keyElement);
    ConfigKey key = ConfigKey::parseCommaSeparated(name);

    bool bPersist = m_pContext->selectAttributeBool(keyElement, "persist", false);

    return controlFromConfigKey(key, bPersist, pCreated);
}

LegacySkinParser::LegacySkinParser(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_pSkinCreatedControls(nullptr),
          m_pKeyboard(nullptr),
          m_pPlayerManager(nullptr),
          m_pControllerManager(nullptr),
          m_pLibrary(nullptr),
          m_pVCManager(nullptr),
          m_pEffectsManager(nullptr),
          m_pRecordingManager(nullptr),
          m_pParent(nullptr) {
}

LegacySkinParser::LegacySkinParser(UserSettingsPointer pConfig,
        QSet<ControlObject*>* pSkinCreatedControls,
        KeyboardEventFilter* pKeyboard,
        PlayerManager* pPlayerManager,
        ControllerManager* pControllerManager,
        Library* pLibrary,
        VinylControlManager* pVCMan,
        EffectsManager* pEffectsManager,
        RecordingManager* pRecordingManager)
        : m_pConfig(pConfig),
          m_pSkinCreatedControls(pSkinCreatedControls),
          m_pKeyboard(pKeyboard),
          m_pPlayerManager(pPlayerManager),
          m_pControllerManager(pControllerManager),
          m_pLibrary(pLibrary),
          m_pVCManager(pVCMan),
          m_pEffectsManager(pEffectsManager),
          m_pRecordingManager(pRecordingManager),
          m_pParent(nullptr) {
    DEBUG_ASSERT(m_pSkinCreatedControls);
}

LegacySkinParser::~LegacySkinParser() {
}

bool LegacySkinParser::canParse(const QString& skinPath) {
    QDir skinDir(skinPath);

    if (!skinDir.exists()) {
        return false;
    }

    if (!skinDir.exists("skin.xml")) {
        return false;
    }

    // TODO check skin.xml for compliance

    return true;
}

// static
QDomElement LegacySkinParser::openSkin(const QString& skinPath) {
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
QList<QString> LegacySkinParser::getSchemeList(const QString& qSkinPath) {

    QDomElement docElem = openSkin(qSkinPath);
    QList<QString> schemeList;

    QDomNode colScheme = docElem.namedItem("Schemes");
    if (!colScheme.isNull() && colScheme.isElement()) {
        QDomNode scheme = colScheme.firstChild();

        while (!scheme.isNull()) {
            if (scheme.isElement()) {
                QString schemeName = XmlParse::selectNodeQString(scheme, "Name");
                schemeList.append(schemeName);
            }
            scheme = scheme.nextSibling();
        }
    }
    return schemeList;
}

// static
void LegacySkinParser::clearSharedGroupStrings() {
    // This frees up the memory allocated by the QString objects
    s_sharedGroupStrings.clear();
}

SkinManifest LegacySkinParser::getSkinManifest(const QDomElement& skinDocument) {
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
        for (int i = 0; i < attribute_nodes.count(); ++i) {
            QDomNode attribute_node = attribute_nodes.item(i);
            if (attribute_node.isElement()) {
                QDomElement attribute_element = attribute_node.toElement();
                QString configKey = attribute_element.attribute("config_key");
                QString persist = attribute_element.attribute("persist");
                QString value = attribute_element.text();
                SkinManifest::Attribute* attr = manifest.add_attribute();
                attr->set_config_key(configKey.toStdString());
                attr->set_persist(persist.toLower() == "true");
                attr->set_value(value.toStdString());
            }
        }
    }
    return manifest;
}

// static
Qt::MouseButton LegacySkinParser::parseButtonState(const QDomNode& node,
                                                   const SkinContext& context) {
    QString buttonState;
    if (context.hasNodeSelectString(node, "ButtonState", &buttonState)) {
        if (buttonState.contains("LeftButton", Qt::CaseInsensitive)) {
            return Qt::LeftButton;
        } else if (buttonState.contains("RightButton", Qt::CaseInsensitive)) {
            return Qt::RightButton;
        }
    }
    return Qt::NoButton;
}

QWidget* LegacySkinParser::parseSkin(const QString& skinPath, QWidget* pParent) {
    ScopedTimer timer("SkinLoader::parseSkin");
    qDebug() << "LegacySkinParser loading skin:" << skinPath;

    m_pContext = std::make_unique<SkinContext>(m_pConfig, skinPath + "/skin.xml");
    m_pContext->setSkinBasePath(skinPath);

    if (m_pParent) {
        qDebug() << "ERROR: Somehow a parent already exists -- you are probably re-using a LegacySkinParser which is not advisable!";
    }
    QDomElement skinDocument = openSkin(skinPath);

    if (skinDocument.isNull()) {
        qDebug() << "LegacySkinParser::parseSkin - failed for skin:" << skinPath;
        return nullptr;
    }

    SkinManifest manifest = getSkinManifest(skinDocument);

    // Apply SkinManifest attributes by looping through the proto.
    for (int i = 0; i < manifest.attribute_size(); ++i) {
        const SkinManifest::Attribute& attribute = manifest.attribute(i);
        if (!attribute.has_config_key()) {
            continue;
        }

        bool ok = false;
        double value = QString::fromStdString(attribute.value()).toDouble(&ok);
        if (!ok) {
            SKIN_WARNING(skinDocument, *m_pContext)
                    << "Error reading double value from skin attribute: "
                    << QString::fromStdString(attribute.value());
            continue;
        }

        ConfigKey configKey = ConfigKey::parseCommaSeparated(
            QString::fromStdString(attribute.config_key()));
        // Set the specified attribute, possibly creating the control
        // object in the process.
        bool created = false;
        // If there is no existing value for this CO in the skin,
        // update the config with the specified value. If the attribute
        // is set to persist, the value will be read when the control is created.
        // TODO: This is a hack, but right now it's the cleanest way to
        // get a CO with a specified initial value.  We should have a better
        // mechanism to provide initial default values for COs.
        if (attribute.persist() &&
            m_pConfig->getValueString(configKey).isEmpty()) {
            m_pConfig->set(configKey, ConfigValue(QString::number(value)));
        }
        ControlObject* pControl = controlFromConfigKey(configKey,
                                                       attribute.persist(),
                                                       &created);
        if (pControl == nullptr) {
            continue;
        }

        if (created) {
            if (!attribute.persist()) {
                // Only set the value if the control wasn't set up through
                // the persist logic.  Skin attributes are always
                // set on skin load.
                pControl->set(value);
            }
        } else {
            if (!attribute.persist()) {
                // Set the value using the static function, so the
                // value changes signal is transmitted to the owner.
                ControlObject::set(configKey, value);
            }
        }
    }

    ColorSchemeParser::setupLegacyColorSchemes(skinDocument, m_pConfig, &m_style, m_pContext.get());

    // don't parent till here so the first opengl waveform doesn't screw
    // up --bkgood
    // I'm disregarding this return value because I want to return the
    // created parent so MixxxMainWindow can use it for various purposes
    // (fullscreen mostly) --bkgood
    m_pParent = pParent;
    QList<QWidget*> widgets = parseNode(skinDocument);

    if (widgets.empty()) {
        SKIN_WARNING(skinDocument, *m_pContext) << "Skin produced no widgets!";
        return nullptr;
    } else if (widgets.size() > 1) {
        SKIN_WARNING(skinDocument, *m_pContext) << "Skin produced more than 1 widget!";
    }
    return widgets[0];
}

LaunchImage* LegacySkinParser::parseLaunchImage(const QString& skinPath, QWidget* pParent) {
    m_pContext = std::make_unique<SkinContext>(m_pConfig, skinPath + "/skin.xml");
    m_pContext->setSkinBasePath(skinPath + "/");

    QDomElement skinDocument = openSkin(skinPath);
    if (skinDocument.isNull()) {
        return nullptr;
    }

    QString nodeName = skinDocument.nodeName();
    if (nodeName != "skin") {
        return nullptr;
    }

    // This allows image urls like
    // url(skin:/style/mixxx-icon-logo-symbolic.svg);
    QStringList skinPaths(skinPath);
    QDir::setSearchPaths("skin", skinPaths);

    QString styleSheet = parseLaunchImageStyle(skinDocument);
    LaunchImage* pLaunchImage = new LaunchImage(pParent, styleSheet);
    setupSize(skinDocument, pLaunchImage);
    return pLaunchImage;
}



QList<QWidget*> wrapWidget(QWidget* pWidget) {
    QList<QWidget*> result;
    if (pWidget != nullptr) {
        result.append(pWidget);
    }
    return result;
}

QList<QWidget*> LegacySkinParser::parseNode(const QDomElement& node) {
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
    } else if (nodeName == "HotcueButton") {
        result = wrapWidget(parseHotcueButton(node));
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
        WVuMeter* pVuMeterWidget = parseStandardWidget<WVuMeter>(node);
        WaveformWidgetFactory::instance()->addTimerListener(pVuMeterWidget);
        result = wrapWidget(pVuMeterWidget);
    } else if (nodeName == "StatusLight") {
        result = wrapWidget(parseStandardWidget<WStatusLight>(node));
    } else if (nodeName == "Display") {
        result = wrapWidget(parseStandardWidget<WDisplay>(node));
    } else if (nodeName == "BeatSpinBox") {
        result = wrapWidget(parseBeatSpinBox(node));
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
    } else if (nodeName == "TrackWidgetGroup") {
        result = wrapWidget(parseTrackWidgetGroup(node));
    } else if (nodeName == "WidgetStack") {
        result = wrapWidget(parseWidgetStack(node));
    } else if (nodeName == "SizeAwareStack") {
        result = wrapWidget(parseSizeAwareStack(node));
    } else if (nodeName == "EffectChainName") {
        result = wrapWidget(parseEffectChainName(node));
    } else if (nodeName == "EffectName") {
        result = wrapWidget(parseEffectName(node));
    } else if (nodeName == "EffectSelector") {
        result = wrapWidget(parseEffectSelector(node));
    } else if (nodeName == "EffectParameterKnob") {
        result = wrapWidget(parseEffectParameterKnob(node));
    } else if (nodeName == "EffectParameterKnobComposed") {
        result = wrapWidget(parseEffectParameterKnobComposed(node));
    } else if (nodeName == "EffectParameterName") {
        result = wrapWidget(parseEffectParameterName(node));
    } else if (nodeName == "EffectButtonParameterName") {
        result = wrapWidget(parseEffectButtonParameterName(node));
    } else if (nodeName == "Spinny") {
        result = wrapWidget(parseSpinny(node));
    } else if (nodeName == "Time") {
        result = wrapWidget(parseLabelWidget<WTime>(node));
    } else if (nodeName == "RecordingDuration") {
        result = wrapWidget(parseRecordingDuration(node));
    } else if (nodeName == "Splitter") {
        result = wrapWidget(parseSplitter(node));
    } else if (nodeName == "LibrarySidebar") {
        result = wrapWidget(parseLibrarySidebar(node));
    } else if (nodeName == "Library") {
        result = wrapWidget(parseLibrary(node));
    } else if (nodeName == "Key") {
        result = wrapWidget(parseEngineKey(node));
    } else if (nodeName == "Battery") {
        result = wrapWidget(parseBattery(node));
    } else if (nodeName == "SetVariable") {
        m_pContext->updateVariable(node);
    } else if (nodeName == "Template") {
        result = parseTemplate(node);
    } else if (nodeName == "SingletonDefinition") {
        parseSingletonDefinition(node);
    } else if (nodeName == "SingletonContainer") {
        result = wrapWidget(parseStandardWidget<WSingletonContainer>(node));
    } else {
        SKIN_WARNING(node, *m_pContext) << "Invalid node name in skin:"
                                       << nodeName;
    }

    if (sDebug) {
        qDebug() << "END PARSE NODE" << nodeName;
    }
    return result;
}

QWidget* LegacySkinParser::parseSplitter(const QDomElement& node) {
    WSplitter* pSplitter = new WSplitter(m_pParent, m_pConfig);
    commonWidgetSetup(node, pSplitter);

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");
    QWidget* pOldParent = m_pParent;
    m_pParent = pSplitter;

    if (!childrenNode.isNull()) {
        // Descend children
        QDomNodeList children = childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (node.isElement()) {
                QList<QWidget*> children = parseNode(node.toElement());
                foreach (QWidget* pChild, children) {
                    if (pChild == nullptr) {
                        continue;
                    }
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

void LegacySkinParser::parseChildren(
        const QDomElement& node,
        WWidgetGroup* pGroup) {
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
                    if (pChild == nullptr) {
                        continue;
                    }
                    pGroup->addWidget(pChild);
                }
            }
        }
    }
    m_pParent = pOldParent;
}

QWidget* LegacySkinParser::parseWidgetGroup(const QDomElement& node) {
    WWidgetGroup* pGroup = new WWidgetGroup(m_pParent);
    commonWidgetSetup(node, pGroup);
    pGroup->setup(node, *m_pContext);
    pGroup->Init();
    parseChildren(node, pGroup);
    return pGroup;
}

QWidget* LegacySkinParser::parseWidgetStack(const QDomElement& node) {
    ControlObject* pNextControl = controlFromConfigNode(node.toElement(), "NextControl");
    ;
    ConfigKey nextConfigKey;
    if (pNextControl != nullptr) {
        nextConfigKey = pNextControl->getKey();
    }

    ControlObject* pPrevControl = controlFromConfigNode(node.toElement(), "PrevControl");
    ConfigKey prevConfigKey;
    if (pPrevControl != nullptr) {
        prevConfigKey = pPrevControl->getKey();
    }

    ControlObject* pCurrentPageControl = nullptr;
    ConfigKey currentPageConfigKey;
    QString currentpage_co = node.attribute("currentpage");
    if (currentpage_co.length() > 0) {
        ConfigKey configKey = ConfigKey::parseCommaSeparated(currentpage_co);
        bool persist = m_pContext->selectAttributeBool(node, "persist", false);
        pCurrentPageControl = controlFromConfigKey(configKey, persist);
        if (pCurrentPageControl != nullptr) {
            currentPageConfigKey = pCurrentPageControl->getKey();
        }
    }

    WWidgetStack* pStack = new WWidgetStack(m_pParent, nextConfigKey,
            prevConfigKey, currentPageConfigKey);
    pStack->setObjectName("WidgetStack");
    pStack->setContentsMargins(0, 0, 0, 0);
    commonWidgetSetup(node, pStack);

    QWidget* pOldParent = m_pParent;
    m_pParent = pStack;

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");
    if (!childrenNode.isNull()) {
        // Descend children
        QDomNodeList children = childrenNode.childNodes();

        for (int i = 0; i < children.count(); ++i) {
            QDomNode node = children.at(i);

            if (!node.isElement()) {
                continue;
            }
            QDomElement element = node.toElement();

            QList<QWidget*> child_widgets = parseNode(element);

            if (child_widgets.empty()) {
                SKIN_WARNING(node, *m_pContext)
                        << "WidgetStack child produced no widget.";
                continue;
            }

            if (child_widgets.size() > 1) {
                SKIN_WARNING(node, *m_pContext)
                        << "WidgetStack child produced multiple widgets."
                        << "All but the first are ignored.";
            }
            QWidget* pChild = child_widgets[0];

            if (pChild == nullptr) {
                continue;
            }

            ControlObject* pControl = nullptr;
            QString trigger_configkey = element.attribute("trigger");
            if (trigger_configkey.length() > 0) {
                ConfigKey configKey = ConfigKey::parseCommaSeparated(trigger_configkey);
                pControl = controlFromConfigKey(configKey, false);
            }
            int on_hide_select = -1;
            QString on_hide_attr = element.attribute("on_hide_select");
            if (on_hide_attr.length() > 0) {
                bool ok = false;
                on_hide_select = on_hide_attr.toInt(&ok);
                if (!ok) {
                    on_hide_select = -1;
                }
            }

            pStack->addWidgetWithControl(pChild, pControl, on_hide_select);
        }
    }

    // Init the widget last now that all the children have been created,
    // so if the current page was saved we can switch to the correct page.
    pStack->Init();
    m_pParent = pOldParent;
    return pStack;
}

QWidget* LegacySkinParser::parseSizeAwareStack(const QDomElement& node) {
    WSizeAwareStack* pStack = new WSizeAwareStack(m_pParent);
    pStack->setObjectName("SizeAwareStack");
    pStack->setContentsMargins(0, 0, 0, 0);
    commonWidgetSetup(node, pStack);

    QWidget* pOldParent = m_pParent;
    m_pParent = pStack;

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");
    if (!childrenNode.isNull()) {
        // Descend children
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

            if (pChild == nullptr) {
                continue;
            }

            pStack->addWidget(pChild);
        }
    }

    m_pParent = pOldParent;
    return pStack;
}

QWidget* LegacySkinParser::parseBackground(const QDomElement& node,
                                           QWidget* pOuterWidget,
                                           QWidget* pInnerWidget) {
    QLabel* bg = new QLabel(pInnerWidget);

    QString filename = m_pContext->selectString(node, "Path");
    QPixmap* background = WPixmapStore::getPixmapNoCache(
        m_pContext->makeSkinPath(filename), m_pContext->getScaleFactor());

    bg->move(0, 0);
    if (background != nullptr && !background->isNull()) {
        bg->setPixmap(*background);
    }

    bg->lower();

    pInnerWidget->move(0,0);
    if (background != nullptr && !background->isNull()) {
        pInnerWidget->setFixedSize(background->width(), background->height());
        pOuterWidget->setMinimumSize(background->width(), background->height());
    }

    // Default background color is now black, if people want to do <invert/>
    // filters they'll have to figure something out for this.
    QColor c(0,0,0);
    QString cStr;
    if (m_pContext->hasNodeSelectString(node, "BgColor", &cStr)) {
        c.setNamedColor(cStr);
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

template<class T>
T* LegacySkinParser::parseStandardWidget(const QDomElement& element) {
    T* pWidget = new T(m_pParent);
    commonWidgetSetup(element, pWidget);
    pWidget->setup(element, *m_pContext);
    pWidget->installEventFilter(m_pKeyboard);
    pWidget->installEventFilter(
            m_pControllerManager->getControllerLearningEventFilter());
    pWidget->Init();
    return pWidget;
}

template <class T>
QWidget* LegacySkinParser::parseLabelWidget(const QDomElement& element) {
    T* pLabel = new T(m_pParent);
    setupLabelWidget(element, pLabel);
    return pLabel;
}

void LegacySkinParser::setupLabelWidget(const QDomElement& element, WLabel* pLabel) {
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

QWidget* LegacySkinParser::parseOverview(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }

    WOverview* overviewWidget = nullptr;

    // "RGB" = "2", "HSV" = "1" or "Filtered" = "0" (LMH) waveform overview type
    int type = m_pConfig->getValue(ConfigKey("[Waveform]","WaveformOverviewType"), 2);
    if (type == 0) {
        overviewWidget = new WOverviewLMH(group, m_pPlayerManager, m_pConfig, m_pParent);
    } else if (type == 1) {
        overviewWidget = new WOverviewHSV(group, m_pPlayerManager, m_pConfig, m_pParent);
    } else {
        overviewWidget = new WOverviewRGB(group, m_pPlayerManager, m_pConfig, m_pParent);
    }

    connect(overviewWidget,
            &WOverview::trackDropped,
            m_pPlayerManager,
            &PlayerManager::slotLoadToPlayer);
    connect(overviewWidget, &WOverview::cloneDeck,
            m_pPlayerManager, &PlayerManager::slotCloneDeck);

    commonWidgetSetup(node, overviewWidget);
    overviewWidget->setup(node, *m_pContext);
    overviewWidget->installEventFilter(m_pKeyboard);
    overviewWidget->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    overviewWidget->Init();

    // Connect the player's load and unload signals to the overview widget.
    connect(pPlayer, &BaseTrackPlayer::newTrackLoaded, overviewWidget, &WOverview::slotTrackLoaded);
    connect(pPlayer, &BaseTrackPlayer::loadingTrack, overviewWidget, &WOverview::slotLoadingTrack);

    // just in case track already loaded
    overviewWidget->slotLoadingTrack(pPlayer->getLoadedTrack(), TrackPointer());
    overviewWidget->slotTrackLoaded(pPlayer->getLoadedTrack());

    return overviewWidget;
}

QWidget* LegacySkinParser::parseVisual(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }

    WWaveformViewer* viewer = new WWaveformViewer(group, m_pConfig, m_pParent);
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
    QObject::connect(pPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            viewer,
            &WWaveformViewer::slotTrackLoaded);
    QObject::connect(pPlayer,
            &BaseTrackPlayer::loadingTrack,
            viewer,
            &WWaveformViewer::slotLoadingTrack);

    connect(viewer,
            &WWaveformViewer::trackDropped,
            m_pPlayerManager,
            &PlayerManager::slotLoadToPlayer);
    connect(viewer, &WWaveformViewer::cloneDeck,
            m_pPlayerManager, &PlayerManager::slotCloneDeck);

    // if any already loaded
    viewer->slotTrackLoaded(pPlayer->getLoadedTrack());

    return viewer;
}

QWidget* LegacySkinParser::parseText(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }

    WTrackText* pTrackText = new WTrackText(m_pParent,
            m_pConfig,
            m_pLibrary->trackCollections(),
            group);
    setupLabelWidget(node, pTrackText);

    connect(pPlayer, &BaseTrackPlayer::newTrackLoaded, pTrackText, &WTrackText::slotTrackLoaded);
    connect(pPlayer, &BaseTrackPlayer::loadingTrack, pTrackText, &WTrackText::slotLoadingTrack);
    connect(pTrackText,
            &WTrackText::trackDropped,
            m_pPlayerManager,
            &PlayerManager::slotLoadToPlayer);
    connect(pTrackText, &WTrackText::cloneDeck, m_pPlayerManager, &PlayerManager::slotCloneDeck);

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        pTrackText->slotTrackLoaded(pTrack);
    }

    return pTrackText;
}

QWidget* LegacySkinParser::parseTrackProperty(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }

    WTrackProperty* pTrackProperty = new WTrackProperty(
            m_pParent,
            m_pConfig,
            m_pLibrary->trackCollections(),
            group);
    setupLabelWidget(node, pTrackProperty);

    connect(pPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            pTrackProperty,
            &WTrackProperty::slotTrackLoaded);
    connect(pPlayer,
            &BaseTrackPlayer::loadingTrack,
            pTrackProperty,
            &WTrackProperty::slotLoadingTrack);
    connect(pTrackProperty,
            &WTrackProperty::trackDropped,
            m_pPlayerManager,
            &PlayerManager::slotLoadToPlayer);
    connect(pTrackProperty,
            &WTrackProperty::cloneDeck,
            m_pPlayerManager,
            &PlayerManager::slotCloneDeck);

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        pTrackProperty->slotTrackLoaded(pTrack);
    }

    return pTrackProperty;
}

QWidget* LegacySkinParser::parseTrackWidgetGroup(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }

    WTrackWidgetGroup* pGroup = new WTrackWidgetGroup(
            m_pParent,
            m_pConfig,
            m_pLibrary->trackCollections(),
            group);
    commonWidgetSetup(node, pGroup);
    pGroup->setup(node, *m_pContext);
    pGroup->Init();
    parseChildren(node, pGroup);

    connect(pPlayer,
            &BaseTrackPlayer::newTrackLoaded,
            pGroup,
            &WTrackWidgetGroup::slotTrackLoaded);
    connect(pPlayer,
            &BaseTrackPlayer::loadingTrack,
            pGroup,
            &WTrackWidgetGroup::slotLoadingTrack);
    connect(pGroup,
            &WTrackWidgetGroup::trackDropped,
            m_pPlayerManager,
            &PlayerManager::slotLoadToPlayer);
    connect(pGroup,
            &WTrackWidgetGroup::cloneDeck,
            m_pPlayerManager,
            &PlayerManager::slotCloneDeck);

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        pGroup->slotTrackLoaded(pTrack);
    }
    return pGroup;
}

QWidget* LegacySkinParser::parseStarRating(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }

    WStarRating* pStarRating = new WStarRating(group, m_pParent);
    commonWidgetSetup(node, pStarRating, false);
    pStarRating->setup(node, *m_pContext);

    connect(pPlayer, &BaseTrackPlayer::newTrackLoaded, pStarRating, &WStarRating::slotTrackLoaded);
    connect(pPlayer, &BaseTrackPlayer::playerEmpty, pStarRating, [pStarRating]() {
        pStarRating->slotTrackLoaded(TrackPointer());
    });

    TrackPointer pTrack = pPlayer->getLoadedTrack();
    if (pTrack) {
        pStarRating->slotTrackLoaded(pTrack);
    }

    return pStarRating;
}

QWidget* LegacySkinParser::parseNumberRate(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    QColor c(255,255,255);
    QString cStr;
    if (m_pContext->hasNodeSelectString(node, "BgColor", &cStr)) {
        c.setNamedColor(cStr);
    }

    QPalette palette;
    //palette.setBrush(QPalette::Background, WSkinColor::getCorrectColor(c));
    palette.setBrush(QPalette::Button, Qt::NoBrush);

    WNumberRate* p = new WNumberRate(group, m_pParent);
    setupLabelWidget(node, p);

    // TODO(rryan): Let's look at removing this palette change in 1.12.0. I
    // don't think it's needed anymore.
    p->setPalette(palette);

    return p;
}

QWidget* LegacySkinParser::parseNumberPos(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    WNumberPos* p = new WNumberPos(group, m_pParent);
    setupLabelWidget(node, p);
    return p;
}

QWidget* LegacySkinParser::parseEngineKey(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    WKey* pEngineKey = new WKey(group, m_pParent);
    setupLabelWidget(node, pEngineKey);
    return pEngineKey;
}

QWidget* LegacySkinParser::parseBeatSpinBox(const QDomElement& node) {
    ControlObject* valueControl = controlFromConfigNode(node.toElement(), "Value");

    ConfigKey configKey;
    if (valueControl != nullptr) {
        configKey = valueControl->getKey();
    }

    WBeatSpinBox* pSpinbox = new WBeatSpinBox(m_pParent, configKey);
    commonWidgetSetup(node, pSpinbox);
    pSpinbox->setup(node, *m_pContext);

    return pSpinbox;
}

QWidget* LegacySkinParser::parseBattery(const QDomElement& node) {
    WBattery *p = new WBattery(m_pParent);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseRecordingDuration(const QDomElement& node) {
    WRecordingDuration *p = new WRecordingDuration(m_pParent, m_pRecordingManager);
    setupBaseWidget(node, p);
    setupWidget(node, p);
    p->setup(node, *m_pContext);
    setupConnections(node, p);
    p->installEventFilter(m_pKeyboard);
    p->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    return p;
}

QWidget* LegacySkinParser::parseSpinny(const QDomElement& node) {
    if (CmdlineArgs::Instance().getSafeMode()) {
        WLabel* dummy = new WLabel(m_pParent);
        //: Shown when Mixxx is running in safe mode.
        dummy->setText(tr("Safe Mode Enabled"));
        return dummy;
    }

    auto* waveformWidgetFactory = WaveformWidgetFactory::instance();

    if (!waveformWidgetFactory->isOpenGlAvailable() &&
            !waveformWidgetFactory->isOpenGlesAvailable()) {
        WLabel* dummy = new WLabel(m_pParent);
        //: Shown when Spinny can not be displayed. Please keep \n unchanged
        dummy->setText(tr("No OpenGL\nsupport."));
        return dummy;
    }

    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = m_pPlayerManager->getPlayer(group);
    if (!pPlayer) {
        SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
        return nullptr;
    }
    WSpinny* spinny = new WSpinny(m_pParent, group, m_pConfig, m_pVCManager, pPlayer);
    commonWidgetSetup(node, spinny);

    connect(waveformWidgetFactory,
            &WaveformWidgetFactory::renderSpinnies,
            spinny,
            &WSpinny::render);
    connect(waveformWidgetFactory, &WaveformWidgetFactory::swapSpinnies, spinny, &WSpinny::swap);
    connect(spinny, &WSpinny::trackDropped, m_pPlayerManager, &PlayerManager::slotLoadToPlayer);
    connect(spinny, &WSpinny::cloneDeck, m_pPlayerManager, &PlayerManager::slotCloneDeck);

    spinny->setup(node, *m_pContext);
    spinny->installEventFilter(m_pKeyboard);
    spinny->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    spinny->Init();
    return spinny;
}

QWidget* LegacySkinParser::parseSearchBox(const QDomElement& node) {
    // TODO(XXX): Currently this is the only opportunity to initialize
    // the static configuration settings of the widget. The settings
    // don't need to be static, if the widget instance could be connected
    // to changes in the configuration.
    const auto searchDebouncingTimeoutMillis =
            m_pConfig->getValue(
                    ConfigKey("[Library]","SearchDebouncingTimeoutMillis"),
                    WSearchLineEdit::kDefaultDebouncingTimeoutMillis);
    WSearchLineEdit::setDebouncingTimeoutMillis(searchDebouncingTimeoutMillis);

    WSearchLineEdit* pLineEditSearch = new WSearchLineEdit(m_pParent);
    commonWidgetSetup(node, pLineEditSearch, false);
    pLineEditSearch->setup(node, *m_pContext);

    m_pLibrary->bindSearchboxWidget(pLineEditSearch);

    return pLineEditSearch;
}

QWidget* LegacySkinParser::parseCoverArt(const QDomElement& node) {
    QString group = lookupNodeGroup(node);
    BaseTrackPlayer* pPlayer = nullptr;
    if (!group.isEmpty()) {
        pPlayer = m_pPlayerManager->getPlayer(group);
        if (!pPlayer) {
            SKIN_WARNING(node, *m_pContext) << "No player found for group:" << group;
            return nullptr;
        }
    }
    WCoverArt* pCoverArt = new WCoverArt(m_pParent, m_pConfig, group, pPlayer);
    commonWidgetSetup(node, pCoverArt);
    pCoverArt->setup(node, *m_pContext);

    // If no group was provided, hook the widget up to the Library.
    if (group.isEmpty()) {
        // Connect cover art signals to the library
        connect(m_pLibrary, &Library::switchToView, pCoverArt, &WCoverArt::slotReset);
        connect(m_pLibrary, &Library::enableCoverArtDisplay, pCoverArt, &WCoverArt::slotEnable);
        connect(m_pLibrary, &Library::trackSelected, pCoverArt, &WCoverArt::slotLoadTrack);
    } else if (pPlayer != nullptr) {
        connect(pCoverArt,
                &WCoverArt::trackDropped,
                m_pPlayerManager,
                &PlayerManager::slotLoadToPlayer);
        connect(pCoverArt, &WCoverArt::cloneDeck,
                m_pPlayerManager, &PlayerManager::slotCloneDeck);
    }

    return pCoverArt;
}

void LegacySkinParser::parseSingletonDefinition(const QDomElement& node) {
    QString objectName = m_pContext->selectString(node, "ObjectName");
    if (objectName.isEmpty()) {
        SKIN_WARNING(node, *m_pContext)
                << "SingletonDefinition requires an ObjectName";
    }

    QDomNode childrenNode = m_pContext->selectNode(node, "Children");
    if (childrenNode.isNull()) {
        SKIN_WARNING(node, *m_pContext)
                << "SingletonDefinition requires a Children tag with one child";
    }

    // Descend children, taking the first valid element.
    QDomNode child_node;
    QDomNodeList children = childrenNode.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        child_node = children.at(i);
        if (child_node.isElement()) {
            break;
        }
    }

    if (child_node.isNull()) {
        SKIN_WARNING(node, *m_pContext)
                << "SingletonDefinition Children node is empty";
        return;
    }

    QDomElement element = child_node.toElement();
    QList<QWidget*> child_widgets = parseNode(element);
    if (child_widgets.empty()) {
        SKIN_WARNING(node, *m_pContext)
                << "SingletonDefinition child produced no widget.";
        return;
    } else if (child_widgets.size() > 1) {
        SKIN_WARNING(node, *m_pContext)
                << "SingletonDefinition child produced multiple widgets."
                << "All but the first are ignored.";
    }

    QWidget* pChild = child_widgets[0];
    if (pChild == nullptr) {
        SKIN_WARNING(node, *m_pContext)
                << "SingletonDefinition child widget is NULL";
        return;
    }

    pChild->setObjectName(objectName);
    m_pContext->defineSingleton(objectName, pChild);
    pChild->hide();
}

QWidget* LegacySkinParser::parseLibrary(const QDomElement& node) {
    WLibrary* pLibraryWidget = new WLibrary(m_pParent);
    pLibraryWidget->installEventFilter(m_pKeyboard);
    pLibraryWidget->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    pLibraryWidget->setup(node, *m_pContext);

    // Connect Library search signals to the WLibrary
    connect(m_pLibrary,
            &Library::search,
            pLibraryWidget,
            &WLibrary::search);

    m_pLibrary->bindLibraryWidget(pLibraryWidget, m_pKeyboard);

    // This must come after the bindLibraryWidget or we will not style any of the
    // LibraryView's because they have not been added yet.
    commonWidgetSetup(node, pLibraryWidget, false);

    return pLibraryWidget;
}

QWidget* LegacySkinParser::parseLibrarySidebar(const QDomElement& node) {
    WLibrarySidebar* pLibrarySidebar = new WLibrarySidebar(m_pParent);
    pLibrarySidebar->installEventFilter(m_pKeyboard);
    pLibrarySidebar->installEventFilter(m_pControllerManager->getControllerLearningEventFilter());
    m_pLibrary->bindSidebarWidget(pLibrarySidebar);
    commonWidgetSetup(node, pLibrarySidebar, false);
    return pLibrarySidebar;
}

QWidget* LegacySkinParser::parseTableView(const QDomElement& node) {
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
            1,                // From row 1
            0,                // From column 0
            1,                // Span 1 row
            3,                // Span 3 columns
            Qt::Alignment()); // Default alignment

    pTabWidget->addWidget(pLibraryPage);
    pTabWidget->setStyleSheet(getLibraryStyle(node));

    return pTabWidget;
}

QString LegacySkinParser::getLibraryStyle(const QDomNode& node) {
    QString style = getStyleFromNode(node);

    // Workaround to support legacy color styling
    QColor color(0,0,0);

    QString styleHack = "";
    QString fgColor;
    if (m_pContext->hasNodeSelectString(node, "FgColor", &fgColor)) {
        color.setNamedColor(fgColor);
        color = WSkinColor::getCorrectColor(color);

        styleHack.append(QString("WLibraryTableView { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("WLibrarySidebar { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("WSearchLineEdit { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QTextBrowser { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QLabel { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QRadioButton { color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QSpinBox { color: %1; }\n ").arg(color.name()));
    }

    QString bgColor;
    if (m_pContext->hasNodeSelectString(node, "BgColor", &bgColor)) {
        color.setNamedColor(bgColor);
        color = WSkinColor::getCorrectColor(color);
        styleHack.append(QString("WLibraryTableView {  background-color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("WLibrarySidebar {  background-color: %1; }\n ").arg(color.name()));

        styleHack.append(QString("WSearchLineEdit {  background-color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QTextBrowser {  background-color: %1; }\n ").arg(color.name()));
        styleHack.append(QString("QSpinBox {  background-color: %1; }\n ").arg(color.name()));
    }

    QString bgColorRowEven;
    if (m_pContext->hasNodeSelectString(node, "BgColorRowEven", &bgColorRowEven)) {
        color.setNamedColor(bgColorRowEven);
        color = WSkinColor::getCorrectColor(color);

        styleHack.append(QString("WLibraryTableView { background: %1; }\n ").arg(color.name()));
    }

    QString bgColorRowUneven;
    if (m_pContext->hasNodeSelectString(node, "BgColorRowUneven", &bgColorRowUneven)) {
        color.setNamedColor(bgColorRowUneven);
        color = WSkinColor::getCorrectColor(color);

        styleHack.append(QString("WLibraryTableView { alternate-background-color: %1; }\n ").arg(color.name()));
    }
    style.prepend(styleHack);
    return style;
}

QDomElement LegacySkinParser::loadTemplate(const QString& path) {
    QFileInfo templateFileInfo(path);

    QString absolutePath = templateFileInfo.absoluteFilePath();

    auto it = m_templateCache.constFind(absolutePath);
    if (it != m_templateCache.constEnd()) {
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
    m_pContext->setSkinTemplatePath(templateFileInfo.absoluteDir().absolutePath());
    return tmpl.documentElement();
}

QList<QWidget*> LegacySkinParser::parseTemplate(const QDomElement& node) {
    if (!node.hasAttribute("src")) {
        SKIN_WARNING(node, *m_pContext)
                << "Template instantiation without src attribute:"
                << node.text();
        return QList<QWidget*>();
    }

    QString path = node.attribute("src");

    std::unique_ptr<SkinContext> pOldContext = std::move(m_pContext);
    m_pContext = std::make_unique<SkinContext>(pOldContext.get());

    QDomElement templateNode = loadTemplate(path);

    if (templateNode.isNull()) {
        SKIN_WARNING(node, *m_pContext) << "Template instantiation for template failed:" << path;
        m_pContext = std::move(pOldContext);
        return QList<QWidget*>();
    }

    if (sDebug) {
        qDebug() << "BEGIN TEMPLATE" << path;
    }

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

    m_pContext = std::move(pOldContext);

    if (sDebug) {
        qDebug() << "END TEMPLATE" << path;
    }
    return widgets;
}

QString LegacySkinParser::lookupNodeGroup(const QDomElement& node) {
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

    QString sharedGroup = getSharedGroupString(group);
    return sharedGroup;
}

// static
QString LegacySkinParser::getSharedGroupString(const QString& channelStr) {
    QSet<QString>::iterator i = s_sharedGroupStrings.insert(channelStr);
    return *i;
}

QWidget* LegacySkinParser::parseHotcueButton(const QDomElement& element) {
    QString group = lookupNodeGroup(element);
    WHotcueButton* pWidget = new WHotcueButton(group, m_pParent);
    commonWidgetSetup(element, pWidget);
    pWidget->setup(element, *m_pContext);
    pWidget->installEventFilter(m_pKeyboard);
    pWidget->installEventFilter(
            m_pControllerManager->getControllerLearningEventFilter());
    pWidget->Init();
    return pWidget;
}

QWidget* LegacySkinParser::parseEffectChainName(const QDomElement& node) {
    WEffectChain* pEffectChain = new WEffectChain(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffectChain);
    return pEffectChain;
}

QWidget* LegacySkinParser::parseEffectName(const QDomElement& node) {
    WEffect* pEffect = new WEffect(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffect);
    return pEffect;
}

QWidget* LegacySkinParser::parseEffectSelector(const QDomElement& node) {
    WEffectSelector* pSelector = new WEffectSelector(m_pParent, m_pEffectsManager);
    commonWidgetSetup(node, pSelector);
    pSelector->setup(node, *m_pContext);
    pSelector->installEventFilter(m_pKeyboard);
    pSelector->installEventFilter(
        m_pControllerManager->getControllerLearningEventFilter());
    pSelector->Init();
    return pSelector;
}

QWidget* LegacySkinParser::parseEffectParameterKnob(const QDomElement& node) {
    WEffectParameterKnob* pParameterKnob =
            new WEffectParameterKnob(m_pParent, m_pEffectsManager);
    commonWidgetSetup(node, pParameterKnob);
    pParameterKnob->setup(node, *m_pContext);
    pParameterKnob->installEventFilter(m_pKeyboard);
    pParameterKnob->installEventFilter(
        m_pControllerManager->getControllerLearningEventFilter());
    pParameterKnob->Init();
    const QList<ControlParameterWidgetConnection*> connections =
            pParameterKnob->connections();
    if (!connections.isEmpty()) {
        pParameterKnob->setupEffectParameterSlot(connections.at(0)->getKey());
    } else {
        SKIN_WARNING(node, *m_pContext)
                << "EffectParameterKnob node could not attach to effect parameter.";
    }
    return pParameterKnob;
}

QWidget* LegacySkinParser::parseEffectParameterKnobComposed(const QDomElement& node) {
    WEffectParameterKnobComposed* pParameterKnob =
            new WEffectParameterKnobComposed(m_pParent, m_pEffectsManager);
    commonWidgetSetup(node, pParameterKnob);
    pParameterKnob->setup(node, *m_pContext);
    pParameterKnob->installEventFilter(m_pKeyboard);
    pParameterKnob->installEventFilter(
        m_pControllerManager->getControllerLearningEventFilter());
    pParameterKnob->Init();
    const QList<ControlParameterWidgetConnection*> connections =
            pParameterKnob->connections();
    if (!connections.isEmpty()) {
        pParameterKnob->setupEffectParameterSlot(connections.at(0)->getKey());
    } else {
        SKIN_WARNING(node, *m_pContext)
                << "EffectParameterKnobComposed node could not attach to effect parameter.";
    }
    return pParameterKnob;
}

QWidget* LegacySkinParser::parseEffectPushButton(const QDomElement& element) {
    WEffectPushButton* pWidget = new WEffectPushButton(m_pParent, m_pEffectsManager);
    commonWidgetSetup(element, pWidget);
    pWidget->setup(element, *m_pContext);
    pWidget->installEventFilter(m_pKeyboard);
    pWidget->installEventFilter(
            m_pControllerManager->getControllerLearningEventFilter());
    pWidget->Init();
    const QList<ControlParameterWidgetConnection*> connections =
            pWidget->leftConnections();
    if (!connections.isEmpty()) {
        pWidget->setupEffectParameterSlot(connections.at(0)->getKey());
    } else {
        SKIN_WARNING(element, *m_pContext)
                << "EffectPushButton node could not attach to effect parameter.";
    }
    return pWidget;
}

QWidget* LegacySkinParser::parseEffectParameterName(const QDomElement& node) {
    WEffectParameterBase* pEffectParameter = new WEffectParameter(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffectParameter);
    return pEffectParameter;
}

QWidget* LegacySkinParser::parseEffectButtonParameterName(const QDomElement& node) {
    WEffectParameterBase* pEffectButtonParameter = new WEffectButtonParameter(m_pParent, m_pEffectsManager);
    setupLabelWidget(node, pEffectButtonParameter);
    return pEffectButtonParameter;
}

void LegacySkinParser::setupPosition(const QDomNode& node, QWidget* pWidget) {
    QString pos;
    if (m_pContext->hasNodeSelectString(node, "Pos", &pos)) {
        QString xs = pos.left(pos.indexOf(","));
        QString ys = pos.mid(pos.indexOf(",") + 1);
        int x = m_pContext->scaleToWidgetSize(xs);
        int y = m_pContext->scaleToWidgetSize(ys);
        pWidget->move(x,y);
    }
}

bool parseSizePolicy(QString* input, QSizePolicy::Policy* policy) {
    if (input->endsWith("me")) {
        *policy = QSizePolicy::MinimumExpanding;
        *input = input->left(input->size() - 2);
    } else if (input->endsWith("e")) {
        *policy = QSizePolicy::Expanding;
        *input = input->left(input->size() - 1);
    } else if (input->endsWith("i")) {
        *policy = QSizePolicy::Ignored;
        *input = input->left(input->size() - 1);
    } else if (input->endsWith("min")) {
        *policy = QSizePolicy::Minimum;
        *input = input->left(input->size() - 3);
    } else if (input->endsWith("max")) {
        *policy = QSizePolicy::Maximum;
        *input = input->left(input->size() - 3);
    } else if (input->endsWith("p")) {
        *policy = QSizePolicy::Preferred;
        *input = input->left(input->size() - 1);
    } else if (input->endsWith("f")) {
        *policy = QSizePolicy::Fixed;
        *input = input->left(input->size() - 1);
    } else {
        return false;
    }
    return true;
}

void LegacySkinParser::setupSize(const QDomNode& node, QWidget* pWidget) {
    QString size;
    if (m_pContext->hasNodeSelectString(node, "MinimumSize", &size)) {
        int comma = size.indexOf(",");
        QString xs = size.left(comma);
        QString ys = size.mid(comma + 1);

        int x = m_pContext->scaleToWidgetSize(xs);
        int y = m_pContext->scaleToWidgetSize(ys);

        // -1 means do not set.
        if (x >= 0 && y >= 0) {
            pWidget->setMinimumSize(x, y);
        } else if (x >= 0 && y == -1) {
            pWidget->setMinimumWidth(x);
        } else if (y >= 0 && x == -1) {
            pWidget->setMinimumHeight(y);
        } else if (x != -1 || y != -1) {
            SKIN_WARNING(node, *m_pContext)
                    << "Could not parse widget MinimumSize:" << size;
	}
    }


    if (m_pContext->hasNodeSelectString(node, "MaximumSize", &size)) {
        int comma = size.indexOf(",");
        QString xs = size.left(comma);
        QString ys = size.mid(comma+1);

        int x = m_pContext->scaleToWidgetSize(xs);
        int y = m_pContext->scaleToWidgetSize(ys);

        // -1 means do not set.
        if (x >= 0 && y >= 0) {
            pWidget->setMaximumSize(x, y);
        } else if (x >= 0 && y == -1) {
            pWidget->setMaximumWidth(x);
        } else if (y >= 0 && x == -1) {
            pWidget->setMaximumHeight(y);
        } else if (x != -1 || y != -1) {
            SKIN_WARNING(node, *m_pContext)
                    << "Could not parse widget MaximumSize:" << size;
        }
    }

    bool hasSizePolicyNode = false;
    if (m_pContext->hasNodeSelectString(node, "SizePolicy", &size)) {
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

    if (m_pContext->hasNodeSelectString(node, "Size", &size)) {
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

        int x = m_pContext->scaleToWidgetSize(xs);
        if (x >= 0) {
            if (hasHorizontalPolicy &&
                    sizePolicy.horizontalPolicy() == QSizePolicy::Fixed) {
                //qDebug() << "setting width fixed to" << x;
                pWidget->setFixedWidth(x);
            } else {
                //qDebug() << "setting width to" << x;
                pWidget->setMinimumWidth(x);
            }
        }

        int y = m_pContext->scaleToWidgetSize(ys);
        if (y >= 0) {
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

//static
QString LegacySkinParser::getStyleFromNode(const QDomNode& node) {
    QDomElement styleElement = SkinContext::selectElement(node, "Style");

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

        QString platformSpecificAttribute;
#if defined(Q_OS_MAC)
        platformSpecificAttribute = "src-mac";
#elif defined(__WINDOWS__)
        platformSpecificAttribute = "src-windows";
#else
        platformSpecificAttribute = "src-linux";
#endif

        if (styleElement.hasAttribute(platformSpecificAttribute)) {
            QString platformSpecificSrc = styleElement.attribute(platformSpecificAttribute);
            QFile platformSpecificFile(platformSpecificSrc);
            if (platformSpecificFile.open(QIODevice::ReadOnly)) {
                QByteArray fileBytes = platformSpecificFile.readAll();

                style += QString::fromLocal8Bit(fileBytes.constData(),
                                                fileBytes.length());
            }
        }

// This section can be enabled on demand. It is useful to tweak
// pixel sized values for different scalings. But we should know if this is
// actually used when migrating to Qt5
#if 0
        // now load style files with suffix for HiDPI scaling.
        // We follow here the Gnome/Unity scaling slider approach, where
        // the widgets are scaled by an integer value once the slider is
        // greater or equal to the next integer step.
        // This should help to scale the GUI along with the native widgets once
        // we are on Qt 5.
        double scaleFactor = m_pContext->getScaleFactor();
        if (scaleFactor >= 3) {
            // Try to load with @3x suffix
            QFileInfo info(file);
            QString strNewName = info.path() + "/" + info.baseName() + "@3x."
                    + info.completeSuffix();
            QFile file(strNewName);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray fileBytes = file.readAll();
                style.prepend(QString::fromLocal8Bit(fileBytes.constData(),
                                               fileBytes.length()));
            }
        } else if (scaleFactor >= 2) {
            // Try to load with @2x suffix
            QFileInfo info(file);
            QString strNewName = info.path() + "/" + info.baseName() + "@2x."
                    + info.completeSuffix();
            QFile file(strNewName);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray fileBytes = file.readAll();
                style.prepend(QString::fromLocal8Bit(fileBytes.constData(),
                                               fileBytes.length()));
            }
        }
#endif
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

void LegacySkinParser::commonWidgetSetup(const QDomNode& node,
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

void LegacySkinParser::setupBaseWidget(const QDomNode& node,
                                       WBaseWidget* pBaseWidget) {
    // Tooltip
    QString toolTip;
    QString toolTipId;
    if (m_pContext->hasNodeSelectString(node, "Tooltip", &toolTip)) {
        pBaseWidget->prependBaseTooltip(toolTip);
    } else if (m_pContext->hasNodeSelectString(node, "TooltipId", &toolTipId)) {
        toolTip = m_tooltips.tooltipForId(toolTipId);
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

void LegacySkinParser::setupWidget(const QDomNode& node,
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
    // check if we have a style from color schema:
    if (!m_style.isEmpty()) {
        style.append(m_style);
        m_style.clear(); // only apply color scheme to the first widget
    }
    if (!style.isEmpty()) {
        pWidget->setStyleSheet(style);
    }
}

void LegacySkinParser::setupConnections(const QDomNode& node, WBaseWidget* pWidget) {
    ControlParameterWidgetConnection* pLastLeftOrNoButtonConnection = nullptr;

    for (QDomNode con = m_pContext->selectNode(node, "Connection");
            !con.isNull();
            con = con.nextSibling()) {
        // Check that the control exists
        ControlObject* control = controlFromConfigNode(con.toElement(), "ConfigKey");

        if (control == nullptr) {
            continue;
        }

        ValueTransformer* pTransformer = nullptr;
        QDomElement transform = m_pContext->selectElement(con, "Transform");
        if (!transform.isNull()) {
            pTransformer = ValueTransformer::parseFromXml(transform, *m_pContext);
        }

        QString property;
        if (m_pContext->hasNodeSelectString(con, "BindProperty", &property)) {
            //qDebug() << "Making property connection for" << property;

            ControlWidgetPropertyConnection* pConnection =
                    new ControlWidgetPropertyConnection(pWidget, control->getKey(),
                                                        pTransformer, property);
            pWidget->addPropertyConnection(pConnection);
        } else {
            bool nodeValue;
            Qt::MouseButton state = parseButtonState(con, *m_pContext);

            bool directionOptionSet = false;
            int directionOption = ControlParameterWidgetConnection::DIR_FROM_AND_TO_WIDGET;
            if (m_pContext->hasNodeSelectBool(
                    con, "ConnectValueFromWidget", &nodeValue)) {
                if (nodeValue) {
                    directionOption = directionOption | ControlParameterWidgetConnection::DIR_FROM_WIDGET;
                } else {
                    directionOption = directionOption & ~ControlParameterWidgetConnection::DIR_FROM_WIDGET;
                }
                directionOptionSet = true;
            }

            if (m_pContext->hasNodeSelectBool(
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
                // no direction option is explicit set
                // Set default flag to allow the widget to change this during setup
                directionOption |= ControlParameterWidgetConnection::DIR_DEFAULT;
            }

            int emitOption =
                    ControlParameterWidgetConnection::EMIT_ON_PRESS;
            if (m_pContext->hasNodeSelectBool(
                    con, "EmitOnDownPress", &nodeValue)) {
                if (nodeValue) {
                    emitOption = ControlParameterWidgetConnection::EMIT_ON_PRESS;
                } else {
                    emitOption = ControlParameterWidgetConnection::EMIT_ON_RELEASE;
                }
            } else if (m_pContext->hasNodeSelectBool(
                    con, "EmitOnPressAndRelease", &nodeValue)) {
                if (nodeValue) {
                    emitOption = ControlParameterWidgetConnection::EMIT_ON_PRESS_AND_RELEASE;
                } else {
                    SKIN_WARNING(con, *m_pContext)
                            << "LegacySkinParser::setupConnections():"
                            << "Setting EmitOnPressAndRelease to false is not necessary.";
                }
            } else {
                // default:
                // no emit option is set
                // Allow to change the emitOption from Widget
                emitOption |= ControlParameterWidgetConnection::EMIT_DEFAULT;
            }

            ControlParameterWidgetConnection* pConnection = new ControlParameterWidgetConnection(
                    pWidget, control->getKey(), pTransformer,
                    static_cast<ControlParameterWidgetConnection::DirectionOption>(directionOption),
                    static_cast<ControlParameterWidgetConnection::EmitOption>(emitOption));

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
                // can't happen. Nothing else is returned by parseButtonState();
                DEBUG_ASSERT(false);
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
    if (pLastLeftOrNoButtonConnection != nullptr) {
        pWidget->setDisplayConnection(pLastLeftOrNoButtonConnection);
    }
}

void LegacySkinParser::addShortcutToToolTip(WBaseWidget* pWidget,
                                            const QString& shortcut, const QString& cmd) {
    if (shortcut.isEmpty()) {
        return;
    }

    QString tooltip;

    // translate shortcut to native text
    QString nativeShortcut = QKeySequence(shortcut, QKeySequence::PortableText).toString(QKeySequence::NativeText);

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

QString LegacySkinParser::parseLaunchImageStyle(const QDomNode& node) {
    return m_pContext->selectString(node, "LaunchImageStyle");
}
