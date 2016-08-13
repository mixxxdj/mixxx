#include "wwidget.h"
#include "wskincolor.h"
#include "wsearchlineedit.h"

#include <QAction>
#include <QDebug>
#include <QFont>
#include <QMenu>
#include <QShortcut>
#include <QStyle>

WSearchLineEdit::WSearchLineEdit(QWidget* pParent)
        : QLineEdit(pParent),
          WBaseWidget(this) {
    setAcceptDrops(false);
    
    QSize iconSize(height()/2, height()/2);
    
    m_pClearButton = new QToolButton(this);
    m_pClearButton->setIcon(QIcon(":/skins/cross_2.png"));
    m_pClearButton->setIconSize(iconSize);
    m_pClearButton->setCursor(Qt::ArrowCursor);
    m_pClearButton->setToolTip(tr("Clear input" , "Clear the search bar input field"));
    m_pClearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_pClearButton->hide();
    
    m_pSaveButton = new QToolButton(this);
    m_pSaveButton->setIcon(QIcon(":/skins/save.png"));
    m_pSaveButton->setIconSize(iconSize);
    m_pSaveButton->setCursor(Qt::ArrowCursor);
    m_pSaveButton->setToolTip(tr("Save query", "Save the current query for later use"));
    m_pSaveButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_pSaveButton->hide();
    
    m_pDropButton = new QToolButton(this);
    m_pDropButton->setIcon(QIcon(":/skins/downArrow.png"));
    m_pDropButton->setIconSize(iconSize);
    m_pDropButton->setCursor(Qt::ArrowCursor);
    m_pDropButton->setToolTip(tr("Restore query", 
                                 "Restore the search with one of the selected queries"));
    m_pDropButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_pDropButton->hide();

    m_place = true;
    showPlaceholder();

    setFocusPolicy(Qt::ClickFocus);
    QShortcut* setFocusShortcut = new QShortcut(
        QKeySequence(tr("Ctrl+F", "Search|Focus")), this);
    connect(setFocusShortcut, SIGNAL(activated()),
            this, SLOT(setFocus()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged(const QString&)));

    // Set up a timer to search after a few hundred milliseconds timeout.  This
    // stops us from thrashing the database if you type really fast.
    m_searchTimer.setSingleShot(true);
    connect(&m_searchTimer, SIGNAL(timeout()),
            this, SLOT(triggerSearch()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSetupTimer(const QString&)));

    // When you hit enter, it will trigger the search.
    connect(this, SIGNAL(returnPressed()),
            this, SLOT(triggerSearch()));

    connect(m_pClearButton, SIGNAL(clicked()),
            this, SLOT(onSearchTextCleared()));
    // Forces immediate update of tracktable
    connect(m_pClearButton, SIGNAL(clicked()),
            this, SLOT(triggerSearch()));
    
    connect(m_pSaveButton, SIGNAL(clicked()),
            this, SLOT(saveQuery()));
    connect(m_pDropButton, SIGNAL(clicked()),
            this, SLOT(restoreQuery()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(updateButtons(const QString&)));

    // The width of the frame for the widget based on the styling.
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    // Ensures the text does not obscure the clear image.
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").
                  arg(m_pClearButton->sizeHint().width() + frameWidth + 1));
}

void WSearchLineEdit::setup(const QDomNode& node, const SkinContext& context) {
    // Background color
    QColor bgc(255,255,255);
    QString bgColorStr;
    if (context.hasNodeSelectString(node, "BgColor", &bgColorStr)) {
        bgc.setNamedColor(bgColorStr);
        setAutoFillBackground(true);
    }
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), WSkinColor::getCorrectColor(bgc));

    // Foreground color
    m_fgc = QColor(0,0,0);
    QString fgColorStr;
    if (context.hasNodeSelectString(node, "FgColor", &fgColorStr)) {
        m_fgc.setNamedColor(fgColorStr);
    }
    bgc = WSkinColor::getCorrectColor(bgc);
    m_fgc = QColor(255 - bgc.red(), 255 - bgc.green(), 255 - bgc.blue());
    pal.setBrush(foregroundRole(), m_fgc);
    setPalette(pal);
}

