#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QFileSystemModel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QSettings>
#include <QMainWindow>
#include <QClipboard>
#include <QMessageBox>
#include <QMimeData>
#include <QHeaderView>
#include <QCloseEvent>
#include <QProcess>
#include <QDebug>

class Pane;
class PreviewPane;
class PathValidator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);
//    ~MainWindow();
    QFileSystemModel* fileSystemModel;
    QTreeView* directoryTreeView;
    QSortFilterProxyModel* fileSystemProxyModel;
    QMenu* contextMenu;
    QToolBar* toolBar;
    Pane* leftPane;
    Pane* rightPane;
    Pane* activePane;

    void setActivePane(Pane* pane);
    Pane* getActivePane();
    void moveTo(QString path);
    void updateViewActions();

public slots:
    void clipboardChanged();

protected:
    void closeEvent(QCloseEvent *event);

signals:

private:
    QMenuBar* menuBar;
//    QStatusBar* statusBar;

    QMenu* fileMenu;
    QMenu* editMenu;
    QMenu* viewMenu;
    QMenu* helpMenu;
    QIcon* aboutIcon;
    QAction* exitAction;
    QAction* preferencesAction;
    QAction* cutAction;
    QAction* copyAction;
    QAction* pasteAction;
    QAction* deleteAction;
    QAction* newFolderAction;
    QAction* detailViewAction;
    QAction* iconViewAction;
    QAction* hiddenAction;
    QActionGroup* viewActionGroup;
    QAction* aboutAction;
    QAction* aboutHelpAction;
    QAction* propertiesAction;
    QSplitter* splitter;
    QItemSelectionModel* treeSelectionModel;
    QSettings* settings;

    void createActionsAndMenus();
    void saveState();
    void restoreState();
    void activeView();

private slots:
    void treeSelectionChanged(QModelIndex current, QModelIndex previous);
    void cut();
    void copy();
    void paste();
    void del();
    void newFolder();
    void showAboutBox();
    void showHelpBox();
    void showPreferences();
    void focusChangedSlot(QWidget *, QWidget *now);
    void toggleToDetailView();
    void toggleToIconView();
    void toggleHidden();
    void showContextMenu(const QPoint&);
    void showProperties();
};

class FileSystemModelFilterProxyModel : public QSortFilterProxyModel
{
protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
};

#endif // MAINWINDOW_H
