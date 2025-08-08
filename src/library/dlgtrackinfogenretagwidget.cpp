
#include "library/dlgtrackinfogenretagwidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

#include "moc_dlgtrackinfogenretagwidget.cpp"

GenreTagWidget::GenreTagWidget(QWidget* parent)
        : QWidget(parent),
          m_layout(new QHBoxLayout(this)),
          m_input(new QLineEdit(this)) {
    m_layout->setSpacing(5);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->addWidget(m_input);
    connect(m_input, &QLineEdit::returnPressed, this, &GenreTagWidget::onTagEntered);
}

void GenreTagWidget::setTags(const QStringList& tags) {
    m_tags.clear();
    QLayoutItem* child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    for (const QString& tag : tags) {
        addTag(tag);
    }
    m_layout->addWidget(m_input);
}

QStringList GenreTagWidget::tags() const {
    return m_tags;
}

void GenreTagWidget::onTagEntered() {
    QString text = m_input->text().trimmed();
    if (!text.isEmpty() && !m_tags.contains(text)) {
        addTag(text);
        m_tags << text;
        emit tagsChanged(m_tags);
    }
    m_input->clear();
}

void GenreTagWidget::addTag(const QString& tagText) {
    QWidget* tag = new QWidget(this);
    QHBoxLayout* tagLayout = new QHBoxLayout(tag);
    tagLayout->setContentsMargins(5, 0, 5, 0);

    QLabel* label = new QLabel(tagText, tag);
    QPushButton* close = new QPushButton("x", tag);
    close->setFixedSize(16, 16);
    close->setStyleSheet("QPushButton { border: none; background: transparent; }");

    tagLayout->addWidget(label);
    tagLayout->addWidget(close);

    tag->setStyleSheet("QWidget { background-color: lightgray; border-radius: 8px; }");

    connect(close, &QPushButton::clicked, this, [this, tagText, tag]() {
        m_tags.removeAll(tagText);
        m_layout->removeWidget(tag);
        tag->deleteLater();
        emit tagsChanged(m_tags);
    });

    m_layout->insertWidget(m_layout->count() - 1, tag);
}
