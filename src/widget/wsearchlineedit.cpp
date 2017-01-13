#include <QAction>
#include <QDebug>
#include <QFont>
#include <QInputDialog>
#include <QMenu>
#include <QString>
#include <QStyle>
#include <QStyleOption>

#include "dialog/savedqueries/dlgsavedquerieseditor.h"
#include "widget/wsearchlineedit.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WSearchLineEdit::WSearchLineEdit(QWidget* pParent)
        : QLineEdit(pParent),
          WBaseWidget(this) {
    setAcceptDrops(false);
    
    QSize iconSize(height() / 2, height() / 2);
    
    m_pClearButton = new QToolButton(this);
    m_pClearButton->setIcon(QIcon(":/skins/cross_2.png"));
    m_pClearButton->setIconSize(iconSize);
    m_pClearButton->setCursor(Qt::ArrowCursor);
    m_pClearButton->setFocusPolicy(Qt::ClickFocus);
    m_pClearButton->setToolTip(tr("Clear input" , "Clear the search bar input field"));
    m_pClearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_pClearButton->hide();
    
    m_pSaveButton = new QToolButton(this);
    QIcon saveIcon;
    saveIcon.addPixmap(QPixmap(":/skins/save.png"), QIcon::Active, QIcon::Off);
    saveIcon.addPixmap(QPixmap(":/skins/save_disabled.png"), QIcon::Disabled, QIcon::Off);
    saveIcon.addPixmap(QPixmap(":/skins/save.png"), QIcon::Active, QIcon::On);
    saveIcon.addPixmap(QPixmap(":/skins/save_disabled.png"), QIcon::Disabled, QIcon::On);
    
    m_pSaveButton->setIcon(saveIcon);
    m_pSaveButton->setIconSize(iconSize);
    m_pSaveButton->setCursor(Qt::ArrowCursor);
    m_pSaveButton->setFocusPolicy(Qt::ClickFocus);
    m_pSaveButton->setToolTip(tr("Save query", "Save the current query for later use"));
    m_pSaveButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_pSaveButton->hide();
    
    m_pDropButton = new QToolButton(this);
    m_pDropButton->setIcon(QIcon(":/skins/downArrow.png"));
    m_pDropButton->setIconSize(iconSize);
    m_pDropButton->setCursor(Qt::ArrowCursor);
    m_pDropButton->setFocusPolicy(Qt::ClickFocus);
    m_pDropButton->setToolTip(tr("Restore query", 
                                 "Restore the search with one of the selected queries"));
    m_pDropButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_pDropButton->hide();

    m_place = true;
    showPlaceholder();

    setFocusPolicy(Qt::ClickFocus);

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

void WSearchLineEdit::setTrackCollection(TrackCollection* pTrackCollection) {
    m_pTrackCollection = pTrackCollection;
}

void WSearchLineEdit::slotRestoreSaveButton() {
    m_pSaveButton->setEnabled(true);
}

void WSearchLineEdit::resizeEvent(QResizeEvent* event) {
    QLineEdit::resizeEvent(event);
    
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    
    QStyleOption option;
    option.initFrom(this);
    const QRect contentsRect = 
            style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
    
    const int Ydeviation = contentsRect.top();
    const int height = contentsRect.height();
    const int fontHeight = fontMetrics().height();
    QSize iconSize(fontHeight, fontHeight);
    m_pDropButton->resize(iconSize);
    m_pSaveButton->resize(iconSize);
    m_pClearButton->resize(iconSize);
    
    m_pDropButton->setIconSize(iconSize);
    m_pSaveButton->setIconSize(iconSize);
    m_pClearButton->setIconSize(iconSize);
    
    const QSize sizeDrop = m_pDropButton->size();
    const QSize sizeSave = m_pSaveButton->size();
    const QSize sizeClear = m_pClearButton->size();
    const int space = 2; // 1px of space between items
    
    int posXDrop = frameWidth + 1;
    int posXSave = posXDrop + sizeDrop.width() + space;
    int posXClear = posXSave + sizeSave.width() + space;
    
    int posYDrop = (height - sizeDrop.height()) / 2 + Ydeviation;
    int posYSave = (height - sizeSave.height()) / 2 + Ydeviation;
    int posYClear = (height - sizeClear.height()) / 2 + Ydeviation;
    
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
    
    // Ensures the text does not obscure the clear image.
    setStyleSheet(QString("QLineEdit { padding-right: %1px; }").arg(posXClear));
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    if (m_place) {
        // This gets rid of the blue mac highlight.
        setAttribute(Qt::WA_MacShowFocusRect, false);
        // Must block signals here so that we don't emit a search() signal via
        // textChanged().
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

void WSearchLineEdit::keyPressEvent(QKeyEvent* event) {
    switch(event->key())
    {
    case Qt::Key_Escape:
        emit cancel();
        break;
    default:
        QLineEdit::keyPressEvent(event);
    }
}

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

void WSearchLineEdit::slotSetupTimer(const QString&) {
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
    // Must block signals here so that we don't emit a search() signal via
    // textChanged().
    blockSignals(true);
    setText(tr("Search..." , "noun"));
    setToolTip(tr("Search" , "noun") + "\n" + tr("Enter a string to search for") + "\n\n"
                  + tr("Shortcut")+ ": \n"
                  + tr("Ctrl+F") + "  " + tr("Focus" , "Give search bar input focus") + "\n"
                  + tr("Ctrl+Backspace") + "  "+ tr("Clear input" , "Clear the search bar input field") + "\n"
                  + tr("Esc") + "  " + tr("Exit search" , "Exit search bar and leave focus")
                  );
    blockSignals(false);
}

void WSearchLineEdit::updateButtons(const QString& text) {
    bool visible = !text.isEmpty() && !m_place;
    m_pDropButton->setVisible(true);
    m_pSaveButton->setVisible(true);
    m_pSaveButton->setEnabled(true);
    m_pClearButton->setVisible(visible);
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::FocusIn) {
        emit(focused());
    }
    return QLineEdit::event(pEvent);
}

void WSearchLineEdit::onSearchTextCleared() {
    QLineEdit::clear();
    emit(searchCleared());
}

void WSearchLineEdit::saveQuery() {
    if (m_pCurrentFeature.isNull()) {
        return;
    }
    
    SavedSearchQuery query;
    query.title = query.query = text();
    
    if (query.title.isEmpty()) {
        // Request a title
        bool ok = false;
        query.title = 
            QInputDialog::getText(nullptr,
                                  tr("Create search query"),
                                  tr("Enter name for empty search query"),
                                  QLineEdit::Normal,
                                  tr("New query"),
                                  &ok).trimmed();
        if (ok == false) {
            return;
        }
    }
        
    m_pCurrentFeature->saveQuery(query);
    m_pSaveButton->setEnabled(false);
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
    QActionGroup* group = new QActionGroup(&menu);
    group->setExclusive(false);
    
    for (const SavedSearchQuery& sQuery : savedQueries) {
        QAction* action = menu.addAction(sQuery.title);
        if (sQuery.pinned) {
            action->setActionGroup(group);
            
            action->setIcon(QIcon(":/images/ic_library_pinned.png"));
            action->setIconVisibleInMenu(true);
            action->setCheckable(true);
            action->setChecked(true);
        }
        action->setData(sQuery.id);
    }
    
    // There's no need to show the queries editor if there are not saved queries
    if (savedQueries.size() > 0) {
        menu.addSeparator();
        QAction* action = menu.addAction(tr("Queries editor"));
        action->setData(-2);
    }
    
    QPoint position = m_pDropButton->pos();
    position += QPoint(0, m_pDropButton->height());
    
    QAction* selected = menu.exec(mapToGlobal(position));
    if (selected == nullptr) return;
    
    bool ok;
    
    // index >= 0  -> Normal data
    // index == -1 -> No saved queries selected
    // index == -2 -> Saved queries editor selected
    int index = selected->data().toInt(&ok);
    if (!ok) return;
    
    if (index >= 0) {
        m_pCurrentFeature->restoreQuery(index);
    } else if (index == -2 && !m_pTrackCollection.isNull()) {
        // If we don't pass a nullptr as parent it uses the parent's style sheet
        // and the table shown is weird
        DlgSavedQueriesEditor editor(m_pCurrentFeature, 
                                     m_pTrackCollection, nullptr);
        editor.exec();
    }
}

void WSearchLineEdit::slotTextChanged(const QString& text) {
    setToolTip(text);
    if (text.isEmpty()) {
        triggerSearch();
        emit(searchCleared());
    }
}
