#include "MainWindow.h"
#include "Pane.h"
#include "PreferencesDialog.h"
#include "Properties.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("FILE MANAGER"));
    setUnifiedTitleAndToolBarOnMac(true);
    createActionsAndMenus();

    settings = new QSettings("MRK_SYSTEMS", "file-manager");
    splitter = new QSplitter(this);

    fileSystemModel = new QFileSystemModel;
    fileSystemModel->setFilter(QDir::NoDot | QDir::AllEntries | QDir::System);
    fileSystemModel->setRootPath("");
    fileSystemModel->setReadOnly(false);

    fileSystemProxyModel = new FileSystemModelFilterProxyModel();
    fileSystemProxyModel->setSourceModel(fileSystemModel);
    fileSystemProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    directoryTreeView = new QTreeView(splitter);
    directoryTreeView->setModel(fileSystemProxyModel);
    directoryTreeView->setHeaderHidden(true);
    directoryTreeView->setUniformRowHeights(true);
    directoryTreeView->hideColumn(1);
    directoryTreeView->hideColumn(2);
    directoryTreeView->hideColumn(3);
    directoryTreeView->setDragDropMode(QAbstractItemView::DropOnly);
    directoryTreeView->setDefaultDropAction(Qt::MoveAction);
    directoryTreeView->setDropIndicatorShown(true);
    directoryTreeView->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    directoryTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(directoryTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    treeSelectionModel = directoryTreeView->selectionModel();
    connect(treeSelectionModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(treeSelectionChanged(QModelIndex, QModelIndex)));

    leftPane = new Pane(splitter);
    rightPane = new Pane(splitter);

    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), SLOT(focusChangedSlot(QWidget*, QWidget*)));

    splitter->addWidget(directoryTreeView);
    splitter->addWidget(leftPane);
    splitter->addWidget(rightPane);
    splitter->setHandleWidth(3);
    this->setCentralWidget(splitter);
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardChanged()));
    restoreState();
}

//MainWindow::~MainWindow()
//{
//}

void MainWindow::focusChangedSlot(QWidget *, QWidget *now)
{
    if (now == rightPane->pathLineEdit || now == rightPane->treeView || now == rightPane->listView)
        setActivePane(rightPane);
    else if (now == leftPane->pathLineEdit || now == leftPane->treeView || now == leftPane->listView)
        setActivePane(leftPane);
}

void MainWindow::setActivePane(Pane* pane)
{
    pane->setActive(true);
    if (pane == leftPane)
        rightPane->setActive(false);
    else
        leftPane->setActive(false);
    activePane = pane;
    updateViewActions();
}

Pane* MainWindow::getActivePane()
{
    return(activePane);
}

void MainWindow::treeSelectionChanged(QModelIndex current, QModelIndex previous)
{
    QFileInfo fileInfo(fileSystemModel->fileInfo(fileSystemProxyModel->mapToSource(current)));
    if(!fileInfo.exists())
        return;
    getActivePane()->moveTo(fileInfo.filePath());
}

void MainWindow::moveTo(const QString path)
{
    QModelIndex index = fileSystemProxyModel->mapFromSource(fileSystemModel->index(path));
    treeSelectionModel->select(index, QItemSelectionModel::Select);
    getActivePane()->moveTo(path);
}

void MainWindow::showContextMenu(const QPoint& position)
{
    contextMenu->exec(directoryTreeView->mapToGlobal(position));
}

void MainWindow::cut()
{
    QModelIndexList selectionList;

    QWidget* focus(focusWidget());
    QAbstractItemView* view;
    if (focus == directoryTreeView || focus == leftPane->treeView || focus == leftPane->listView || focus == rightPane->treeView || focus == rightPane->listView){
        view =  qobject_cast<QAbstractItemView *>(focus);
        selectionList = view->selectionModel()->selectedIndexes();
    }

    if(selectionList.count() == 0)
        return;

    QApplication::clipboard()->setMimeData(fileSystemModel->mimeData(selectionList));
    pasteAction->setData(true);

    view->selectionModel()->clear();
}

void MainWindow::copy()
{
    QModelIndexList selectionList;

    QWidget* focus(focusWidget());
    QAbstractItemView* view;
    if (focus == directoryTreeView || focus == leftPane->treeView || focus == leftPane->listView || focus == rightPane->treeView || focus == rightPane->listView){
        view =  qobject_cast<QAbstractItemView *>(focus);
        selectionList = view->selectionModel()->selectedIndexes();
    }

    if(selectionList.count() == 0)
        return;

    QApplication::clipboard()->setMimeData(fileSystemModel->mimeData(selectionList));
    pasteAction->setData(false);
#ifdef __HAIKU__
    pasteAction->setEnabled(true);
#endif
}

