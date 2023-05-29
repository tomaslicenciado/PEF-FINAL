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

QStringList AdminBD::filtrarPais(QString pais, QString tabla)
{

}

QStringList AdminBD::filtrarRegion(QString region, QString tabla)
{

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

bool AdminBD::existe_dato(QString tabla, QString region, QString pais, QString dia)
{
    QString id_region, id_pais;
    QSqlQuery query (bd);
    int contador_de_registros = 0;

    //Buscamos el id de país. Si el país no existe, el dato no existe. Si el país está repetido, da error.
    query.exec("SELECT id FROM paises WHERE pais = '"+pais+"'");
    while( query.next() )  {
        contador_de_registros++;
        if (contador_de_registros > 1){
            QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla paises");
            return true;
        }
        id_pais.append(query.value(0).toString());
    }
    if (contador_de_registros == 0){
        return false;
    }

    //Si la región no está detallada, indica que es sólo el país, por lo que el id_region debe ser 0
    if (region.compare("")==0){
        id_region = "0";
    }
    else {
        contador_de_registros = 0;
        //Buscamos el id de region. Si la region no existe, el dato no existe. Si la region está repetida, da error.
        query.exec("SELECT id FROM regiones WHERE region = '"+region+"'");
        while( query.next() )  {
            contador_de_registros++;
            if (contador_de_registros > 1){
                QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla regiones");
                return true;
            }
            id_region.append(query.value(0).toString());
        }
        if (contador_de_registros == 0){
            return false;
        }
    }

    contador_de_registros = 0;
    //Obtenidos id de pais y region, controlamos el dato específico (fecha, region, pais). NO se controla la cantidad
    query.exec("SELECT id FROM "+tabla+" WHERE id_pais = '"+id_pais+"' AND id_region = '"+id_region+"' AND fecha = '"+dia+"'");
    while( query.next() )  {
        contador_de_registros++;
        if (contador_de_registros > 1){
            QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla "+tabla);
            return true;
        }
    }
    if (contador_de_registros == 0){
        return false;
    }

    return true;
}

bool AdminBD::insertar_dato(QString tabla, QString region, QString pais, QString dia, QString cantidad)
{
    QString id_region, id_pais;
    QSqlQuery query (bd);
    int contador_de_registros = 0;

    //Buscamos el id de país. Si el país no existe, lo insertamos. Si el país está repetido, da error.
    query.exec("SELECT id FROM paises WHERE pais = '"+pais+"'");
    while( query.next() )  {
        contador_de_registros++;
        if (contador_de_registros > 1){
            QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla paises");
            return false;
        }
        id_pais.append(query.value(0).toString());
    }
    if (contador_de_registros == 0){
        query.exec("INSERT INTO paises (pais) VALUES ('"+pais+"')");
        contador_de_registros = 0;
        query.exec("SELECT id FROM paises WHERE pais = '"+pais+"'");
        while( query.next() )  {
            contador_de_registros++;
            if (contador_de_registros > 1){
                QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla paises");
                return false;
            }
            id_pais.append(query.value(0).toString());
        }
        if (contador_de_registros == 0){
            QMessageBox::critical(nullptr,"Error de insercion","No se realizó correctamente el proceso de inserción del país "+pais);
            return false;
        }
    }

    //Si la región no está detallada, indica que es sólo el país, por lo que el id_region debe ser 0
    if (region.compare("")==0){
        id_region = "0";
    }
    else {
        contador_de_registros = 0;
        //Buscamos el id de region. Si la region no existe, el dato no existe. Si la region está repetida, da error.
        query.exec("SELECT id FROM regiones WHERE region = '"+region+"' AND id_pais = '"+id_pais+"'");
        while( query.next() )  {
            contador_de_registros++;
            if (contador_de_registros > 1){
                QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla regiones");
                return false;
            }
            id_region.append(query.value(0).toString());
        }
        if (contador_de_registros == 0){
            query.exec("INSERT INTO regiones (region,id_pais) VALUES ('"+region+"','"+id_pais+"')");
            contador_de_registros = 0;
            query.exec("SELECT id FROM regiones WHERE region = '"+region+"' AND id_pais = '"+id_pais+"'");
            while( query.next() )  {
                contador_de_registros++;
                if (contador_de_registros > 1){
                    QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla regiones");
                    return false;
                }
                id_region.append(query.value(0).toString());
            }
            if (contador_de_registros == 0){
                QMessageBox::critical(nullptr,"Error de insercion","No se realizó correctamente el proceso de inserción de region");
                return false;
            }
        }
    }



    //Obtenidos id de pais y region (insertados si no estaban), insertamos el dato específico (fue controlada su existencia en la función existe_dato, llamada
    //en netcsv.cpp antes de ser llamada esta función insertar_dato
    query.exec("INSERT INTO "+tabla+" (fecha,id_pais,id_region,cant) VALUES ('"+dia+"','"+id_pais+"','"+id_region+"','"+cantidad+"')");

    //Controlamos correcta inserción
    contador_de_registros = 0;
    query.exec("SELECT id FROM "+tabla+" WHERE id_pais = '"+id_pais+"' AND id_region = '"+id_region+"' AND fecha = '"+dia+"'");
    while( query.next() )  {
        contador_de_registros++;
        if (contador_de_registros > 1){
            QMessageBox::critical(nullptr,"Error de Base de Datos","Se encuentra repetido un registro en la tabla "+tabla);
            return false;
        }
    }
    if (contador_de_registros == 0){
        QMessageBox::critical(nullptr,"Error de Inserción","No se realizó correctamente el proceso de inserción de dato");
        return false;
    }
    qDebug() << "Dato insertado con éxito";

    return true;
}
