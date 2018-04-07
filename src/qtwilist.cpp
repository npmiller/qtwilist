#include "qtwilist.h"
#include "ui_adddialog.h"
#include "ui_qtwilist.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QProcess>

AddDialog::AddDialog(QWidget *parent) : QDialog(parent), ui(new Ui::adddialog) {
	ui->setupUi(this);
}

qtwilist::qtwilist(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::qtwilist), process(new QProcess(this)),
      list() {
	ui->setupUi(this);
	setWindowIcon(QIcon(":/icon.svg"));

	proxy = new StreamSort(this);
	proxy->setSourceModel(&list);
	proxy->setDynamicSortFilter(true);
	proxy->sort(0);
	ui->streamList->setModel(proxy);

	ui->mainToolBar->addAction(ui->actionPlay);
	ui->mainToolBar->addAction(ui->actionChat);
	ui->mainToolBar->addAction(ui->actionAdd);
	ui->mainToolBar->addAction(ui->actionRemove);
	ui->mainToolBar->addAction(ui->actionRefresh);
	ui->centralWidget->addAction(ui->actionQuit);

	connect(ui->actionPlay, SIGNAL(triggered(bool)), this,
	        SLOT(startStream(bool)));
	connect(ui->actionAdd, &QAction::triggered, this, &qtwilist::actionAdd);
	connect(ui->actionChat, &QAction::triggered, this, &qtwilist::actionChat);
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
	bool live = index.data(StreamList::LiveRole).toBool();
	QString name = index.data(StreamList::NameRole).toString();
	if (live) {
		QStringList cmd = command.arg(name).split(" ");
		QString prg = cmd.takeFirst();

		// if another stream is open close it
		if (QProcess::NotRunning != process->state()) {
			process->terminate();
			// TODO: this is not immediate we need to wait before starting the
			// new one or use another QProcess while this one cleans up, maybe
			// allow multiple stream to play at the same time
		}

		// start playing
		process->start(prg, cmd);

		// update the ui
		ui->statusBar->showMessage("Playing: " + name);
		ui->actionPlay->setEnabled(false);
	}
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

void qtwilist::actionChat(bool checked) {
	auto selection = ui->streamList->selectionModel()->selectedIndexes();
	if (selection.size() <= 0)
		return;

	// Open chat in browser
	QDesktopServices::openUrl(
	    QUrl("https://www.twitch.tv/" +
	         selection.at(0).data(StreamList::NameRole).toString() +
	         "/chat?popout="));
}

void qtwilist::actionRemove(bool checked) {
	auto selection = ui->streamList->selectionModel()->selectedIndexes();
	if (selection.size() <= 0)
		return;

	list.remove(proxy->mapToSource(selection.at(0)));
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

qtwilist::~qtwilist() {
	delete process;
	delete ui;
}
