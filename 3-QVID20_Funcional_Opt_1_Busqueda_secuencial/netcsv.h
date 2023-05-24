#ifndef NETCSV_H
#define NETCSV_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include "adminbd.h"

class NetCSV : public QObject
{
    Q_OBJECT
public:
    explicit NetCSV(QObject *parent = nullptr);
    void agregar_csv(QString csv_url, QString tabla);

private:
    QNetworkAccessManager * manager;
    int current;
    QStringList tablas;
    QStringList urls;
    AdminBD base;
    QList<QByteArray> nombres_campos;
    QList<QByteArray> lineas;

    QList<QStringList> filtrar_datos_existentes();
    int existe_dato(QList<QStringList> tabla, QStringList dato);

private slots:
    void extraer_csv(QNetworkReply* reply);
    void actualizar_bd();

public slots:
    void procesar_siguiente();


signals:
    void csv_cargado();
    void tabla_actualizada();
    void necesario_actualizar();
    void next();
    void finished();

};

#endif // NETCSV_H
