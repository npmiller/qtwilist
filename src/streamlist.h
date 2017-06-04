#ifndef STREAMLIST_H
#define STREAMLIST_H

#include <QAbstractListModel>
#include <QSettings>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QList>
#include <QSslError>

/* class TwitchAPI : QObject { */
/* 	Q_OBJECT */
/* 	public: */
/* 	TwitchAPI(QObject *parent = 0); */
/* 	void getUser(QString name); */

/* 	public slots: */
/* 	void replyFinished(QNetworkReply* reply); */
/* 	void readyRead(); */
/* 	void error(QNetworkReply::NetworkError err); */
/* 	void sslErrors(QList<QSslError> errs); */

/* 	private: */
/* 		QNetworkAccessManager manager; */
/* }; */

class Stream : public QObject {
	Q_OBJECT

	public:
	Stream(QString name, QNetworkAccessManager& manager, QObject * parent = 0);
	Stream(Stream && s);
	void fetch();

	public slots:
		void finishedUser();
		void finishedStreams();
		void finishedLogo();

	public:
	QString name;
	bool live;
	QString logo_path;

	private:
	QNetworkAccessManager& manager;
	QNetworkReply *reply;
	QString id;
};

class StreamList : public QAbstractListModel
{
public:
	StreamList(QObject * parent = 0);
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

	void reload();
	virtual QVariant data(const QModelIndex & index, int role) const;
	virtual ~StreamList() {};

	QVector<Stream*> streams;
	QNetworkAccessManager manager;

};

#endif
