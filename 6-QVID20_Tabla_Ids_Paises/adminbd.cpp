#include "adminbd.h"

AdminBD::AdminBD(QObject *parent) : QObject(parent)
{
    bd = QSqlDatabase::addDatabase( "QSQLITE" );
}

AdminBD::~AdminBD()
{
    if (bd.isOpen()){
        bd.close();
    }
}

bool AdminBD::conectar(QString archivoBd)
{
    QFileInfo arch(archivoBd);
    if (!(arch.exists() && arch.isFile())){
        QMessageBox::warning(nullptr,"Advertencia","El archivo de base de datos no existe. Se creará " + archivoBd );
    }

    bd.setDatabaseName(archivoBd);

    if (bd.open()){
        return true;
    }
    else {
        QMessageBox::critical(nullptr,"Error","Error en la apertura de la base de datos");
        return false;
    }
}

QSqlDatabase AdminBD::getBd() const
{
    return bd;
}

bool AdminBD::is_Open()
{
    return bd.isOpen();
}

bool AdminBD::comprobar_tabla(QString tabla)
{
    QStringList tablas = bd.tables();

    return tablas.contains(tabla);
}

bool AdminBD::insertar_dato(QString tabla, QString id_region, QString id_pais, QString dia, QString cantidad)
{
    QSqlQuery query(bd);
    QString qinsert = "INSERT INTO '"+tabla+"' (fecha,id_pais,id_region,cant) VALUES ('"+dia+"','"+id_pais+"','"+id_region+"','"+cantidad+"')";
    query.exec(qinsert);
    if (query.numRowsAffected() <= 0){
        QMessageBox::critical(nullptr,"Error de Inserción","No se realizó correctamente el proceso de inserción del dato");
        return false;
    }
    return true;
}

bool AdminBD::actualizar_dato(QString tabla, QString id_region, QString id_pais, QString dia, QString cantidad)
{
    QSqlQuery query(bd);
    QString qupdate = "UPDATE '"+tabla+"' SET cant = '"+cantidad+"' WHERE fecha = '"+dia+"' AND id_pais = '"+id_pais+"' AND id_region = '"+id_region+"'";
    query.exec(qupdate);
    if (query.numRowsAffected() <= 0){
        QMessageBox::critical(nullptr,"Error de Actualización","No se realizó correctamente el proceso de actualización del dato");
        return false;
    }
    return true;
}

QList<QStringList> AdminBD::obtener_tabla(QString tabla)
{
    QList<QStringList> resultado;
    QSqlQuery query(bd);

    query.exec("SELECT * FROM "+tabla);

    while (query.next()){
        QStringList fila;
        fila.append(query.value("fecha").toString());
        fila.append(query.value("id_pais").toString());
        fila.append(query.value("id_region").toString());
        fila.append(query.value("cant").toString());
        resultado.append(fila);
    }

    return resultado;
}

QString AdminBD::obtener_id_pais(QString pais)
{
    QString id_pais = "";
    QSqlQuery query(bd);

    query.exec("SELECT id FROM paises WHERE pais = '"+pais+"'");
    if (query.next()){
        id_pais = query.value(0).toString();
    }
    else {
        query.exec("INSERT INTO paises (pais) VALUES ('"+pais+"')");
        query.exec("SELECT id FROM paises WHERE pais = '"+pais+"'");
        if (query.next()){
            id_pais = query.value(0).toString();
        }
    }

    return id_pais;
}

QString AdminBD::obtener_id_region(QString region, QString id_pais)
{
    QString id_region = "";
    QSqlQuery query(bd);

    query.exec("SELECT id FROM regiones WHERE region = '"+region+"' AND id_pais = '"+id_pais+"'");
    if (query.next()){
        id_region = query.value(0).toString();
    }
    else{
        query.exec("INSERT INTO regiones (region, id_pais) VALUES ('"+region+"','"+id_pais+"')");
        query.exec("SELECT id FROM regiones WHERE region = '"+region+"' AND id_pais = '"+id_pais+"'");
        if (query.next()){
            id_region = query.value(0).toString();
        }
    }

    return id_region;
}
