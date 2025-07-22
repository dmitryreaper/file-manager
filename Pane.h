#ifndef PANE_H
#define PANE_H

//#include <QtGui>
#include <QTreeView>
#include <QListView>
#include <QLineEdit>
#include <QStackedWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QVBoxLayout>

class MainWindow;

class Pane : public QFrame
{
    Q_OBJECT
public:
    explicit Pane(QWidget* parent=0);

    MainWindow *mainWindow; // for convenience
    QTreeView *treeView;
    QListView *listView;
    QLineEdit* pathLineEdit;
    QStackedWidget* stackedWidget;

    enum ViewMode
    {
        TreeViewMode,
        ListViewMode
    };

    void moveTo(const QString& path);
    void setActive(bool active);
    bool isActive() const;
    void setViewTo(const ViewMode viewMode);

protected:

private:
    bool active;
    QVBoxLayout* vBoxLayout;

signals:

private slots:
    void doubleClickedOnEntry(QModelIndex index);
    void showContextMenu(const QPoint&);
    void pathLineEditChanged();
};

#endif // PANE_H
