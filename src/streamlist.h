#ifndef STREAMLIST_H
#define STREAMLIST_H

#include <QAbstractListModel>
#include <QSettings>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QList>
#include <QSslError>

class TwitchAPI : QObject {
	Q_OBJECT
	public:
	TwitchAPI(QObject *parent = 0);
	void getUser(QString name);

	public slots:
		void replyFinished(QNetworkReply* reply);
	void readyRead();
	void error(QNetworkReply::NetworkError err);
	void sslErrors(QList<QSslError> errs);

	private:
		QNetworkAccessManager manager;
};

class StreamList : public QAbstractListModel
{
public:
	StreamList(QObject * parent = 0);
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

	void reload();
	virtual QVariant data(const QModelIndex & index, int role) const;
	virtual ~StreamList() {};

	QStringList streams;
	TwitchAPI api;

};

#endif
