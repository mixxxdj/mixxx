#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent),
          ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Populate fieldComboBox
    ui->fieldComboBox->addItems({"Artist", "Album", "Genre"});

    // Populate conditionComboBox
    ui->conditionComboBox->addItems({"contains", "is", "starts with"});

    // Connect buttons to slots
    connect(ui->addConditionButton,
            &QPushButton::clicked,
            this,
            &MainWindow::on_addConditionButton_clicked);
    connect(ui->generatePlaylistButton,
            &QPushButton::clicked,
            this,
            &MainWindow::on_generatePlaylistButton_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addConditionButton_clicked() {
    QString field = ui->fieldComboBox->currentText();
    QString condition = ui->conditionComboBox->currentText();
    QString value = ui->valueLineEdit->text();

    QString conditionText = QString("%1 %2 %3").arg(field, condition, value);
    ui->conditionsListWidget->addItem(conditionText);

    ui->valueLineEdit->clear();
}

void MainWindow::on_generatePlaylistButton_clicked() {
    QStringList conditions;
    for (int i = 0; i < ui->conditionsListWidget->count(); ++i) {
        conditions << ui->conditionsListWidget->item(i)->text();
    }

    // Here you would implement the logic to generate the playlist based on the conditions
    // For now, we'll just print the conditions to the console
    foreach (const QString& condition, conditions) {
        qDebug() << condition;
    }
}
