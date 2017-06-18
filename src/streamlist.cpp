#include "streamlist.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSettings>
#include <QSize>
#include <QStandardPaths>

static constexpr char url_base[] = "https://api.twitch.tv/kraken";

Stream::Stream(QString name, QNetworkAccessManager &manager, QObject *parent)
    : QObject(parent), name(name), live(false), manager(manager),
      reply(nullptr) {
	// load from settings
	QSettings settings;
	settings.beginGroup("streams");
	settings.beginGroup(name);

	id = settings.value("id", "").toString();
	logo_path = settings.value("logo", "").toString();

	settings.endGroup();
	settings.endGroup();
	
	// TODO: add slider for logo size
	if (!logo_path.isEmpty()) {
		decoration = QIcon(logo_path).pixmap(40 ,40, QIcon::Disabled);
	} else {
		decoration = QPixmap(QSize(40, 40));
		decoration.fill();
	}

	// if these information isn't in the settings fetch it
	if (id.isEmpty() || logo_path.isEmpty()) {
		fetch();
	}
}

void Stream::fetch() {
	qDebug() << "fetch";
	// TODO: bundle user requests to update logos at startup
	QNetworkRequest request(
	    QUrl(QString(url_base) + "/users?login=" + name));
	StreamList::prepareRequest(request);

	reply = manager.get(request);
	connect(reply, &QNetworkReply::finished, this, &Stream::finishedUser);
}

void Stream::finishedUser() {
	qDebug() << "Finished user";
	// read JSON reply
	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	if (!doc.isObject()) {
		qDebug() << "Reply is not a JSON object";
	}
	QJsonObject obj = doc.object();

	qDebug() << obj;

	// check if the specified user exists
	if (0 == obj["_total"].toInt()) {
		// TODO: do this check in the add dialog
		QMessageBox err;
		err.setText(
		    QStringLiteral("User '%1' doesn't exist.").arg(name));
		err.setIcon(QMessageBox::Warning);
		err.exec();
		return;
	}

	// get user id
	id = obj["users"].toArray()[0].toObject()["_id"].toString();

	// mark network reply for deletion
	disconnect(reply, &QNetworkReply::finished, this,
	           &Stream::finishedUser);
	reply->deleteLater();

	// download user logo
	QNetworkRequest request(
	    QUrl(obj["users"].toArray()[0].toObject()["logo"].toString()));
	StreamList::prepareRequest(request);

	QNetworkReply *logo_reply = manager.get(request);
	connect(logo_reply, &QNetworkReply::finished, this,
	        &Stream::finishedLogo);

	// store the user id in the settings
	QSettings settings;
	settings.beginGroup("streams");
	settings.beginGroup(name);
	settings.setValue("id", id);
	settings.endGroup();
	settings.endGroup();
}

void Stream::toggleLive() {
	live = !live;

	if (!logo_path.isEmpty()) {
		// TODO: add emblem on logo when live
		decoration = QIcon(logo_path).pixmap(
		    40, 40, live ? QIcon::Normal : QIcon::Disabled);
	}
}

void Stream::finishedLogo() {
	QNetworkReply *reply = static_cast<QNetworkReply *>(sender());

	QDir dir(
	    QStandardPaths::writableLocation(QStandardPaths::CacheLocation));

	// create dir if it doesn't exist
	dir.mkpath(dir.path());

	QString ext =
	    reply->header(QNetworkRequest::ContentTypeHeader).toString();
	// image/<extension>
	ext.remove(0, 6);

	logo_path = dir.filePath(name + "_logo." + ext);
	QFile file(logo_path);
	file.open(QIODevice::WriteOnly);
	file.write(reply->readAll());
	file.close();

	// store the logo path in the settings
	QSettings settings;
	settings.beginGroup("streams");
	settings.beginGroup(name);
	settings.setValue("logo", logo_path);
	settings.endGroup();
	settings.endGroup();

	decoration = QIcon(logo_path).pixmap(
	    40, 40, live ? QIcon::Normal : QIcon::Disabled);

	reply->deleteLater();
	disconnect(reply, &QNetworkReply::finished, this,
	           &Stream::finishedLogo);
}

