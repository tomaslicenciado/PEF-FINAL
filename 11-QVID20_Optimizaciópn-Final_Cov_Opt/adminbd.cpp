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

bool AdminBD::is_Open()
{
    return bd.isOpen();
}

bool AdminBD::comprobar_tabla(QString tabla)
{
    QStringList tablas = bd.tables();

    return tablas.contains(tabla);
}

bool AdminBD::insertar_actualizar_bloque(const QString tabla, const QList<QStringList>* datos)
{
    QSqlQuery query(bd);
    QString qinsert = "INSERT INTO '"+tabla+"' (fecha,id_pais,id_region,cant) VALUES ";
    QString qupdate = "UPDATE '"+tabla+"' SET cant = CASE ";
    bool ins_datos=false, upd_datos = false;
    foreach (const QStringList dato, *datos) {
        if(dato.at(4).compare("I")==0){
            qinsert.append("('"+dato.at(0)+"','"+dato.at(1)+"','"+dato.at(2)+"','"+dato.at(3)+"'),\n");
            ins_datos = true;
        }
        else{
            qupdate.append("WHEN fecha = '"+dato.at(0)+"' AND id_pais = '"+dato.at(1)+"' AND id_region = '"+dato.at(2)+"' THEN '"+dato.at(3)+"'\n");
            upd_datos = true;
        }
    }
    if (ins_datos){
        qinsert.chop(2);
        query.exec(qinsert);
        if (query.numRowsAffected() <= 0){
            QMessageBox::critical(nullptr,"Error de Inserción","No se realizó correctamente el proceso de inserción del dato");
            return false;
        }
    }
    if (upd_datos){
        qupdate.append("ELSE cant END");
        query.exec(qupdate);
        if (query.numRowsAffected() <= 0){
            QMessageBox::critical(nullptr,"Error de Actualización","No se realizó correctamente el proceso de actualización del dato");
            return false;
        }
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