void MainWindow::paste()
{
    QWidget* focus(focusWidget());
    Qt::DropAction cutOrCopy(pasteAction->data().toBool() ? Qt::MoveAction : Qt::CopyAction);

    if (focus == leftPane->treeView || focus == leftPane->listView || focus == rightPane->treeView || focus == rightPane->listView){
        fileSystemModel->dropMimeData(QApplication::clipboard()->mimeData(), cutOrCopy, 0, 0, qobject_cast<QAbstractItemView *>(focus)->rootIndex());
    } else if (focus == directoryTreeView){
        fileSystemModel->dropMimeData(QApplication::clipboard()->mimeData(), cutOrCopy, 0, 0,  fileSystemProxyModel->mapToSource(directoryTreeView->currentIndex()));
    }
}

void MainWindow::del()
{
    QModelIndexList selectionList;
    bool yesToAll = false;
    bool ok = false;
    bool confirm = true;

    QWidget* focus(focusWidget());
    QAbstractItemView* view;
    if (focus == directoryTreeView || focus == leftPane->treeView || focus == leftPane->listView || focus == rightPane->treeView || focus == rightPane->listView){
        view = qobject_cast<QAbstractItemView *>(focus);
        selectionList = view->selectionModel()->selectedIndexes();
    }

    for(int i = 0; i < selectionList.count(); ++i)
    {
        QFileInfo file(fileSystemModel->filePath(selectionList.at(i)));
        if(file.isWritable())
        {
            if(file.isSymLink()) ok = QFile::remove(file.filePath());
            else
            {
                if(!yesToAll)
                {
                    if(confirm)
                    {
                        int answer = QMessageBox::information(this, tr("Удалить файл"), "Вы точно хотите удалить файл <p><b>\"" + file.filePath() + "</b>?",QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll);
                        if(answer == QMessageBox::YesToAll)
                            yesToAll = true;
                        if(answer == QMessageBox::No)
                            return;
                    }
                }
                ok = fileSystemModel->remove(selectionList.at(i));
            }
        }
        else if(file.isSymLink())
            ok = QFile::remove(file.filePath());
    }

    if(!ok)
        QMessageBox::information(this, tr("Ошибка удаления"), tr("Некоторые файлы удалить не удалось."));
}

void MainWindow::newFolder()
{
    QAbstractItemView* currentView = qobject_cast<QAbstractItemView *>(getActivePane()->stackedWidget->currentWidget());
    QModelIndex newDir = fileSystemModel->mkdir(currentView->rootIndex(), QString("Новая папка"));

    if (newDir.isValid()) {
        currentView->selectionModel()->setCurrentIndex(newDir, QItemSelectionModel::ClearAndSelect);
        currentView->edit(newDir);
    }
}

void MainWindow::clipboardChanged()
{
    if(QApplication::clipboard()->mimeData()->hasUrls())
        pasteAction->setEnabled(true);
    else
    {
        pasteAction->setEnabled(false);
    }
}

void MainWindow::toggleToDetailView()
{
    getActivePane()->setViewTo(Pane::TreeViewMode);
}

void MainWindow::toggleToIconView()
{
    getActivePane()->setViewTo(Pane::ListViewMode);
}

void MainWindow::toggleHidden()
{
    if(hiddenAction->isChecked())
        fileSystemModel->setFilter(QDir::NoDot | QDir::AllEntries | QDir::System | QDir::Hidden);
    else
        fileSystemModel->setFilter(QDir::NoDot | QDir::AllEntries | QDir::System);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveState();
    event->accept();
}