void StreamList::prepareRequest(QNetworkRequest &r) {
	// set API version
	r.setRawHeader(QByteArray("Accept"),
	               QByteArray("application/vnd.twitchtv.v5+json"));

	// set Client ID
	r.setRawHeader(QByteArray("Client-ID"),
	               QByteArray("cxajit9fktm6qfmkauvlkwmoalrqrs"));
}

void StreamList::finishedCheckLive() {
	qDebug() << "Finished streams";
	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

	disconnect(reply, &QNetworkReply::finished, this,
	           &StreamList::finishedCheckLive);
	reply->deleteLater();

	if (!doc.isObject()) {
		qDebug() << "Reply is not a JSON object";
		return;
	}
	QJsonObject obj = doc.object();

	unsigned int total = obj["_total"].toInt();
	if (total < 0) {
		return;
	}

	QJsonArray arr = obj["streams"].toArray();

	int mini = -1;
	int maxi = -1;
	for (int i = 0; i < streams.size(); ++i) {
		bool live = false;
		Stream *s = streams.at(i);
		for (int j = 0; j < total; ++j) {
			if (0 ==
			    QString::compare(arr[j]
			                         .toObject()["channel"]
			                         .toObject()["display_name"]
			                         .toString(),
			                     s->name, Qt::CaseInsensitive)) {
				qDebug() << "match: " << s->name << i << streams.size();
				live = true;
			}
		}

		if (live != s->live) {
			s->toggleLive();

			// first index to have changed
			mini = (mini < 0) ? i : mini;

			// we have a new maximum
			maxi = i;
		}
	}

	// see if we need to notify the change
	if ((mini > 0) && (maxi >= mini)) {
		qDebug() << "Emit data changed" << mini << maxi;
		Q_EMIT dataChanged(createIndex(mini, 0), createIndex(maxi, 0));
	}
}

StreamList::StreamList(QObject *parent)
    : QAbstractListModel(parent), manager(parent) {
	QSettings settings;
	settings.beginGroup("streams");
	Q_FOREACH (QString s, settings.childGroups()) {
		streams.append(new Stream(s, manager, this));
	}
	settings.endGroup();

	checkLive();

	// TODO: enable polling? add desktop notifications + systray
	// check for live channels every minute
	/* timer.setInterval(1000 * 60); */
	/* connect(&timer, &QTimer::timeout, this, &StreamList::checkLive); */
	/* timer.start(); */
}

StreamList::~StreamList() {
	// delete stream objects
	Q_FOREACH (Stream* s, streams) {
		delete s;
	}
}

void StreamList::checkLive() {
	if (streams.size() < 0) {
		return;
	}

	// build query for all our streams
	QString url = QString(url_base) + "/streams/?channel=";
	url += streams.at(0)->id;
	for (int i = 1; i < streams.size(); ++i) {
		url += ",";
		url += streams.at(i)->id;
	}
	qDebug() << url;

	// request streams information
	QNetworkRequest request(url);
	StreamList::prepareRequest(request);

	reply = manager.get(request);
	connect(reply, &QNetworkReply::finished, this,
	        &StreamList::finishedCheckLive);
}

void StreamList::add(QString name) {
	Stream *s = new Stream(name, manager, this);

	streams.append(s);

	QModelIndex index = createIndex(streams.size() - 1, 0);
	Q_EMIT dataChanged(index, index);
}

void StreamList::remove(QModelIndex index) {
	Stream *s = streams.takeAt(index.row());

	QSettings settings;
	settings.beginGroup("streams");
	settings.remove(s->name);
	settings.endGroup();

	// delete logo file if we have one
	if (!s->logo_path.isEmpty()) {
		QFile file(s->logo_path);
		file.remove();
	}

	Q_EMIT(dataChanged(index, index));

	delete s;
}

int StreamList::rowCount(const QModelIndex &parent) const {
	return streams.size();
}

QVariant StreamList::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.row() >= rowCount())
		return QVariant();

	const Stream *s = streams.at(index.row());

	switch (role) {
	case Qt::DisplayRole:
		return s->name + (s->live ? " - live" : QString());
	case Qt::DecorationRole:
		return s->decoration;
	}

	return QVariant();
}
