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

    bool is_Open();
    bool comprobar_tabla(QString tabla);
    QList<QStringList> obtener_tabla(QString tabla);
    QString obtener_id_pais(QString pais);
    QString obtener_id_region(QString region, QString id_pais);
    bool insertar_actualizar_bloque(const QString tabla, const QList<QStringList>* datos);


private:
    QSqlDatabase bd;

public slots:

signals:

};

#endif // ADMINBD_H
