#pragma once

#include <QCompleter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QStringList>
#include <QStringListModel>
#include <QWidget>

#include "control/controlproxy.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

// Forward declarations
class TrackCollection;
class GenreDao;

class WGenreTagInput : public QWidget {
    Q_OBJECT

  public:
    explicit WGenreTagInput(QWidget* parent = nullptr);
    ~WGenreTagInput() override = default;

    void setTrack(TrackPointer pTrack);
    QStringList getGenres() const;
    void setGenres(const QStringList& genres);

    // Multi-track mode
    void setMultiTrackMode(bool multiTrack);
    bool isMultiTrackMode() const;

    // Auto-save functionality
    void setAutoSave(bool autoSave);
    bool isAutoSave() const;

    // Set the genre completer with a list of genres
    void setGenreCompleter(const QStringList& genres);

    void debugState() const;

    // Enable/disable editing
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    // Autocomplete support
    void setCompleterModel(QStringListModel* model);

    // Database integration
    void setTrackCollection(TrackCollection* pTrackCollection);
    void loadGenresFromTrack();
    void saveGenresToTrack();

  public slots:
    void addGenre(const QString& genre);
    void removeGenre(const QString& genre);
    void clear();

  signals:
    void genresChanged();
    void editingFinished();

  protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

    // Drag & Drop events
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

  private slots:
    void slotTrackGenresChanged(const QStringList& genres);
    void slotAddGenre();
    void slotRemoveGenre();
    void slotLineEditFinished();
    void slotCompleterActivated(const QString& text);

  private:
    void setupUI();
    void setupGenreCompleter();
    void updateGenreTags();
    void enterEditMode();
    void exitEditMode();
    QString formatGenresForDisplay() const;

    QWidget* createGenreTag(const QString& genre);

    TrackPointer m_pTrack;
    QStringList m_genres;

    // Database integration
    TrackCollection* m_pTrackCollection;

    bool m_readOnly;
    bool m_editMode;

    // Multi-track mode
    bool m_multiTrackMode;
    bool m_autoSave;

    // UI components
    QHBoxLayout* m_pMainLayout;
    QScrollArea* m_pScrollArea;
    QWidget* m_pTagContainer;
    QHBoxLayout* m_pTagLayout;
    QLineEdit* m_pLineEdit;
    QPushButton* m_pAddButton;
    QLabel* m_pDisplayLabel;

    // Autocomplete
    QCompleter* m_pCompleter;
    QStringListModel* m_pCompleterModel;

    // Drag & Drop state
    QPoint m_dragStartPosition;
    QString m_draggedGenre;
    bool m_isDragging;

    Q_DISABLE_COPY(WGenreTagInput)
};
