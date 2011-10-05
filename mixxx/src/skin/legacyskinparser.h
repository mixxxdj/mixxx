#ifndef LEGACYSKINPARSER_H
#define LEGACYSKINPARSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDomElement>
#include <QMutex>

#include "configobject.h"
#include "skin/skinparser.h"
#include "vinylcontrol/vinylcontrolmanager.h"

class Library;
class MixxxKeyboard;
class PlayerManager;
class WAbstractControl;

class LegacySkinParser : public QObject, public SkinParser {
    Q_OBJECT
  public:
    LegacySkinParser(ConfigObject<ConfigValue>* pConfig,
                     MixxxKeyboard* pKeyboard, PlayerManager* pPlayerManager,
                     Library* pLibrary, VinylControlManager* pVCMan);
    virtual ~LegacySkinParser();

    virtual bool canParse(QString skinPath);
    virtual QWidget* parseSkin(QString skinPath, QWidget* pParent);

    // Legacy support for looking up the scheme list.
    static QList<QString> getSchemeList(QString qSkinPath);
    static void freeChannelStrings();
  private:
    static QDomElement openSkin(QString skinPath);

    QWidget* parseNode(QDomElement node, QWidget* pGrandparent);

    // Support for various legacy behavior
    void parseColorSchemes(QDomElement node);
    void setControlDefaults(QDomNode node, WAbstractControl* pControl);
    bool compareConfigKeys(QDomNode node, QString key);

    // Parsers for each node
    QWidget* parseWidgetGroup(QDomElement node);
    QWidget* parseBackground(QDomElement node, QWidget* pGrandparent);
    QWidget* parsePushButton(QDomElement node);
    QWidget* parseSliderComposed(QDomElement node);
    QWidget* parseVisual(QDomElement node);
    QWidget* parseOverview(QDomElement node);
    QWidget* parseText(QDomElement node);
    QWidget* parseTrackProperty(QDomElement node);
    QWidget* parseVuMeter(QDomElement node);
    QWidget* parseStatusLight(QDomElement node);
    QWidget* parseDisplay(QDomElement node);
    QWidget* parseNumberRate(QDomElement node);
    QWidget* parseNumberPos(QDomElement node);
    QWidget* parseNumberBpm(QDomElement node);
    QWidget* parseNumber(QDomElement node);
    QWidget* parseLabel(QDomElement node);
    QWidget* parseKnob(QDomElement node);
    QWidget* parseTableView(QDomElement node);
    QWidget* parseStyle(QDomElement node);
    QWidget* parseSpinny(QDomElement node);

    void setupPosition(QDomNode node, QWidget* pWidget);
    void setupSize(QDomNode node, QWidget* pWidget);
    void setupWidget(QDomNode node, QWidget* pWidget);
    void setupConnections(QDomNode node, QWidget* pWidget);

    QString lookupNodeGroup(QDomElement node);
    static const char* safeChannelString(QString channelStr);

    ConfigObject<ConfigValue>* m_pConfig;
    MixxxKeyboard* m_pKeyboard;
    PlayerManager* m_pPlayerManager;
    Library* m_pLibrary;
    VinylControlManager* m_pVCManager;
    QWidget *m_pParent;
    static QList<const char*> s_channelStrs;
    static QMutex s_safeStringMutex;
};


#endif /* LEGACYSKINPARSER_H */
