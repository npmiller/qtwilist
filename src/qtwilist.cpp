#include "qtwilist.h"
#include "ui_qtwilist.h"
#include "ui_adddialog.h"

#include <QProcess>
#include <QDebug>
#include <QDialog>


AddDialog::AddDialog(QWidget *parent) : QDialog(parent), ui(new Ui::adddialog) {
    ui->setupUi(this);
	connect(this, &QDialog::accepted, this, &AddDialog::addStream);
}
void AddDialog::addStream() {
	QString name = ui->lineEdit->text().trimmed();
	if (name.isEmpty())
		return;

	QSettings settings;
	settings.beginGroup("streams");
	settings.beginGroup(name);
	settings.setValue("link", name);
	settings.endGroup();
	settings.endGroup();
}

qtwilist::qtwilist(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::qtwilist),
	process(new QProcess(this)), list()
{
    ui->setupUi(this);
	ui->streamList->setModel(&list);

	ui->mainToolBar->addAction(ui->actionPlay);
	ui->mainToolBar->addAction(ui->actionAdd);
	ui->mainToolBar->addAction(ui->actionRemove);

	connect(ui->actionPlay, SIGNAL(triggered(bool)), this, SLOT(startStream(bool)));
	connect(ui->actionAdd, &QAction::triggered, this, &qtwilist::actionAdd);
	connect(ui->actionRemove, &QAction::triggered, this, &qtwilist::actionRemove);

	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(done(int, QProcess::ExitStatus)));

	QSettings settings;
	command = settings.value("command", "streamlink twitch.tv/%1 best").toString();
}

void qtwilist::actionAdd(bool checked) {
	AddDialog* dialog = new AddDialog(this);
	if (QDialog::Accepted == dialog->exec()) {
		list.reload();
	}
}

void qtwilist::actionRemove(bool checked) {
	auto selection = ui->streamList->selectionModel()->selectedIndexes();
	if (selection.size() <= 0)
		return;
	QString stream_name = list.streams.at(selection.at(0).row())->name;

	QSettings settings;
	settings.beginGroup("streams");
	settings.remove(stream_name);
	settings.endGroup();
	list.reload();
}

void qtwilist::startStream(bool checked) {
	(void)checked;
	auto selection = ui->streamList->selectionModel()->selectedIndexes();
	if (selection.size() <= 0)
		return;
	QString stream_name = list.streams.at(selection.at(0).row())->name;
	QStringList cmd = command.arg(stream_name).split(" ");
	QString prg = cmd.takeFirst();
	process->start(prg, cmd);
	ui->statusBar->showMessage("Playing: " + stream_name);
	ui->actionPlay->setEnabled(false);
	
}

void qtwilist::done(int exitCode, QProcess::ExitStatus exitStatus) {
	ui->statusBar->clearMessage();
	ui->actionPlay->setEnabled(true);
}

qtwilist::~qtwilist()
{
    delete ui;
}
