#ifndef LEGACYSKINPARSER_H
#define LEGACYSKINPARSER_H

#include <QObject>
#include <QString>
#include <QDomElement>

#include "configobject.h"
#include "skin/skinparser.h"

class Library;
class MixxxKeyboard;
class PlayerManager;

class LegacySkinParser : public QObject, public SkinParser {
    Q_OBJECT
  public:
    LegacySkinParser(ConfigObject<ConfigValue>* pConfig,
                     MixxxKeyboard* pKeyboard, PlayerManager* pPlayerManager,
                     Library* pLibrary);
    virtual ~LegacySkinParser();

    virtual bool canParse(QString skinPath);
    virtual QWidget* parseSkin(QString skinPath, QWidget* pParent);

    // Legacy support for looking up the scheme list.
    static QList<QString> getSchemeList(QString qSkinPath);
  private:
    static QDomElement openSkin(QString skinPath);

    QWidget* parseNode(QDomElement node, QWidget* pParent);
    void parseColorSchemes(QDomElement node);

    QWidget* parseBackground(QDomElement node, QWidget* pParent);
    QWidget* parsePushButton(QDomElement node, QWidget* pParent);
    QWidget* parseSliderComposed(QDomElement node, QWidget* pParent);
    QWidget* parseVisual(QDomElement node, QWidget* pParent);
    QWidget* parseOverview(QDomElement node, QWidget* pParent);
    QWidget* parseText(QDomElement node, QWidget* pParent);
    QWidget* parseVuMeter(QDomElement node, QWidget* pParent);
    QWidget* parseStatusLight(QDomElement node, QWidget* pParent);
    QWidget* parseDisplay(QDomElement node, QWidget* pParent);
    QWidget* parseNumberRate(QDomElement node, QWidget* pParent);
    QWidget* parseNumberPos(QDomElement node, QWidget* pParent);
    QWidget* parseNumberBpm(QDomElement node, QWidget* pParent);
    QWidget* parseNumber(QDomElement node, QWidget* pParent);
    QWidget* parseLabel(QDomElement node, QWidget* pParent);
    QWidget* parseKnob(QDomElement node, QWidget* pParent);
    QWidget* parseTableView(QDomElement node, QWidget* pParent);

    ConfigObject<ConfigValue>* m_pConfig;
    MixxxKeyboard* m_pKeyboard;
    PlayerManager* m_pPlayerManager;
    Library* m_pLibrary;
};


#endif /* LEGACYSKINPARSER_H */
