#ifndef QTWILIST_H
#define QTWILIST_H

#include "streamlist.h"
#include <QAbstractListModel>
#include <QDebug>
#include <QDialog>
#include <QMainWindow>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>

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

class qtwilist : public QMainWindow {
	Q_OBJECT

      public:
	explicit qtwilist(QWidget *parent = 0);
	~qtwilist();

      public slots:
	void startStream(bool checked);
	void done(int exitCode, QProcess::ExitStatus exitStatus);
	void actionAdd(bool checked);
	void actionChat(bool checked);
	void actionRemove(bool checked);
	void play(const QModelIndex &index);

      private:
	Ui::qtwilist *ui;
	QProcess *process;
	StreamList list;
	StreamSort *proxy;
	QSystemTrayIcon *tray;
	QString command;
	QString quality;
};

#endif // QTWILIST_H