void MainWindow::createActionsAndMenus()
{    
    deleteAction = new QAction(QIcon::fromTheme("edit-delete", QIcon(":/Images/Delete.ico")), tr("Удалить"), this );
    deleteAction->setStatusTip(tr("Удалить файл"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(del()));

    newFolderAction = new QAction(QIcon::fromTheme("edit-new", QIcon(":/Images/NewFolder.ico")), tr("Новая папка"), this );
    newFolderAction->setStatusTip(tr("Создать новую папку"));
    newFolderAction->setShortcut(QKeySequence::New);
    connect(newFolderAction, SIGNAL(triggered()), this, SLOT(newFolder()));

    preferencesAction = new QAction(QIcon::fromTheme("preferences-other", QIcon(":/Images/Preferences.ico")), tr("&Предпочтения..."), this );
    preferencesAction->setStatusTip(tr("Set Preferences"));
    preferencesAction->setShortcut(QKeySequence::Preferences);
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(showPreferences()));

    exitAction = new QAction(QIcon::fromTheme("application-exit", QIcon(":/Images/Exit.png")), tr("&Выйти"), this );
    exitAction->setMenuRole(QAction::QuitRole);
    exitAction->setStatusTip(tr("Закрыть FILE-MANAGER"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    cutAction = new QAction(QIcon::fromTheme("edit-cut", QIcon(":/Images/Cut.png")), tr("Вырезать"), this );
    cutAction->setStatusTip(tr("Вырезать файл"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

    copyAction = new QAction(QIcon::fromTheme("edit-copy", QIcon(":/Images/Copy.png")), tr("Скопировать"), this );
    copyAction->setStatusTip(tr("Скопировать файл"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAction = new QAction(QIcon::fromTheme("edit-paste", QIcon(":/Images/Paste.png")), tr("Вставить"), this );
    pasteAction->setStatusTip(tr("Вставить файл"));
    pasteAction->setEnabled(false);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

    detailViewAction = new QAction(QIcon::fromTheme("view-list-details", QIcon(":/Images/DetailView.ico")), tr("Детальный просмотр"), this );
    detailViewAction->setStatusTip(tr("Показать детальный список"));
    detailViewAction->setCheckable(true);
    connect(detailViewAction, SIGNAL(triggered()), this, SLOT(toggleToDetailView()));

    iconViewAction = new QAction(QIcon::fromTheme("view-list-icons", QIcon(":/Images/IconView.ico")), tr("Просмотр"), this );
    iconViewAction->setStatusTip(tr("Паказать список с иконками"));
    iconViewAction->setCheckable(true);
    connect(iconViewAction, SIGNAL(triggered()), this, SLOT(toggleToIconView()));

    hiddenAction = new QAction(QIcon::fromTheme("folder-saved-search"), tr("Показать скрытые"), this );
    hiddenAction->setStatusTip(tr("Показать скрытые файлы"));
    hiddenAction->setCheckable(true);
    connect(hiddenAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));

    viewActionGroup = new QActionGroup(this);
    viewActionGroup->addAction(detailViewAction);
    viewActionGroup->addAction(iconViewAction);

    aboutAction = new QAction(QIcon::fromTheme("О приложении", QIcon(":/Images/About.ico")), tr("&О приложении"), this );
    aboutAction->setStatusTip(tr("О Файловом менеджере"));
    aboutAction->setMenuRole (QAction::AboutRole);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutBox()));

    aboutHelpAction = new QAction(QIcon::fromTheme("Справка", QIcon(":/Images/help.ico")), tr("Справка приложения"), this);
    aboutHelpAction->setStatusTip(tr("Справка приложения"));
    aboutHelpAction->setMenuRole(QAction::AboutRole);
    connect(aboutHelpAction, SIGNAL(triggered()), this, SLOT(showHelpBox()));

    propertiesAction = new QAction(QIcon::fromTheme("Свойства файла", QIcon(":/Images/Properties.ico")), tr("Свойства"), this );
    propertiesAction->setStatusTip(tr("Свойства"));
    propertiesAction->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(propertiesAction, SIGNAL(triggered()), this, SLOT(showProperties()));

    menuBar = new QMenuBar(0);

    fileMenu = menuBar->addMenu(tr("&Файл"));
    fileMenu->addAction(newFolderAction);
    fileMenu->addAction(deleteAction);
    fileMenu->addAction(preferencesAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu = menuBar->addMenu(tr("&Редактирование"));
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);

    viewMenu = menuBar->addMenu(tr("&Просмотр"));
    viewMenu->addAction(detailViewAction);
    viewMenu->addAction(iconViewAction);
    viewMenu->addAction(hiddenAction);

    helpMenu = menuBar->addMenu(tr("&Помощь"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutHelpAction);

    setMenuBar(menuBar);

    toolBar = addToolBar(tr("Main"));

    contextMenu = new QMenu(this);
    contextMenu->addAction(detailViewAction);
    contextMenu->addAction(iconViewAction);
    contextMenu->addAction(propertiesAction);
    contextMenu->addSeparator();
    contextMenu->addAction(cutAction);
    contextMenu->addAction(copyAction);
    contextMenu->addAction(pasteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(deleteAction);

    toolBar->addAction(detailViewAction);
    toolBar->addAction(iconViewAction);
}

void MainWindow::saveState()
{
    settings->setValue("Geometry", saveGeometry());
    settings->setValue("Показать статус бар", statusBar()->isVisible());
    settings->setValue("ShowToolBar", toolBar->isVisible());

    settings->setValue("MainSplitterSizes", splitter->saveState());
    settings->setValue("LeftPaneActive", leftPane->isActive());

    settings->setValue("LeftPanePath", leftPane->pathLineEdit->text());
    settings->setValue("LeftPaneFileListHeader", leftPane->treeView->header()->saveState());
    settings->setValue("LeftPaneViewMode", leftPane->stackedWidget->currentIndex());

    settings->setValue("RightPanePath", rightPane->pathLineEdit->text());
    settings->setValue("RightPaneFileListHeader", rightPane->treeView->header()->saveState());
    settings->setValue("RightPaneViewMode", rightPane->stackedWidget->currentIndex());
    settings->setValue("ShowHidden", hiddenAction->isChecked());
}

void MainWindow::restoreState()
{
    restoreGeometry(settings->value("Geometry").toByteArray());
    toolBar->setVisible(settings->value("ShowToolBar", QVariant(true)).toBool());
    statusBar()->setVisible(settings->value("ShowStatusBar", QVariant(false)).toBool());
    splitter->restoreState(settings->value("MainSplitterSizes").toByteArray());
    setActivePane(settings->value("LeftPaneActive", 1).toBool() ? leftPane : rightPane);
    leftPane->treeView->header()->restoreState(settings->value("LeftPaneFileListHeader").toByteArray());
    leftPane->moveTo(settings->value("LeftPanePath", "").toString());
    leftPane->stackedWidget->setCurrentIndex(settings->value("LeftPaneViewMode", 0).toInt());
    rightPane->treeView->header()->restoreState(settings->value("RightPaneFileListHeader").toByteArray());
    rightPane->moveTo(settings->value("RightPanePath", "").toString());
    rightPane->stackedWidget->setCurrentIndex(settings->value("RightPaneViewMode", 0).toInt());
    hiddenAction->setChecked(settings->value("ShowHidden", false).toBool());
    toggleHidden();
}

void MainWindow::updateViewActions()
{
    switch (activePane->stackedWidget->currentIndex())
    {
    case Pane::TreeViewMode:
        detailViewAction->setChecked(true);
        break;
    case Pane::ListViewMode:
        iconViewAction->setChecked(true);
        break;
    }
}

void MainWindow::showAboutBox()
{
    QMessageBox::about(this, tr("About FILE MANAGER"),
                       tr("<h2>FILE MANAGER</h2>"
                          "<p><em>Version 1.0</em>"
                          "<p>File Manager<br>"
                          "Автор программы: Дубновицкий Дмитрий "
                          "<p>2024</br>"));
}

void MainWindow::showHelpBox()
{
    QMessageBox::about(this, tr("Справка программы"),
                       tr("<h1>Основные функции</h1>"
                          "<h2>1. Навигация по файловой системе</h2>"
                          "<p>Файловый менеджер позволяет перемещаться по файловой системе вашего компьютера. Вы можете открывать папки и просматривать их содержимое.</br>"

                          "<p>Открытие папки: Для открытия папки дважды щелкните по её значку или выберите её и нажмите Enter</br>"
                          "<p>Возврат на уровень выше: Нажмите кнопку \"Назад\" или используйте стрелку \"Вверх\" на панели инструментов. </br>"

                          "<h2>2. Управление файлами и папками</h2>"
                          "<p>Файловый менеджер предоставляет различные инструменты для управления вашими файлами и папками.<br>"

                          "<p>Создание новой папки: Щелкните правой кнопкой мыши в пустом месте в окне файлового менеджера и выберите \"Создать папку\".</br>"
                          "<p>Переименование: Щелкните правой кнопкой мыши на файле или папке и выберите \"Переименовать\", или выберите файл/папку и нажмите F2.</br>"
                          "<p>Копирование и вставка: Выберите файл/папку, нажмите Ctrl+C для копирования, затем перейдите в целевую папку и нажмите Ctrl+V для вставки.</br>"
                          "<p>Перемещение: Выберите файл/папку и нажмите Ctrl+X для вырезания, затем перейдите в целевую папку и нажмите Ctrl+V для вставки.</br>"
                          "<p>Удаление: Выберите файл/папку и нажмите Del или щелкните правой кнопкой мыши и выберите \"Удалить\".</br>"

                          "<h2>Завершение работы</h2>"
                          "<p>Не забудьте сохранять все изменения и корректно завершать работу с файловым менеджером.</br>"
                          "<p>Закрытие программы: Закройте файловый менеджер, щелкнув на значок \"Закрыть\" в верхнем правом углу окна или выбрав \"Выход\" в меню.</br>"));

}

void MainWindow::showPreferences()
{
    PreferencesDialog preferences(this);
    preferences.exec();
}

void MainWindow::showProperties()
{
    Properties properties(this);
    properties.exec();
}

bool FileSystemModelFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QFileSystemModel* fileSystemModel = qobject_cast<QFileSystemModel*>(sourceModel());
    if (fileSystemModel->isDir(index0) && fileSystemModel->fileName(index0).compare("..") != 0)
        return true;
    else
        return false;
}
