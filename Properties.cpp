#include <QIcon>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QAbstractItemView>
#include <QDateTime>
#include <QTextEdit>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include "Pane.h"
#include "Properties.h"
#include "MainWindow.h"

Properties::Properties(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Properties"));
    setWindowIcon(QIcon(":/Images/Properties.ico"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setMargin(5);

    MainWindow* mainWindow(static_cast<MainWindow*>(parent));
    QWidget* focus(mainWindow->focusWidget());
    QAbstractItemView* view = nullptr; // Инициализируем nullptr

    // Проверяем, является ли фокус одним из представлений элементов
    if (focus == mainWindow->directoryTreeView ||
        focus == mainWindow->leftPane->treeView ||
        focus == mainWindow->leftPane->listView ||
        focus == mainWindow->rightPane->treeView ||
        focus == mainWindow->rightPane->listView)
    {
        view = qobject_cast<QAbstractItemView *>(focus);
    }

    // Инициализируем descriptionEdit и tagsEdit вне блока if
    // чтобы избежать потенциального неинициализированного доступа, если 'view' равен null.
    descriptionEdit = new QTextEdit();
    tagsEdit = new QLineEdit();

    if (view && view->selectionModel() && view->selectionModel()->selectedIndexes().count() > 0) {
        QFileInfo fileInfo = mainWindow->fileSystemModel->fileInfo(view->selectionModel()->selectedIndexes().at(0));
        currentFileInfo = fileInfo; // Сохраняем информацию о файле

        QLabel* nLabel = new QLabel(fileInfo.fileName());
        formLayout->addRow(tr("Имя:"), nLabel);
        QLabel* pLabel = new QLabel(fileInfo.path());
        formLayout->addRow(tr("Путь:"), pLabel);
        QLabel* sLabel = new QLabel(QString::number(fileInfo.size()) + " byte");
        formLayout->addRow(tr("Размер:"), sLabel);
        // Исправление для устаревшего 'created()'
        QLabel* cLabel = new QLabel(fileInfo.birthTime().toString()); // Используем birthTime()
        formLayout->addRow(tr("Создан:"), cLabel);
        QLabel* mLabel = new QLabel(fileInfo.lastModified().toString());
        formLayout->addRow(tr("Последнее изменение:"), mLabel);
        QLabel* rLabel = new QLabel(fileInfo.lastRead().toString());
        formLayout->addRow(tr("Последнее чтение:"), rLabel);
        QLabel* oLabel = new QLabel(fileInfo.owner());
        formLayout->addRow(tr("Другие:"), oLabel);
        QLabel* gLabel = new QLabel(fileInfo.group());
        formLayout->addRow(tr("Группа:"), gLabel);

        // Добавляем поля для описания и тегов
        descriptionEdit->setPlaceholderText(tr("Введите описание файла..."));
        tagsEdit->setPlaceholderText(tr("Введите теги через запятую..."));

        // Загружаем существующие метаданные из БД
        QSqlQuery query;
        query.prepare("SELECT description, tags FROM file_metadata WHERE path = :path");
        query.bindValue(":path", currentFileInfo.absoluteFilePath());
        if (query.exec()) {
            if (query.next()) {
                descriptionEdit->setText(query.value("description").toString());
                tagsEdit->setText(query.value("tags").toString());
            }
        } else {
            qDebug() << "Не удалось загрузить метаданные для" << currentFileInfo.absoluteFilePath() << ":" << query.lastError().text();
        }
    } else {
        // Если файл не выбран или фокус не на представлении, отключаем редактирование и показываем сообщение.
        descriptionEdit->setPlaceholderText(tr("Выберите файл для просмотра свойств."));
        descriptionEdit->setReadOnly(true);

        tagsEdit->setPlaceholderText(tr(""));
        tagsEdit->setReadOnly(true);
        // currentFileInfo будет по умолчанию недействительным, если файл не выбран.
        // Это нормально, так как метод accept() проверяет currentFileInfo.exists()
    }

    formLayout->addRow(tr("Описание:"), descriptionEdit);
    formLayout->addRow(tr("Теги:"), tagsEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void Properties::accept()
{
    // Сохраняем/обновляем метаданные в базе данных
    // currentFileInfo может быть недействительным, если при открытии диалога не был выбран файл.
    // Проверка .exists() гарантирует, что мы пытаемся сохранить только если был выбран действительный файл.
    if (currentFileInfo.exists()) {
        QSqlQuery query;
        // Используем ON CONFLICT для PostgreSQL для обработки INSERT и UPDATE
        query.prepare("INSERT INTO file_metadata (path, description, tags) "
                      "VALUES (:path, :description, :tags) "
                      "ON CONFLICT (path) DO UPDATE SET "
                      "description = EXCLUDED.description, "
                      "tags = EXCLUDED.tags");
        query.bindValue(":path", currentFileInfo.absoluteFilePath());
        query.bindValue(":description", descriptionEdit->toPlainText());
        query.bindValue(":tags", tagsEdit->text());

        if (!query.exec()) {
            QMessageBox::critical(this, tr("Ошибка сохранения метаданных"),
                                  tr("Не удалось сохранить метаданные файла: %1").arg(query.lastError().text()));
            // Исправлена опечатка: query.lastFileInfo.absoluteFilePath() -> currentFileInfo.absoluteFilePath()
            qDebug() << "Ошибка сохранения метаданных для" << currentFileInfo.absoluteFilePath() << ":" << query.lastError().text();
        } else {
            qDebug() << "Метаданные для" << currentFileInfo.absoluteFilePath() << "сохранены.";
        }
    }
    done(1);
}

void Properties::reject()
{
    done(0);
}