void WSearchLineEdit::resizeEvent(QResizeEvent* e) {
    QLineEdit::resizeEvent(e);
    
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    
    QSize iconSize(height()/2, height()/2);
    m_pDropButton->setIconSize(iconSize);
    m_pSaveButton->setIconSize(iconSize);
    m_pClearButton->setIconSize(iconSize);
    
    const QSize sizeDrop = m_pDropButton->sizeHint();
    const QSize sizeSave = m_pSaveButton->sizeHint();
    const QSize sizeClear = m_pClearButton->sizeHint();
    const int space = 1; // 1px of space between items
    
    int posXDrop = frameWidth + 1;
    int posXSave = posXDrop + sizeDrop.width() + space;
    int posXClear = posXSave + sizeSave.width() + space;
    
    int posYDrop = (height() - sizeDrop.height())/2;
    int posYSave = (height() - sizeSave.height())/2;
    int posYClear = (height() - sizeClear.height())/2;
    
    if (layoutDirection() == Qt::LeftToRight) {
        posXDrop += sizeDrop.width();
        posXSave += sizeDrop.width();
        posXClear += sizeDrop.width();
        
        m_pDropButton->move(width() - posXDrop, posYDrop);
        m_pSaveButton->move(width() - posXSave, posYSave);
        m_pClearButton->move(width() - posXClear, posYClear);
    } else {
        m_pDropButton->move(posXDrop, posYDrop);
        m_pSaveButton->move(posXSave, posYSave);
        m_pClearButton->move(posXClear, posYClear);
    }
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    if (m_place) {
        // This gets rid of the blue mac highlight.
        setAttribute(Qt::WA_MacShowFocusRect, false);
        //Must block signals here so that we don't emit a search() signal via
        //textChanged().
        blockSignals(true);
        setText("");
        blockSignals(false);
        QPalette pal = palette();
        pal.setColor(foregroundRole(), m_fgc);
        setPalette(pal);
        m_place = false;
        emit(searchStarting());
    }
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    if (text().isEmpty()) {
        m_place = true;
        showPlaceholder();
        emit(searchCleared());
    } else {
        m_place = false;
    }
}

// slot
void WSearchLineEdit::restoreSearch(const QString& text, QPointer<LibraryFeature> pFeature) {
    if(text.isNull()) {
        // disable
        setEnabled(false);
        blockSignals(true);
        setText("- - -");
        blockSignals(false);
        m_pSaveButton->hide();
        m_pDropButton->hide();
        m_pCurrentFeature = nullptr;
        return;
    }
    setEnabled(true);
    m_pCurrentFeature = pFeature;
    qDebug() << "WSearchLineEdit::restoreSearch(" << text << ")";
    blockSignals(true);
    setText(text);
    blockSignals(false);
    if (text == "") {
        m_place = true;
        showPlaceholder();
    } else {
        QPalette pal = palette();
        pal.setColor(foregroundRole(), m_fgc);
        setPalette(pal);
        m_place = false;
    }
    updateButtons(text);
}

void WSearchLineEdit::slotSetupTimer(const QString& text)
{
    Q_UNUSED(text);
    m_searchTimer.stop();
    //300 milliseconds timeout
    m_searchTimer.start(300);
}

void WSearchLineEdit::triggerSearch()
{
    m_searchTimer.stop();
    emit(search(text()));
}

void WSearchLineEdit::showPlaceholder() {
    //Must block signals here so that we don't emit a search() signal via
    //textChanged().
    blockSignals(true);
    setText(tr("Search..." , "noun"));
    setToolTip(tr("Search" , "noun") + "\n" + tr("Enter a string to search for") + "\n\n"
                  + tr("Shortcut")+ ": \n"
                  + tr("Ctrl+F") + "  " + tr("Focus" , "Give search bar input focus") + "\n"
                  + tr("Ctrl+Backspace") + "  "+ tr("Clear input" , "Clear the search bar input field") + "\n"
                  + tr("Esc") + "  " + tr("Exit search" , "Exit search bar and leave focus")
                  );
    blockSignals(false);
    QPalette pal = palette();
    pal.setColor(foregroundRole(), Qt::lightGray);
    setPalette(pal);
}

void WSearchLineEdit::updateButtons(const QString& text)
{
    bool visible = !text.isEmpty() && !m_place;
    m_pDropButton->show();
    m_pSaveButton->setVisible(visible);
    m_pClearButton->setVisible(visible);
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QLineEdit::event(pEvent);
}

void WSearchLineEdit::onSearchTextCleared() {
    QLineEdit::clear();
    emit(searchCleared());
}

void WSearchLineEdit::saveQuery() {
    SavedSearchQuery query;
    query.title = query.query = text();
    if (!m_pCurrentFeature.isNull()) {
        m_pCurrentFeature->saveQuery(query);
    }
    setText("");
}

void WSearchLineEdit::restoreQuery() {
    if (m_pCurrentFeature.isNull()) {
        return;
    }
    
    const QList<SavedSearchQuery>& savedQueries = m_pCurrentFeature->getSavedQueries();
    
    QMenu menu;
    
    if (savedQueries.size() <= 0) {
        QAction* action = menu.addAction(tr("No saved queries"));
        action->setData(-1);
    }
    for (int i = 0; i < savedQueries.size(); ++i) {
        QAction* action = menu.addAction(savedQueries[i].title);
        action->setData(i);
    }
    QPoint position = m_pDropButton->pos();
    position += QPoint(0, m_pDropButton->height());
    
    QAction* selected = menu.exec(mapToGlobal(position));
    if (selected == nullptr) {
        return;
    }
    
    int index = selected->data().toInt();
    if (index < 0) {
        return;
    }
    QString text = savedQueries[index].query;
    blockSignals(true);
    setText(text);
    updateButtons(text);
    blockSignals(false);    
    
    m_pCurrentFeature->restoreQuery(index);
}

void WSearchLineEdit::slotTextChanged(const QString& text) {
    if (text.isEmpty()) {
        triggerSearch();
        emit(searchCleared());
    }
}
