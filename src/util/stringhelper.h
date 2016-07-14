#ifndef STRINGHELPER_H
#define STRINGHELPER_H

class StringHelper
{
  public:
    StringHelper();
    
    static QChar getFirstCharForGrouping(const QString &text);
};

#endif // STRINGHELPER_H
