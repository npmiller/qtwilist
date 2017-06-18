#ifndef STREAMLIST_H
#define STREAMLIST_H

#include <QAbstractListModel>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QSettings>
#include <QSslError>
#include <QTimer>
#include <QUrl>

class Stream : public QObject {
	Q_OBJECT

      public:
	Stream(QString name, QNetworkAccessManager &manager,
	       QObject *parent = 0);
	Stream(Stream &&s);
	void fetch();
	void toggleLive();

      public slots:
	void finishedUser();
	void finishedLogo();

      public:
	QString name;
	bool live;
	QString logo_path;
	QString id;
	QPixmap decoration;

      private:
	QNetworkAccessManager &manager;
	QNetworkReply *reply;
};

class StreamList : public QAbstractListModel {
	Q_OBJECT
      public:
	static void prepareRequest(QNetworkRequest &r);

	StreamList(QObject *parent = 0);
	virtual ~StreamList();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role) const;

	void add(QString name);
	void remove(QModelIndex index);

	QVector<Stream *> streams;
	QNetworkAccessManager manager;

      public slots:
	void finishedCheckLive();
	void checkLive();

      private:
	QNetworkReply *reply;
	QTimer timer;
};

#endif
