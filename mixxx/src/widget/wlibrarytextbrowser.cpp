// wlibrarytextbrowser.cpp
// Created 10/23/2009 by RJ Ryan (rryan@mit.edu)

#include "widget/wlibrarytextbrowser.h"

#include "widget/wwidget.h"
#include "widget/wskincolor.h"

WLibraryTextBrowser::WLibraryTextBrowser(QWidget* parent)
        : QTextBrowser(parent) {

}

WLibraryTextBrowser::~WLibraryTextBrowser() {

}

void WLibraryTextBrowser::setup(QDomNode node) {
    QPalette pal = palette();
    if (!WWidget::selectNode(node, "BgColor").isNull()) {
        QString bgColor = WWidget::selectNodeQString(node, "BgColor");
        QColor bg;
        bg.setNamedColor(bgColor);
        bg = WSkinColor::getCorrectColor(bg);
        pal.setColor(QPalette::Base, bg);
    }
    if (!WWidget::selectNode(node, "FgColor").isNull()) {
        QString fgColor = WWidget::selectNodeQString(node, "FgColor");
        QColor fg;
        fg.setNamedColor(fgColor);
        fg = WSkinColor::getCorrectColor(fg);
        pal.setColor(QPalette::Text, fg);
    }
    setPalette(pal);
}

void WLibraryTextBrowser::onSearch(const QString& text) {

}

void WLibraryTextBrowser::onSearchStarting() {

}

void WLibraryTextBrowser::onSearchCleared() {

}

void WLibraryTextBrowser::onShow() {

}

void WLibraryTextBrowser::loadSelectedTrack() {
    // Not applicable to text views
}

void WLibraryTextBrowser::loadSelectedTrackToGroup(QString group) {
    // Not applicable to text views
}

void WLibraryTextBrowser::moveSelection(int delta) {
    // Not applicable to text views
}
