#include "netcsv.h"
#include <QUrl>

NetCSV::NetCSV(QObject *parent) : QObject(parent),
                                  manager( new QNetworkAccessManager( this ) )
{
    tablas.clear();
    urls.clear();

    base.conectar("../bd/bd.sqlite");
    if (!base.is_Open()){
        QMessageBox::critical(nullptr,"Error","No se pudo abrir la base de datos");
    }

    nombres_campos.clear();
    lineas.clear();
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(extraer_csv(QNetworkReply*)));
    connect(this,SIGNAL(csv_cargado()),this,SLOT(actualizar_bd()));
    connect(this,SIGNAL(next()),this,SLOT(procesar_siguiente()));
    current = 0;
}

void NetCSV::agregar_csv(QString csv_url, QString tabla)
{
    tablas.append(tabla);
    urls.append(csv_url);
}

void NetCSV::extraer_csv(QNetworkReply *reply)
{
    qDebug() << "Cargando datos en memoria";
    QByteArray ba = reply->readAll();

    lineas = ba.split('\n');

    if ( ! lineas.isEmpty() )  {
        nombres_campos = lineas.at( 0 ).split( ',' );
        lineas.removeFirst();
    }
    qDebug() << "Datos cargados en memoria";
    emit csv_cargado();
}

void NetCSV::actualizar_bd()
{
    qDebug() << "Procedemos a grabar los datos en la DB";
    QString tabla = tablas[current];
    QList< QByteArray > datos_linea;
    for (int i=0; i<lineas.size()-1; i++){
        datos_linea.clear();
        if (lineas.at(i).contains("'")){
            QString l = lineas.at(i);
            l.replace("'", " ");
            lineas.replace(i, l.toUtf8());
        }

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
        int cant_datos = datos_linea.size();
        for (int j=4; j<cant_datos; j++){
            if (!base.existe_dato(tabla,datos_linea.at(0),datos_linea.at(1),nombres_campos.at(j))){
                base.insertar_dato(tabla,datos_linea.at(0),datos_linea.at(1),nombres_campos.at(j),datos_linea.at(j));
            }
        }
    }
    current++;
    emit next();
}

void NetCSV::procesar_siguiente()
{
    if (tablas.length() != urls.length()){
        QMessageBox::critical(nullptr,"Error","La cantidad de tablas a revisar es distinta a la cantidad de csv seleccionados");
    }
    else if (current < tablas.length()){
        qDebug() << "Iniciando proceso";
        QString csv_url_str = urls[current];
        QString tabla_str = tablas[current];

        nombres_campos.clear();
        lineas.clear();
        QUrl csv_url;


        csv_url.setUrl(csv_url_str);

        if (!base.comprobar_tabla(tabla_str)){
            QMessageBox::critical(nullptr,"Error","No existe la tabla en la base de datos. Compruebe el nombre");
        }
        else {
            manager->get(QNetworkRequest(csv_url));
            qDebug() << "Leyendo datos";
        }
    }
    else {
        qDebug() << "Datos controlados en su totalidad";
        emit finished();
    }
}

