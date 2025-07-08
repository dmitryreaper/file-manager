#include <QApplication>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include "MainWindow.h"

// Функция для инициализации базы данных
bool initDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName("file_manager_db");
    db.setUserName("dima");
    db.setPassword("zxc011");

    if (!db.open()) {
        // Исправлено: db.lastError() теперь имеет полный тип
        QMessageBox::critical(nullptr, QObject::tr("Ошибка подключения к базе данных"),
                              QObject::tr("Не удалось подключиться к базе данных PostgreSQL: %1").arg(db.lastError().text()));
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
        return false;
    }

    qDebug() << "Подключение к базе данных PostgreSQL успешно установлено.";

    // Создаем таблицу, если она не существует
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS file_metadata ("
                    "path TEXT PRIMARY KEY,"
                    "description TEXT,"
                    "tags TEXT" // Можно хранить теги как строку, разделенную запятыми, или как JSON
                    ")")) {
        // Исправлено: query.lastError() теперь имеет полный тип
        QMessageBox::critical(nullptr, QObject::tr("Ошибка создания таблицы"),
                              QObject::tr("Не удалось создать таблицу file_metadata: %1").arg(query.lastError().text()));
        qDebug() << "Ошибка создания таблицы:" << query.lastError().text();
        return false;
    }
    qDebug() << "Таблица 'file_metadata' проверена/создана.";
    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FILE MANAGER");
    app.setWindowIcon(QIcon(":/Images/file.ico"));

    // Инициализируем базу данных перед созданием главного окна
    if (!initDatabase()) {
        return 1; // Выходим, если не удалось подключиться к БД
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
