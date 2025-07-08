#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QDialog>
#include <QTextEdit> 
#include <QLineEdit> 
#include <QFileInfo> 

class Properties : public QDialog
{
    Q_OBJECT
public:
    explicit Properties(QWidget *parent = 0);

signals:

public slots:

private slots:
    void accept();
    void reject();

private:
    QTextEdit* descriptionEdit;
    QLineEdit* tagsEdit;
    QFileInfo currentFileInfo; 
};

#endif
