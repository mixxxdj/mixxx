#ifndef SKINPARSER_H
#define SKINPARSER_H

#include <QString>
#include <QWidget>

class SkinParser {
  public:
    SkinParser() { }
    virtual ~SkinParser() { }

    virtual bool canParse(const QString& skinPath) = 0;
    virtual QWidget* parseSkin(const QString& skinPath, QWidget* pParent) = 0;
};

#endif /* SKINPARSER_H */
