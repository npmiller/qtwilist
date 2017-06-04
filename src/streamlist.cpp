#include "streamlist.h"

#include <QObject>
#include <QSettings>
#include <QAbstractListModel>
#include <QIcon>
#include <QNetworkRequest>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QSize>
#include <QStandardPaths>
#include <QDir>

static constexpr char client_id[] = "";
static constexpr char url_base[] = "https://api.twitch.tv/kraken";

Stream::Stream(QString name, QNetworkAccessManager& manager, QObject *parent) : QObject(parent), name(name), live(false), manager(manager), reply(nullptr) {
	fetch();
}

void Stream::fetch() {
	qDebug() << "fetch";
	QNetworkRequest request(QUrl(QString(url_base) + "/users?login=" + name));
	request.setRawHeader(QByteArray("Accept"), QByteArray("application/vnd.twitchtv.v5+json"));
	request.setRawHeader(QByteArray("Client-ID"), QByteArray("cxajit9fktm6qfmkauvlkwmoalrqrs"));

	reply = manager.get(request);
	connect(reply, &QNetworkReply::finished , this, &Stream::finishedUser);
}

void Stream::finishedUser() {
	qDebug() << "Finished user";
	// read JSON reply
	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	if (!doc.isObject()) {
		qDebug() << "Reply is not a JSON object";
	}
	QJsonObject obj = doc.object();

	// get user id
	id = obj["users"].toArray()[0].toObject()["_id"].toString();

	// mark network reply for deletion
	disconnect(reply, &QNetworkReply::finished , this, &Stream::finishedUser);
	reply->deleteLater();

	// request streams information
	QNetworkRequest request(QUrl(QString(url_base) + "/streams/?channel=" + id));
	request.setRawHeader(QByteArray("Accept"), QByteArray("application/vnd.twitchtv.v5+json"));
	request.setRawHeader(QByteArray("Client-ID"), QByteArray("cxajit9fktm6qfmkauvlkwmoalrqrs"));

	reply = manager.get(request);
	connect(reply, &QNetworkReply::finished , this, &Stream::finishedStreams);

	// download user logo
	request.setUrl(QUrl(obj["users"].toArray()[0].toObject()["logo"].toString()));
	QNetworkReply *logo_reply = manager.get(request);
	connect(logo_reply, &QNetworkReply::finished , this, &Stream::finishedLogo);
}

void Stream::finishedLogo() {
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

	QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));

	// create dir if it doesn't exist
	dir.mkpath(dir.path());

	QString ext = reply->header(QNetworkRequest::ContentTypeHeader).toString();
	// image/<extension>
	ext.remove(0, 6);

	logo_path = dir.filePath(name + "_logo" + ext);
	QFile file(logo_path);
	file.open(QIODevice::WriteOnly);
	file.write(reply->readAll());
	file.close();

	reply->deleteLater();
	disconnect(reply, &QNetworkReply::finished , this, &Stream::finishedLogo);
}

void Stream::finishedStreams() {
	qDebug() << "Finished streams";
	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	if (!doc.isObject()) {
		qDebug() << "Reply is not a JSON object";
	}
	QJsonObject obj = doc.object();

	live = (obj["_total"].toInt() > 0);

	disconnect(reply, &QNetworkReply::finished , this, &Stream::finishedStreams);
	reply->deleteLater();
}

/* TwitchAPI::TwitchAPI(QObject *parent) : QObject(parent), manager(parent) { */
/* 	connect(&manager, SIGNAL(finished(QNetworkReply*)), */
/* 			this, SLOT(replyFinished(QNetworkReply*))); */
/* } */

/* void TwitchAPI::replyFinished(QNetworkReply* reply) { */
/* 	qDebug() << "reply finished" << reply->request().rawHeaderList(); */
/* 	QByteArray r = reply->readLine(); */
/* 	QJsonDocument doc = QJsonDocument::fromJson(r); */
/* 	QJsonObject obj = doc.object(); */
/* 	qDebug() << obj["users"].toArray()[0].toObject()["_id"]; */

/* 	QNetworkRequest request(QUrl(obj["users"].toArray()[0].toObject()["logo"].toString())); */

/* 	qDebug() << QStandardPaths::writableLocation(QStandardPaths::CacheLocation); */
/* } */

/* void TwitchAPI::readyRead() { */
/* 	qDebug() << "ready read"; */
/* } */
/* void TwitchAPI::error(QNetworkReply::NetworkError err) { */
/* 	qDebug() << err<< "error"; */
/* } */
/* void TwitchAPI::sslErrors(QList<QSslError> errs) { */
/* 	qDebug() << "ssl error"; */
/* } */

/* void TwitchAPI::getUser(QString name) { */
/* 	// prepare request */
/* 	QNetworkRequest request(QUrl(QString(url_base) + "/users?login=" + name)); */
/* 	request.setRawHeader(QByteArray("Accept"), QByteArray("application/vnd.twitchtv.v5+json")); */
/*     request.setRawHeader(QByteArray("Client-ID"), QByteArray("cxajit9fktm6qfmkauvlkwmoalrqrs")); */
/* 	qDebug() << "get user\n" << request.rawHeaderList(); */

/* 	qDebug() << request.url().toString(); */
/* 	const QList<QByteArray>& rawHeaderList(request.rawHeaderList()); */
/* 	Q_FOREACH (QByteArray rawHeader, rawHeaderList) { */
/* 		qDebug() << rawHeader << ": " << request.rawHeader(rawHeader); */
/* 	} */
/* 	QNetworkReply *reply = manager.get(request); */
/* 	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead())); */
/* 	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), */
/* 			this, SLOT(error(QNetworkReply::NetworkError))); */
/* 	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), */
/* 			this, SLOT(sslErrors(QList<QSslError>))); */
/* } */


StreamList::StreamList(QObject* parent) : QAbstractListModel(parent), manager(parent) {
	reload();
};

void StreamList::reload() {
	QSettings settings;
	settings.beginGroup("streams");
	beginResetModel();
	streams.clear();
	Q_FOREACH(QString s, settings.childGroups()) {
		streams.append(new Stream(s, manager, this));
	}
	endResetModel();
	settings.endGroup();
}

int StreamList::rowCount(const QModelIndex & parent) const {
	return streams.size();
};


QVariant StreamList::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (index.row() >= rowCount())
		return QVariant();

	const Stream* s = streams.at(index.row());

	switch (role) {
		case Qt::DisplayRole:
			return s->name + (s->live ? " - live" : QString());
		case Qt::DecorationRole:
			return (s->logo_path.isEmpty() ? QVariant() : QIcon(s->logo_path));
	}

	return QVariant();
};
