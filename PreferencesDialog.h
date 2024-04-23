#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
class QDialogButtonBox;
class QGroupBox;
//class QRadioButton;
class QCheckBox;
class QToolBar;
class QStatusBar;

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = 0);

private:
    QDialogButtonBox* buttonBox;
    QCheckBox* showToolBarCheckBox;
    QCheckBox* showStatusBarCheckBox;
    QToolBar* toolBar;
    QStatusBar* statusBar;

signals:

public slots:

private slots:
    void accept();
    void reject();
};
#endif
