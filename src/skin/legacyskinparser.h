#ifndef LEGACYSKINPARSER_H
#define LEGACYSKINPARSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDomElement>
#include <QMutex>

#include "preferences/usersettings.h"
#include "skin/skinparser.h"
#include "vinylcontrol/vinylcontrolmanager.h"
#include "skin/tooltips.h"
#include "proto/skin.pb.h"
#include "util/memory.h"

class WBaseWidget;
class Library;
class KeyboardEventFilter;
class PlayerManager;
class EffectsManager;
class RecordingManager;
class ControllerManager;
class SkinContext;
class WLabel;
class ControlObject;
class LaunchImage;
class WWidgetGroup;

class LegacySkinParser : public QObject, public SkinParser {
    Q_OBJECT
  public:
    LegacySkinParser(UserSettingsPointer pConfig);
    LegacySkinParser(UserSettingsPointer pConfig,
                     KeyboardEventFilter* pKeyboard, PlayerManager* pPlayerManager,
                     ControllerManager* pControllerManager,
                     Library* pLibrary, VinylControlManager* pVCMan,
                     EffectsManager* pEffectsManager,
                     RecordingManager* pRecordingManager);
    virtual ~LegacySkinParser();

    virtual bool canParse(const QString& skinPath);
    virtual QWidget* parseSkin(const QString& skinPath, QWidget* pParent);

    LaunchImage* parseLaunchImage(const QString& skinPath, QWidget* pParent);

    // Legacy support for looking up the scheme list.
    static QList<QString> getSchemeList(const QString& qSkinPath);
    // Parse a skin manifest from the provided skin document root.
    static mixxx::skin::SkinManifest getSkinManifest(const QDomElement& skinDocument);
    static void freeChannelStrings();

    static Qt::MouseButton parseButtonState(const QDomNode& node,
                                            const SkinContext& context);

  private:
    static QDomElement openSkin(const QString& skinPath);

    QList<QWidget*> parseNode(const QDomElement& node);

    // Load the given template from file and return its document element.
    QDomElement loadTemplate(const QString& path);

    // Parsers for each node

    // Most widgets can use parseStandardWidget.
    template <class T>
    QWidget* parseStandardWidget(const QDomElement& element, bool timerListener=false);

    // Label widgets.
    template <class T>
    QWidget* parseLabelWidget(const QDomElement& element);
    void setupLabelWidget(const QDomElement& element, WLabel* pLabel);
    QWidget* parseText(const QDomElement& node);
    QWidget* parseTrackProperty(const QDomElement& node);
    QWidget* parseStarRating(const QDomElement& node);
    QWidget* parseNumberRate(const QDomElement& node);
    QWidget* parseNumberPos(const QDomElement& node);
    QWidget* parseEngineKey(const QDomElement& node);
    QWidget* parseBeatSpinBox(const QDomElement& node);
    QWidget* parseEffectChainName(const QDomElement& node);
    QWidget* parseEffectName(const QDomElement& node);
    QWidget* parseEffectParameterName(const QDomElement& node);
    QWidget* parseEffectButtonParameterName(const QDomElement& node);
    QWidget* parseEffectPushButton(const QDomElement& node);
    QWidget* parseEffectSelector(const QDomElement& node);

    // Legacy pre-1.12.0 skin support.
    QWidget* parseBackground(const QDomElement& node, QWidget* pOuterWidget, QWidget* pInnerWidget);

    // Grouping / layout.
    QWidget* parseWidgetGroup(const QDomElement& node);
    QWidget* parseWidgetStack(const QDomElement& node);
    QWidget* parseSizeAwareStack(const QDomElement& node);
    QWidget* parseSplitter(const QDomElement& node);
    void parseSingletonDefinition(const QDomElement& node);
    QWidget* parseSingletonContainer(const QDomElement& node);

    // Visual widgets.
    QWidget* parseVisual(const QDomElement& node);
    QWidget* parseOverview(const QDomElement& node);
    QWidget* parseSpinny(const QDomElement& node);

    // Library widgets.
    QWidget* parseTableView(const QDomElement& node);
    QWidget* parseSearchBox(const QDomElement& node);
    QWidget* parseLibrary(const QDomElement& node);
    QWidget* parseLibrarySidebar(const QDomElement& node);
    QWidget* parseBattery(const QDomElement& node);
    QWidget* parseRecordingDuration(const QDomElement& node);
    QWidget* parseCoverArt(const QDomElement& node);

    // Renders a template.
    QList<QWidget*> parseTemplate(const QDomElement& node);

    void commonWidgetSetup(const QDomNode& node, WBaseWidget* pBaseWidget,
                           bool allowConnections=true);
    void setupPosition(const QDomNode& node, QWidget* pWidget);
    void setupSize(const QDomNode& node, QWidget* pWidget);
    void setupBaseWidget(const QDomNode& node, WBaseWidget* pBaseWidget);
    void setupWidget(const QDomNode& node, QWidget* pWidget,
                     bool setupPosition=true);
    void setupConnections(const QDomNode& node, WBaseWidget* pWidget);
    void addShortcutToToolTip(WBaseWidget* pWidget, const QString& shortcut, const QString& cmd);
    QString getLibraryStyle(const QDomNode& node);
    QString getStyleFromNode(const QDomNode& node);

    QString lookupNodeGroup(const QDomElement& node);
    static const char* safeChannelString(const QString& channelStr);
    ControlObject* controlFromConfigNode(const QDomElement& element,
                                         const QString& nodeName,
                                         bool* created);

    QString parseLaunchImageStyle(const QDomNode& node);
    void parseChildren(const QDomElement& node, WWidgetGroup* pGroup);

    UserSettingsPointer m_pConfig;
    KeyboardEventFilter* m_pKeyboard;
    PlayerManager* m_pPlayerManager;
    ControllerManager* m_pControllerManager;
    Library* m_pLibrary;
    VinylControlManager* m_pVCManager;
    EffectsManager* m_pEffectsManager;
    RecordingManager* m_pRecordingManager;
    QWidget* m_pParent;
    std::unique_ptr<SkinContext> m_pContext;
    Tooltips m_tooltips;
    QHash<QString, QDomElement> m_templateCache;
    static QList<const char*> s_channelStrs;
    static QMutex s_safeStringMutex;
};


#endif /* LEGACYSKINPARSER_H */
