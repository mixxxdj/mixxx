#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QWidgetAction>

#include "util/parented_ptr.h"
#include "widget/wcolorpicker.h"

class WColorPickerAction : public QWidgetAction {
  public:
    WColorPickerAction(QWidget* parent)
            : QWidgetAction(parent) {
        m_pColorPicker = make_parented<WColorPicker>(true);

        QHBoxLayout* pLayout = new QHBoxLayout();
        pLayout->addWidget(m_pColorPicker);

        QWidget* pWidget = new QWidget();
        pWidget->setLayout(pLayout);
        setDefaultWidget(pWidget);
    }

    WColorPicker* colorPicker() {
        return m_pColorPicker.get();
    }

  private:
    parented_ptr<WColorPicker> m_pColorPicker;
};
