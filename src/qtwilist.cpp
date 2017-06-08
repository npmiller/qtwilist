#include "qtwilist.h"
#include "ui_adddialog.h"
#include "ui_qtwilist.h"

#include <QDebug>
#include <QDialog>
#include <QProcess>

AddDialog::AddDialog(QWidget *parent) : QDialog(parent), ui(new Ui::adddialog) {
	ui->setupUi(this);
}

qtwilist::qtwilist(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::qtwilist), process(new QProcess(this)),
      list() {
	ui->setupUi(this);
	ui->streamList->setModel(&list);

	ui->mainToolBar->addAction(ui->actionPlay);
	ui->mainToolBar->addAction(ui->actionAdd);
	ui->mainToolBar->addAction(ui->actionRemove);
	ui->mainToolBar->addAction(ui->actionRefresh);
	ui->centralWidget->addAction(ui->actionQuit);

	connect(ui->actionPlay, SIGNAL(triggered(bool)), this,
	        SLOT(startStream(bool)));
	connect(ui->actionAdd, &QAction::triggered, this, &qtwilist::actionAdd);
	connect(ui->actionRemove, &QAction::triggered, this,
	        &qtwilist::actionRemove);
	connect(ui->actionRefresh, &QAction::triggered, &list,
	        &StreamList::checkLive);

	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
	        SLOT(done(int, QProcess::ExitStatus)));

	connect(ui->streamList, &QAbstractItemView::activated, this,
	        &qtwilist::play);

	QSettings settings;
	command = settings.value("command", "streamlink twitch.tv/%1 best")
	              .toString();
}

void qtwilist::play(const QModelIndex &index) {
	QString stream_name = list.streams.at(index.row())->name;
	QStringList cmd = command.arg(stream_name).split(" ");
	QString prg = cmd.takeFirst();
	process->start(prg, cmd);
	ui->statusBar->showMessage("Playing: " + stream_name);
	ui->actionPlay->setEnabled(false);
}

void qtwilist::actionAdd(bool checked) {
	AddDialog *dialog = new AddDialog(this);
	if (QDialog::Accepted == dialog->exec()) {
		QString name = dialog->ui->lineEdit->text().trimmed();
		if (!name.isEmpty()) {
			list.add(name);
		}
	}
}

void qtwilist::actionRemove(bool checked) {
	auto selection = ui->streamList->selectionModel()->selectedIndexes();
	if (selection.size() <= 0)
		return;

	list.remove(selection.at(0));
}

void qtwilist::startStream(bool checked) {
	(void)checked;
	auto selection = ui->streamList->selectionModel()->selectedIndexes();
	if (selection.size() <= 0)
		return;

	play(selection.at(0));
}

void qtwilist::done(int exitCode, QProcess::ExitStatus exitStatus) {
	ui->statusBar->clearMessage();
	ui->actionPlay->setEnabled(true);
}

qtwilist::~qtwilist() { delete ui; }
