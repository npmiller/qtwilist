#ifndef QTWILIST_H
#define QTWILIST_H

#include <QMainWindow>
#include <QProcess>
#include <QAbstractListModel>
#include <QSettings>
#include <QDialog>
#include <QDebug>
#include "streamlist.h"

namespace Ui {
class qtwilist;
class adddialog;
}

class AddDialog : public QDialog {
Q_OBJECT
	public:
	AddDialog(QWidget *parent = nullptr);

	Ui::adddialog *ui;
};

class qtwilist : public QMainWindow
{
    Q_OBJECT

public:
    explicit qtwilist(QWidget *parent = 0);
    ~qtwilist();

public slots:
	void startStream(bool checked);
	void done(int exitCode, QProcess::ExitStatus exitStatus);
	void actionAdd(bool checked);
	void actionRemove(bool checked);

private:
    Ui::qtwilist *ui;
	QProcess *process;
	StreamList list;
	QString command;
	QString quality;
};


#endif // QTWILIST_H
