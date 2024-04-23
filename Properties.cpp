#include <QIcon>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QAbstractItemView>
#include <QLabel>
#include <QDateTime>
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
    QAbstractItemView* view;
    if (focus == mainWindow->directoryTreeView || focus == mainWindow->leftPane->treeView || focus == mainWindow->leftPane->listView || focus == mainWindow->rightPane->treeView || focus == mainWindow->rightPane->listView){
        view =  qobject_cast<QAbstractItemView *>(focus);
        QFileInfo fileInfo = mainWindow->fileSystemModel->fileInfo(view->selectionModel()->selectedIndexes().at(0));
        QLabel* nLabel = new QLabel(fileInfo.fileName());
        formLayout->addRow(tr("Имя:"), nLabel);
        QLabel* pLabel = new QLabel(fileInfo.path());
        formLayout->addRow(tr("Путь:"), pLabel);
        QLabel* sLabel = new QLabel(QString::number(fileInfo.size()) + " byte");
        formLayout->addRow(tr("Размер:"), sLabel);
        QLabel* cLabel = new QLabel(fileInfo.created().toString());
        formLayout->addRow(tr("Создан:"), cLabel);
        QLabel* mLabel = new QLabel(fileInfo.lastModified().toString());
        formLayout->addRow(tr("Последнее изменение:"), mLabel);
        QLabel* rLabel = new QLabel(fileInfo.lastRead().toString());
        formLayout->addRow(tr("Последнее чтение:"), rLabel);
        QLabel* oLabel = new QLabel(fileInfo.owner());
        formLayout->addRow(tr("Другие:"), oLabel);
        QLabel* gLabel = new QLabel(fileInfo.group());
        formLayout->addRow(tr("Группа:"), gLabel);
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void Properties::accept()
{
    done(1);
}

void Properties::reject()
{
    done(0);
}
