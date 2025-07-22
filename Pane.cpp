#include "MainWindow.h"
#include "Pane.h"

Pane::Pane(QWidget *parent) : QFrame (parent)
{
    mainWindow = static_cast<MainWindow*>(parent->parent());
    pathLineEdit = new QLineEdit;
    connect(pathLineEdit, SIGNAL(editingFinished()), this, SLOT(pathLineEditChanged()));

    treeView = new QTreeView(this);
    treeView->setRootIsDecorated(false);
    treeView->setModel(mainWindow->fileSystemModel);
    treeView->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setItemsExpandable(false);
    treeView->setDragDropMode(QAbstractItemView::DragDrop);
    treeView->setDefaultDropAction(Qt::MoveAction);
    treeView->setDropIndicatorShown(true);
    //treeView->setMouseTracking(true);
    //treeView->setSortingEnabled(true);
    treeView->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, SIGNAL(activated(QModelIndex)), this, SLOT(doubleClickedOnEntry(QModelIndex)));
    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    listView = new QListView(this);
    listView->setWrapping(true);
    listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listView->setDragDropMode(QAbstractItemView::DragDrop);
    listView->setDefaultDropAction(Qt::MoveAction);
    listView->setDropIndicatorShown(true);
    listView->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(doubleClickedOnEntry(QModelIndex)));
    listView->setModel(mainWindow->fileSystemModel);
    listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(listView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    stackedWidget = new QStackedWidget();
    stackedWidget->addWidget(treeView);
    stackedWidget->addWidget(listView);

    vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->addWidget(pathLineEdit);
    vBoxLayout->addWidget(stackedWidget);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(0);
    setLayout(vBoxLayout);
    setFrameStyle(QFrame::Box | QFrame::Plain);
}

void Pane::setActive(bool active)
{
    if (this->active == active) {
        return;
    }

    this->active = active;
    treeView->setAlternatingRowColors(active);
    listView->setAlternatingRowColors(active);
    //    setLineWidth(active ? 1 : 0);
}

void Pane::doubleClickedOnEntry(QModelIndex index)
{
    Qt::KeyboardModifiers keybMod = QApplication::keyboardModifiers();
    if(keybMod == Qt::ControlModifier || keybMod == Qt::ShiftModifier)
        return;

    QFileInfo fileInfo(mainWindow->fileSystemModel->filePath(index));

    if(fileInfo.isDir()) {
        moveTo(fileInfo.absoluteFilePath());
    } else if (fileInfo.isExecutable()) {
        QProcess::startDetached(fileInfo.absoluteFilePath(), QStringList());
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    }
}

bool Pane::isActive() const
{
    return active;
}

void Pane::showContextMenu(const QPoint& position)
{
    mainWindow->contextMenu->exec(listView->mapToGlobal(position));
}

void Pane::moveTo(const QString& path)
{
    pathLineEdit->setText(path);
    QModelIndex index(mainWindow->fileSystemModel->index(path));
    treeView->setRootIndex(index);
    listView->setRootIndex(index);
    if (active)
        mainWindow->directoryTreeView->setCurrentIndex(mainWindow->fileSystemProxyModel->mapFromSource(index));
}

void Pane::setViewTo(const ViewMode viewMode)
{
    stackedWidget->setCurrentIndex(viewMode);
}

void Pane::pathLineEditChanged()
{
    QFileInfo file(pathLineEdit->text());
    if(file.isDir())
        moveTo(pathLineEdit->text());
    else
        pathLineEdit->setText(mainWindow->fileSystemModel->filePath(treeView->rootIndex()));
}
