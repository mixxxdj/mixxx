#ifndef WSCROLLAREA_H
#define WSCROLLAREA_H
#include <QScrollArea>

#include <library/libraryview.h>

class WScrollArea : public QScrollArea, public LibraryView
{
public:
    WScrollArea(QWidget* parent = nullptr);
    
    void onShow() {}
};

#endif // WSCROLLAREA_H
