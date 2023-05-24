#ifndef NETCSV_H
#define NETCSV_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>
#include "adminbd.h"

class NetCSV : public QObject
{
    Q_OBJECT
public:
    explicit NetCSV(QObject *parent = nullptr);

    bool validar_csv_bd(QString csv_url, QString tabla);

private:
    QNetworkAccessManager * manager;
    QString tabla;
    QUrl csv_url;
    AdminBD base;
    QList<QByteArray> nombres_campos;
    QList<QByteArray> lineas;

private slots:
    void extraer_csv(QNetworkReply* reply);
    void comparar_csv_bd();
    void actualizar_bd();

public slots:


signals:
    void csv_cargado();
    void tabla_actualizada();
    void necesario_actualizar();

};

#endif // NETCSV_H
