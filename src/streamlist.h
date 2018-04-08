#ifndef STREAMLIST_H
#define STREAMLIST_H

#include <QAbstractListModel>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSslError>
#include <QTimer>
#include <QUrl>

namespace Ui {
class qtwilist;
class adddialog;
class StreamWidget;
}

class Stream : public QObject {
	Q_OBJECT

      public:
	Stream(QString name, QNetworkAccessManager &manager,
	       QObject *parent = 0);
	Stream(Stream &&s);
	void fetch();
	void toggleLive(QString status);

      public slots:
	void finishedUser();
	void finishedLogo();

      public:
	QString name;
	QString status;
	bool live;
	uint32_t views;
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

	enum DataRoles {
		NameRole = Qt::UserRole,
		LiveRole,
		ViewsRole,
	};

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
	QTimer timer;
};

class StreamSort : public QSortFilterProxyModel {
	Q_OBJECT
      public:
    StreamSort(QObject *parent = 0);
	bool lessThan(const QModelIndex &left,
	              const QModelIndex &right) const override;
};

/* class StreamDelegate : public QAbstractItemDelegate { */
/* 	Q_OBJECT */
/*       public: */
/* 	StreamDelegate(QWidget *parent = 0) : QAbstractItemDelegate(parent){}; */
/* 	virtual ~StreamDelegate() {}; */

/* 	void paint(QPainter *painter, const QStyleOptionViewItem &option, */
/* 	           const QModelIndex &index) const override; */
/* 	/1* QSize sizeHint(const QStyleOptionViewItem &option, *1/ */
/* 	/1*                const QModelIndex &index) const override; *1/ */
/* }; */

#endif
