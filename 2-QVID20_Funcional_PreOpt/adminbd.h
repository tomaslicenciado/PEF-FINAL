#ifndef ADMINBD_H
#define ADMINBD_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

class AdminBD : public QObject
{
    Q_OBJECT
public:
    explicit AdminBD(QObject *parent = nullptr);
    ~AdminBD();

    bool conectar(QString archivoBd);
    QSqlDatabase getBd() const;

    bool is_Open();
    bool comprobar_tabla(QString tabla);
    bool existe_dato(QString tabla, QString region, QString pais, QString dia);
    bool insertar_dato(QString tabla, QString region, QString pais, QString dia, QString cantidad);

private:
    QSqlDatabase bd;

public slots:

signals:

};

#endif // ADMINBD_H
