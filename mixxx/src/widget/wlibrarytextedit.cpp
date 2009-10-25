// wlibrarytextedit.cpp
// Created 10/23/2009 by RJ Ryan (rryan@mit.edu)

#include "widget/wlibrarytextedit.h"

#include "widget/wwidget.h"
#include "widget/wskincolor.h"

WLibraryTextEdit::WLibraryTextEdit(QWidget* parent)
        : QTextEdit(parent) {

}

WLibraryTextEdit::~WLibraryTextEdit() {

}

void WLibraryTextEdit::setup(QDomNode node) {
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

void WLibraryTextEdit::onSearch(const QString& text) {

}

void WLibraryTextEdit::onSearchStarting() {

}

void WLibraryTextEdit::onSearchCleared() {

}
