#include "netcsv.h"

NetCSV::NetCSV(QObject *parent) : QObject(parent),
                                  manager( new QNetworkAccessManager( this ) )
{
    tabla.clear();
    csv_url.clear();

    base.conectar("../bd/bd.sqlite");
    if (!base.is_Open()){
        QMessageBox::critical(nullptr,"Error","No se pudo abrir la base de datos");
    }

    nombres_campos.clear();
    lineas.clear();

}

bool NetCSV::validar_csv_bd(QString csv_url_str, QString tabla_str)
{
    bool r = true;

    nombres_campos.clear();
    lineas.clear();
    tabla.clear();
    csv_url.clear();


    csv_url.setUrl(csv_url_str);

    if (!base.comprobar_tabla(tabla_str)){
        QMessageBox::critical(nullptr,"Error","No existe la tabla en la base de datos. Compruebe el nombre");
        return false;
    }
    tabla = tabla_str;

    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(extraer_csv(QNetworkReply*)));
    manager->get(QNetworkRequest(csv_url));
    connect(this,SIGNAL(csv_cargado()),this,SLOT(comparar_csv_bd()));
    connect(this,SIGNAL(necesario_actualizar()),this,SLOT(actualizar_bd()));

    return r;
}

void NetCSV::extraer_csv(QNetworkReply *reply)
{
    QByteArray ba = reply->readAll();

    lineas = ba.split('\n');

    if ( ! lineas.isEmpty() )  {
        nombres_campos = lineas.at( 0 ).split( ',' );
        lineas.removeFirst();
    }
    emit csv_cargado();
}

void NetCSV::comparar_csv_bd()
{
    bool r = true;
    QList< QByteArray > datos_linea;
    for (int i=0; i<lineas.size()-1; i++){
        datos_linea.clear();

        if ( lineas.at( i ).contains( "\"" ) && lineas.at( i ).split( ',' ).size() != nombres_campos.size() )  {
            QString linea_con_comillas = lineas.at( i );

            int primer_comilla = linea_con_comillas.indexOf( "\"" );
            int segunda_comilla = linea_con_comillas.indexOf( "\"", primer_comilla + 1 );

            QString cadena_con_coma = linea_con_comillas.mid( primer_comilla, segunda_comilla - primer_comilla );
            cadena_con_coma.replace( ",", "." );

            linea_con_comillas.replace( primer_comilla, segunda_comilla - primer_comilla, cadena_con_coma );

            lineas.replace( i, linea_con_comillas.toUtf8() );
        }
        if (lineas.at(i).contains("\'")){
            QString remplazo = lineas.at(i);
            remplazo.replace("\'"," ");
            lineas.replace(i,remplazo.toUtf8());
        }

        datos_linea = lineas.at(i).split(',');
        for (int j=4; j<datos_linea.size(); j++){
            r = r && base.existe_dato(tabla,datos_linea.at(0),datos_linea.at(1),nombres_campos.at(j));
        }
    }

    if (r){
        QMessageBox::information(nullptr,"Base actualizada","Los datos de la base de datos se encuentran actualizados");
    }
    else{
        QMessageBox::information(nullptr,"Base desactualizada","Hay datos nuevos que no se encuentran en la base.\n Procedemos a actualizar base de datos.");
        emit necesario_actualizar();
    }
}

void NetCSV::actualizar_bd()
{
    QList< QByteArray > datos_linea;
    for (int i=0; i<lineas.size()-1; i++){
        datos_linea.clear();

        if ( lineas.at( i ).contains( "\"" ) && lineas.at( i ).split( ',' ).size() != nombres_campos.size() )  {
            QString linea_con_comillas = lineas.at( i );

            int primer_comilla = linea_con_comillas.indexOf( "\"" );
            int segunda_comilla = linea_con_comillas.indexOf( "\"", primer_comilla + 1 );

            QString cadena_con_coma = linea_con_comillas.mid( primer_comilla, segunda_comilla - primer_comilla );
            cadena_con_coma.replace( ",", "." );

            linea_con_comillas.replace( primer_comilla, segunda_comilla - primer_comilla, cadena_con_coma );

            lineas.replace( i, linea_con_comillas.toUtf8() );
        }

        datos_linea = lineas.at(i).split(',');
        for (int j=4; j<datos_linea.size(); j++){
            if (!base.existe_dato(tabla,datos_linea.at(0),datos_linea.at(1),nombres_campos.at(j))){
                base.insertar_dato(tabla,datos_linea.at(0),datos_linea.at(1),nombres_campos.at(j),datos_linea.at(j));
            }
        }
    }
}

