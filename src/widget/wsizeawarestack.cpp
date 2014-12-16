#include <QtDebug>
#include <QStackedLayout>
#include <QResizeEvent>

#include "widget/wsizeawarestack.h"

class SizeAwareLayout : public QStackedLayout
{
  public:
    QSize minimumSize() const
    {
        QSize s(0, 0) ;
        QWidget *w = widget(0);
        if (w) {
            // Minimum Widget is at index 0;
            s = w->minimumSize();
        }
        return s;
    }

    int setCurrentIndexForSize(const QSize& s) {
        int n = count();
        if (n <= 0) {
            return -1;
        }

        int i = currentIndex();

        QWidget *wc = widget(i);
        bool notFit = false;
        if (i > 0) {
            // Check minimum, but not for the smallest, it is the fallback
            notFit =  wc->minimumHeight() > s.height() || wc->minimumWidth() > s.width();
        }
        if (i < n - 1 && !notFit) {
            // Check maximum, but not for the biggest, it is the fallback
            notFit = wc->maximumHeight() < s.height() || wc->maximumWidth() < s.width();
        }

        if (notFit) {
            QWidget *w;
            for (i = 0; i < n; ++i) {
                w = widget(i);
                if (w) {
                    if (w->maximumHeight() >= s.height() && w->maximumWidth() >= s.width() &&
                            w->minimumHeight() <= s.height() && w->minimumWidth() <= s.width()) {
                        // perfectly fit
                        setCurrentIndex(i);
                        return i;
                    }
                }
            }
            // no perfect fit, check minimum only to avoid chopping
            for (i = n-1; i >= 0; --i) {
                w = widget(i);
                if (w) {
                    if (w->minimumHeight() <= s.height() && w->minimumWidth() <= s.width()) {
                        // fit with gap
                        setCurrentIndex(i);
                        return i;
                    }
                }
            }
            // fallback: take smallest
            setCurrentIndex(0);
            return 0;
        }

        return i;
    }
};

WSizeAwareStack::WSizeAwareStack(QWidget* parent)
        : QWidget(parent),
          WBaseWidget(this) {
    m_layout = new SizeAwareLayout();
    setLayout(m_layout);
}

WSizeAwareStack::~WSizeAwareStack() {
}

int WSizeAwareStack::addWidget(QWidget *widget) {
    // smallest widgets should be added first
    return m_layout->addWidget(widget);
}

bool WSizeAwareStack::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QWidget::event(pEvent);
}

void WSizeAwareStack::resizeEvent(QResizeEvent* event) {
    m_layout->setCurrentIndexForSize(event->size());
}
