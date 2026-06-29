#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_ZONAS 100
#define MAX_REGISTROS 31
#define MAX_CONTAMINANTES 7
#define MAX_ALERTAS 500
#define MAX_PREDICCIONES 100

#define MAX_CODIGO 15
#define MAX_NOMBRE 50
#define MAX_TIPO 25
#define MAX_FECHA 12
#define MAX_HORA 10
#define MAX_MENSAJE 150

typedef struct {
    char fecha[MAX_FECHA];
    float co2;
    float so2;
    float pm25;
    float pm10;
    float co;
    float no2;
    float o3;
    float temperatura;
    float viento;
    float humedad;
} DailyRecord;

typedef struct {
    char codigo[MAX_CODIGO];
    char nombre[MAX_NOMBRE];
    char tipo[MAX_TIPO];
    float habitantes;
    DailyRecord registros[MAX_REGISTROS];
    int registros_count;
} Zone;

typedef struct {
    char codigoZone[MAX_CODIGO];
    char nombreZona[MAX_NOMBRE];
    char fecha[MAX_FECHA];
    char hora[MAX_HORA];
    char contaminante[MAX_NOMBRE];
    char abreviatura[10];
    float valor;
    float limite;
    char categoria[30];
    char recomendacion[MAX_MENSAJE];
} Alerta;

typedef struct {
    char codigoZona[MAX_CODIGO];
    char nombreZona[MAX_NOMBRE];
    float co2;
    float so2;
    float pm25;
    float pm10;
    float co;
    float no2;
    float o3;
    float temperatura;
    float viento;
    float humedad;
    char nivel[30];
    char mensaje[MAX_MENSAJE];
} Prediction;

typedef struct {
    char nombre[MAX_NOMBRE];
    char abreviatura[10];
    char unidad[15];
    float limite;
    float limiteAlerta;
    char descripcion[100];
    char recomendacion[MAX_MENSAJE];
} ContaminanteReferencia;

typedef struct {
    ContaminanteReferencia contaminantes[MAX_CONTAMINANTES];
    char clasificaciones[5][20];
    float umbralTemperaturaBaja;
    float umbralTemperaturaNormal;
    float umbralTemperaturaAlta;
    float umbralTemperaturaMuyAlta;
    float umbralHumedadBaja;
    float umbralHumedadNormal;
    float umbralHumedadAlta;
    float umbralHumedadMuyAlta;
    float umbralVientoCalma;
    float umbralVientoDebil;
    float umbralVientoModerado;
    float umbralVientoFuerte;
} ReferenciaSistema;

void inicializarReferencia(ReferenciaSistema *referencia, const char *archivo);
void limpiarBuffer(void);
void trim(char *cadena);
void normalizarTexto(char *cadena);
int validarVacio(const char *cadena);
int validarSoloLetras(const char *cadena);
int validarSoloNumeros(const char *cadena);
int validarNumeroDecimal(const char *cadena);
int validarRangoReal(float valor, float min, float max);
int validarRangoEntero(int valor, int min, int max);

void cargarZonasPorDefecto(Zone zonas[], int *cantidad);
void cargarDatosDesdeArchivo(Zone zonas[], int *cantidad, Alerta alertas[], int *alertas_count, Prediction predicciones[], int *predicciones_count, const char *zonas_file, const char *alertas_file, const char *predicciones_file);
void guardarDatosEnArchivo(const Zone zonas[], int cantidad, const Alerta alertas[], int alertas_count, const Prediction predicciones[], int predicciones_count, const char *zonas_file, const char *alertas_file, const char *predicciones_file);
void exportarReporte(const Zone zonas[], int cantidad, const Alerta alertas[], int alertas_count, const Prediction predicciones[], int predicciones_count, const char *archivo);

void leerTexto(char *dest, size_t tam, const char *mensaje);
void leerFecha(char *dest, size_t tam, const char *mensaje);
void leerNumero(float *dest, float min, float max, const char *mensaje);
void leerEntero(int *dest, int min, int max, const char *mensaje);

void agregarZona(Zone zonas[], int *cantidad);
void ingresarRegistroDiario(Zone *zona);
float calcularPromedioPonderado(const Zone *zona, int tipo_contaminante);
float calcularIndiceCompuesto(const Zone *zona);
const char *clasificarCalidad(float indice);

void mostrarMonitoreoActual(const Zone zonas[], int cantidad);
void mostrarPromediosHistoricos(const Zone zonas[], int cantidad);
void generarPrediccion(const Zone *zona, Prediction *prediccion);
void generarAlertas(const Zone zonas[], int cantidad, Alerta alertas[], int *alertas_count, Prediction predicciones[], int *predicciones_count);
void mostrarPredicciones(const Zone zonas[], int cantidad, const Prediction predicciones[], int predicciones_count);
void mostrarAlertas(const Alerta alertas[], int alertas_count);
void consultarZona(const Zone zonas[], int cantidad, const Prediction predicciones[], int predicciones_count);
void mostrarTodasZonasOrdenadas(const Zone zonas[], int cantidad);
void mostrarEstadisticas(const Zone zonas[], int cantidad, const Alerta alertas[], int alertas_count);
void modificarZona(Zone zonas[], int cantidad);
void eliminarZona(Zone zonas[], int *cantidad);

void mostrarMenu(void);
void ejecutarMenu(Zone zonas[], int *cantidad, Alerta alertas[], int *alertas_count, Prediction predicciones[], int *predicciones_count);

#endif