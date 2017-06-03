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

static constexpr char client_id[] = "";
static constexpr char url_base[] = "https://api.twitch.tv/kraken";

TwitchAPI::TwitchAPI(QObject *parent) : QObject(parent), manager(parent) {
	connect(&manager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(replyFinished(QNetworkReply*)));
}

void TwitchAPI::replyFinished(QNetworkReply* reply) {
	qDebug() << "reply finished" << reply->request().rawHeaderList();
	QByteArray r = reply->readLine();
	QJsonDocument doc = QJsonDocument::fromJson(r);
	QJsonObject obj = doc.object();
	qDebug() << obj["users"].toArray()[0].toObject()["_id"];
	qDebug() << obj["users"].toArray()[0].toObject()["logo"].toString();

}

void TwitchAPI::readyRead() {
	qDebug() << "ready read";
}
void TwitchAPI::error(QNetworkReply::NetworkError err) {
	qDebug() << err<< "error";
}
void TwitchAPI::sslErrors(QList<QSslError> errs) {
	qDebug() << "ssl error";
}

void TwitchAPI::getUser(QString name) {
	// prepare request
	QNetworkRequest request(QUrl(QString(url_base) + "/users?login=" + name));
	request.setRawHeader(QByteArray("Accept"), QByteArray("application/vnd.twitchtv.v5+json"));
    request.setRawHeader(QByteArray("Client-ID"), QByteArray("cxajit9fktm6qfmkauvlkwmoalrqrs"));
	qDebug() << "get user\n" << request.rawHeaderList();

	qDebug() << request.url().toString();
	const QList<QByteArray>& rawHeaderList(request.rawHeaderList());
	Q_FOREACH (QByteArray rawHeader, rawHeaderList) {
		qDebug() << rawHeader << ": " << request.rawHeader(rawHeader);
	}
	QNetworkReply *reply = manager.get(request);
	connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, SLOT(error(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
			this, SLOT(sslErrors(QList<QSslError>)));
}


StreamList::StreamList(QObject* parent) : QAbstractListModel(parent), api(parent) {
	api.getUser("t90official");

	reload();
};

void StreamList::reload() {
	QSettings settings;
	settings.beginGroup("streams");
	beginResetModel();
	streams = settings.childGroups();
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

	switch (role) {
		case Qt::DisplayRole:
			return QString("%1").arg(streams.at(index.row()));
		case Qt::DecorationRole:
			return QIcon::fromTheme("edit-undo");
	}

	return QVariant();
};
