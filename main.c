#include "funciones.h"

int main(void) {
    Zone zonas[MAX_ZONAS];
    int cantidad_zonas = 0;

    Alerta alertas[MAX_ALERTAS];
    int cantidad_alertas = 0;

    Prediction predicciones[MAX_PREDICCIONES];
    int cantidad_predicciones = 0;

    ReferenciaSistema referencia;

    inicializarReferencia(&referencia, "referencia.txt");
    cargarZonasPorDefecto(zonas, &cantidad_zonas);
    cargarDatosDesdeArchivo(zonas, &cantidad_zonas, alertas, &cantidad_alertas, predicciones, &cantidad_predicciones, "zonas.dat", "alertas.dat", "predicciones.dat");

    ejecutarMenu(zonas, &cantidad_zonas, alertas, &cantidad_alertas, predicciones, &cantidad_predicciones);

    return 0;
}