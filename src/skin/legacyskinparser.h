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

class WBaseWidget;
class Library;
class MixxxKeyboard;
class PlayerManager;
class EffectsManager;
class ControllerManager;
class SkinContext;
class WLabel;
class ControlObject;
class LaunchImage;

class LegacySkinParser : public QObject, public SkinParser {
    Q_OBJECT
  public:
    LegacySkinParser();
    LegacySkinParser(UserSettingsPointer pConfig,
                     MixxxKeyboard* pKeyboard, PlayerManager* pPlayerManager,
                     ControllerManager* pControllerManager,
                     Library* pLibrary, VinylControlManager* pVCMan,
                     EffectsManager* pEffectsManager);
    virtual ~LegacySkinParser();

    virtual bool canParse(QString skinPath);
    virtual QWidget* parseSkin(QString skinPath, QWidget* pParent);

    LaunchImage* parseLaunchImage(QString skinPath, QWidget* pParent);

    // Legacy support for looking up the scheme list.
    static QList<QString> getSchemeList(QString qSkinPath);
    // Parse a skin manifest from the provided skin document root.
    static mixxx::skin::SkinManifest getSkinManifest(QDomElement skinDocument);
    static void freeChannelStrings();

    static Qt::MouseButton parseButtonState(QDomNode node,
                                            const SkinContext& context);

  private:
    static QDomElement openSkin(QString skinPath);

    QList<QWidget*> parseNode(QDomElement node);

    // Load the given template from file and return its document element.
    QDomElement loadTemplate(const QString& path);

    // Parsers for each node

    // Most widgets can use parseStandardWidget.
    template <class T>
    QWidget* parseStandardWidget(QDomElement element, bool timerListener=false);

    // Label widgets.
    template <class T>
    QWidget* parseLabelWidget(QDomElement element);
    void setupLabelWidget(QDomElement element, WLabel* pLabel);
    QWidget* parseText(QDomElement node);
    QWidget* parseTrackProperty(QDomElement node);
    QWidget* parseStarRating(QDomElement node);
    QWidget* parseNumberRate(QDomElement node);
    QWidget* parseNumberPos(QDomElement node);
    QWidget* parseEngineKey(QDomElement node);
    QWidget* parseEffectChainName(QDomElement node);
    QWidget* parseEffectName(QDomElement node);
    QWidget* parseEffectParameterName(QDomElement node);
    QWidget* parseEffectButtonParameterName(QDomElement node);
    QWidget* parseEffectPushButton(QDomElement node);

    // Legacy pre-1.12.0 skin support.
    QWidget* parseBackground(QDomElement node, QWidget* pOuterWidget, QWidget* pInnerWidget);

    // Grouping / layout.
    QWidget* parseWidgetGroup(QDomElement node);
    QWidget* parseWidgetStack(QDomElement node);
    QWidget* parseSizeAwareStack(QDomElement node);
    QWidget* parseSplitter(QDomElement node);
    void parseSingletonDefinition(QDomElement node);
    QWidget* parseSingletonContainer(QDomElement node);

    // Visual widgets.
    QWidget* parseVisual(QDomElement node);
    QWidget* parseOverview(QDomElement node);
    QWidget* parseSpinny(QDomElement node);

    // Library widgets.
    QWidget* parseTableView(QDomElement node);
    QWidget* parseSearchBox(QDomElement node);
    QWidget* parseLibrary(QDomElement node);
    QWidget* parseLibrarySidebar(QDomElement node);
    QWidget* parseCoverArt(QDomElement node);

    // Renders a template.
    QList<QWidget*> parseTemplate(QDomElement node);

    void commonWidgetSetup(QDomNode node, WBaseWidget* pBaseWidget,
                           bool allowConnections=true);
    void setupPosition(QDomNode node, QWidget* pWidget);
    void setupSize(QDomNode node, QWidget* pWidget);
    void setupBaseWidget(QDomNode node, WBaseWidget* pBaseWidget);
    void setupWidget(QDomNode node, QWidget* pWidget,
                     bool setupPosition=true);
    void setupConnections(QDomNode node, WBaseWidget* pWidget);
    void addShortcutToToolTip(WBaseWidget* pWidget, const QString& shortcut, const QString& cmd);
    QString getLibraryStyle(QDomNode node);
    QString getStyleFromNode(QDomNode node);

    QString lookupNodeGroup(QDomElement node);
    static const char* safeChannelString(QString channelStr);
    ControlObject* controlFromConfigNode(const QDomElement& element,
                                         const QString& nodeName,
                                         bool* created);

    QString parseLaunchImageStyle(QDomNode node);

    UserSettingsPointer m_pConfig;
    MixxxKeyboard* m_pKeyboard;
    PlayerManager* m_pPlayerManager;
    ControllerManager* m_pControllerManager;
    Library* m_pLibrary;
    VinylControlManager* m_pVCManager;
    EffectsManager* m_pEffectsManager;
    QWidget* m_pParent;
    SkinContext* m_pContext;
    Tooltips m_tooltips;
    QHash<QString, QDomElement> m_templateCache;
    static QList<const char*> s_channelStrs;
    static QMutex s_safeStringMutex;
};


#endif /* LEGACYSKINPARSER_H */
