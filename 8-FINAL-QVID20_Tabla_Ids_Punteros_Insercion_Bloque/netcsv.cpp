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

QList<QStringList> NetCSV::filtrar_datos_existentes()
{
    QList<QStringList> datos_a_grabar;
    qDebug() << "Levantando tabla";
    QList<QStringList> tabla = base.obtener_tabla(tablas[current]);
    QList<int> posiciones_paises = obtener_posiciones_paises(&tabla);
    qDebug() << "Tabla en memoria";
    QList< QByteArray > datos_linea;
    QString id_pais = "";
    QString id_region = "";
    QString fecha = "";
    QString cant = "";
    QString pais = "";
    QString region = "";
    QStringList dato;
    int existe = 0;
    int posin = 0, posfin = 0;

    qDebug() << "Trabajando en memoria";
    int cant_lineas = lineas.size()-1;
    for (int i=0; i<cant_lineas; i++){
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
            dato.clear();
            if (pais.compare(datos_linea.at(1)) != 0){
                id_pais.clear();
                id_pais = base.obtener_id_pais(datos_linea.at(1));
                pais = datos_linea.at(1);
            }
            if (region.compare(datos_linea.at(0)) != 0){
                id_region.clear();
                id_region = datos_linea.at(0)!=""?base.obtener_id_region(datos_linea.at(0), id_pais):"";
                region = datos_linea.at(0);
            }
            fecha = nombres_campos.at(j);
            cant = datos_linea.at(j);

            dato.append(fecha);
            dato.append(id_pais);
            dato.append(id_region);
            dato.append(cant);

            if (id_pais.toInt()>posiciones_paises[0]){
                dato.append("I");
                datos_a_grabar.append(dato);
            }
            else{
                posin = posiciones_paises[id_pais.toInt()];
                if (id_pais.toInt()==posiciones_paises[0])
                    posfin = tabla.size();
                else
                    posfin = posiciones_paises[id_pais.toInt() +1];

                existe = existe_dato(&tabla, dato, posin, posfin);
                if (existe == 0){
                    dato.append("I");
                    datos_a_grabar.append(dato);
                }
                else if (existe == 2){
                    dato.append("U");
                    datos_a_grabar.append(dato);
                }
            }

        }
    }
    qDebug() << "Fin del trabajo en memoria";
    return datos_a_grabar;
}

// Devuelve 0 si el dato no existe, 1 si el dato existe exactamente igual o 2 si existe la fecha pero la cantidad es diferente (para actualizar)
//Implementación original con búsqueda lineal
int NetCSV::existe_dato(const QList<QStringList>* tabla, QStringList dato, int posicion_inicial, int posicion_final)
{
    for(int i=posicion_inicial; i<posicion_final;i++) {
        if(tabla->at(i).at(2).compare(dato.at(2))==0 && tabla->at(i).at(0).contains(dato.at(0))){
            if (tabla->at(i).at(3).compare(dato.at(3))==0)
                return 1;
            else
                return 2;
        }
    }
    return 0;
}

QList<int> NetCSV::obtener_posiciones_paises(const QList<QStringList>* tabla)
{
    QList<int> posiciones_paises;

    posiciones_paises.append(0);
    int id_actual = 0, i=0;
    for(; i<tabla->size();i++){
        if(tabla->at(i)[1].toInt()!=id_actual){
            id_actual = tabla->at(i)[1].toInt();
            if(posiciones_paises.size()==id_actual-1)
                posiciones_paises.append(0);
            posiciones_paises.append(i);
        }
    }
    posiciones_paises[0]=id_actual;

    return posiciones_paises;
}

void NetCSV::actualizar_bd()
{
    qDebug() << "Procedemos a grabar los datos en la DB";
    QString tabla = tablas[current];
    qDebug() << QDateTime::currentDateTime();
    QList<QStringList> datos_a_grabar = filtrar_datos_existentes();
    qDebug() << QDateTime::currentDateTime();
    qDebug() << QDateTime::currentDateTime();
    qDebug() << "Conectar a la base para actualizar e insertar";
    qDebug() << "Datos diferentes: " << datos_a_grabar.size();
    base.insertar_actualizar_bloque(tabla, &datos_a_grabar);
//    foreach (const QStringList dato, datos_a_grabar) {
//        if (dato.at(4).compare("I") == 0)
//            base.insertar_dato(tabla,dato.at(2),dato.at(1),dato.at(0),dato.at(3));
//        else
//            base.actualizar_dato(tabla,dato.at(2),dato.at(1),dato.at(0),dato.at(3));
//    }
    current++;
    qDebug() << QDateTime::currentDateTime();
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

        qDebug() << QDateTime::currentDateTime();
        emit finished();
    }
}

