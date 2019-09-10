#pragma once
#include "skin/skincontext.h"

// These classes provide a way for skins to specify pixmap paths for the
// icons in complex widgets, for example the buttons in DlgAutoDJ. The skin XML
// for the widget would have a Buttons element like:
//
// <Buttons>
//   <Button name="foo">
//     <State name="On">on_graphic.svg</State>
//     <State name="Off">off_graphic.svg</State>
//   </Button>
// </Buttons>
//
// Call ButtonIcon::parseIcons in the widget's setup method and store the result
// in a member variable.

class SkinButtonState {
  public:
    QString name;
    PixmapSource pixmapSource;
};

class SkinButton {
  public:
    QString name;
    QMap<QString, SkinButtonState> states;

    void addState(SkinButtonState state) {
        states.insert(state.name, state);
    }

    static QMap<QString, SkinButton> parseIcons(
        const QDomNode& node, const SkinContext& context) {

        QMap<QString, SkinButton> icons;
        QDomElement buttonIconsList = context.selectElement(node, "Buttons");
        QDomNode buttonIconNode = context.selectNode(buttonIconsList, "Button");
        while (!buttonIconNode.isNull()) {
            if (buttonIconNode.isElement() && buttonIconNode.nodeName() == "Button") {
                SkinButton buttonIcon;
                QString buttonName = buttonIconNode.toElement().attribute("name");
                if (buttonName.isEmpty()) {
                    buttonIconNode = buttonIconNode.nextSibling();
                    continue;
                }
                buttonIcon.name = buttonName;

                QDomNode buttonStateNode = context.selectNode(buttonIconNode, "State");
                while (!buttonStateNode.isNull()) {
                    if (buttonStateNode.isElement() && buttonStateNode.nodeName() == "State") {
                        SkinButtonState buttonIconState;
                        QString buttonIconStateName = buttonStateNode.toElement().attribute("name");
                        if (!buttonIconStateName.isEmpty()) {
                            buttonIconState.name = buttonIconStateName;
                            buttonIconState.pixmapSource = context.getPixmapSource(buttonStateNode);
                            buttonIcon.addState(buttonIconState);
                        }
                    }
                    buttonStateNode = buttonStateNode.nextSibling();
                }

                icons.insert(buttonName, buttonIcon);
            }
            buttonIconNode = buttonIconNode.nextSibling();
        }
        return icons;
    }
};
