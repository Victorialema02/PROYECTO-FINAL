#include "funciones.h"

static ReferenciaSistema g_referencia;

static void inicializarContaminante(ContaminanteReferencia *contaminante, const char *nombre, const char *abreviatura, const char *unidad, float limite, float limiteAlerta, const char *descripcion, const char *recomendacion) {
    strncpy(contaminante->nombre, nombre, sizeof(contaminante->nombre) - 1);
    contaminante->nombre[sizeof(contaminante->nombre) - 1] = '\0';
    strncpy(contaminante->abreviatura, abreviatura, sizeof(contaminante->abreviatura) - 1);
    contaminante->abreviatura[sizeof(contaminante->abreviatura) - 1] = '\0';
    strncpy(contaminante->unidad, unidad, sizeof(contaminante->unidad) - 1);
    contaminante->unidad[sizeof(contaminante->unidad) - 1] = '\0';
    contaminante->limite = limite;
    contaminante->limiteAlerta = limiteAlerta;
    strncpy(contaminante->descripcion, descripcion, sizeof(contaminante->descripcion) - 1);
    contaminante->descripcion[sizeof(contaminante->descripcion) - 1] = '\0';
    strncpy(contaminante->recomendacion, recomendacion, sizeof(contaminante->recomendacion) - 1);
    contaminante->recomendacion[sizeof(contaminante->recomendacion) - 1] = '\0';
}

static int extraerUltimoFloatDeLinea(const char *linea, float *valor) {
    const char *p = linea;
    float ultimo = 0.0f;
    int encontrado = 0;

    while (*p != '\0') {
        if (isdigit((unsigned char)*p) || *p == '-' || *p == '.') {
            char *fin = NULL;
            float numero = strtof(p, &fin);
            if (fin != p) {
                ultimo = numero;
                encontrado = 1;
                p = fin;
                continue;
            }
        }
        p++;
    }

    if (encontrado) {
        *valor = ultimo;
        return 1;
    }

    return 0;
}

static const ContaminanteReferencia *buscarContaminantePorAbreviatura(const char *abreviatura) {
    if (abreviatura == NULL) {
        return NULL;
    }

    for (int i = 0; i < MAX_CONTAMINANTES; i++) {
        if (strcmp(g_referencia.contaminantes[i].abreviatura, abreviatura) == 0) {
            return &g_referencia.contaminantes[i];
        }
    }

    return NULL;
}

static float obtenerLimiteContaminante(const char *abreviatura) {
    const ContaminanteReferencia *contaminante = buscarContaminantePorAbreviatura(abreviatura);
    return contaminante != NULL ? contaminante->limite : 0.0f;
}

static float obtenerLimiteAlertaContaminante(const char *abreviatura) {
    const ContaminanteReferencia *contaminante = buscarContaminantePorAbreviatura(abreviatura);
    return contaminante != NULL ? contaminante->limiteAlerta : 0.0f;
}

static const char *obtenerNombreContaminante(const char *abreviatura) {
    const ContaminanteReferencia *contaminante = buscarContaminantePorAbreviatura(abreviatura);
    return contaminante != NULL ? contaminante->nombre : "Contaminante";
}

const char *obtenerRecomendacion(const char *tipoZona, const char *abreviatura) {
    const ContaminanteReferencia *contaminante = buscarContaminantePorAbreviatura(abreviatura);
    if (contaminante != NULL && contaminante->recomendacion[0] != '\0') {
        return contaminante->recomendacion;
    }

    if (strcmp(tipoZona, "Industrial") == 0) {
        return "Reducir emisiones industriales y activar filtros.";
    }
    if (strcmp(tipoZona, "Residencial") == 0) {
        return "Evitar actividades al aire libre y mantener ventanas cerradas.";
    }

    return "Mantener monitoreo continuo y reforzar medidas preventivas.";
}

static void mostrarSeparador(char caracter, int longitud) {
    for (int i = 0; i < longitud; i++) {
        putchar(caracter);
    }
    putchar('\n');
}

static void mostrarEncabezado(const char *titulo) {
    int longitud = 64;
    int espacios = (longitud - (int)strlen(titulo)) / 2;
    mostrarSeparador('=', longitud);
    printf("%*s%s%*s\n", espacios, "", titulo, longitud - espacios - (int)strlen(titulo), "");
    mostrarSeparador('=', longitud);
}

void inicializarReferencia(ReferenciaSistema *referencia, const char *archivo) {
    char linea[400];
    FILE *fp = NULL;
    int indiceActual = -1;
    int recomendacionActiva = 0;

    memset(referencia, 0, sizeof(*referencia));
    strcpy(referencia->clasificaciones[0], "Buena");
    strcpy(referencia->clasificaciones[1], "Moderada");
    strcpy(referencia->clasificaciones[2], "Mala");
    strcpy(referencia->clasificaciones[3], "Muy Mala");
    strcpy(referencia->clasificaciones[4], "Peligrosa");

    inicializarContaminante(&referencia->contaminantes[0], "Dioxido de Carbono", "CO2", "mg/m3", 35000.0f, 35000.0f, "Gas de combustion industrial", "Incrementar la ventilacion y controlar fuentes de emision.");
    inicializarContaminante(&referencia->contaminantes[1], "Monoxido de Carbono", "CO", "mg/m3", 4.0f, 4.0f, "Gas toxico de combustion", "Mejorar la ventilacion y revisar fuentes de combustion.");
    inicializarContaminante(&referencia->contaminantes[2], "Dioxido de Azufre", "SO2", "ug/m3", 40.0f, 40.0f, "Gas industrial", "Reducir emisiones industriales y monitorear procesos.");
    inicializarContaminante(&referencia->contaminantes[3], "Dioxido de Nitrogeno", "NO2", "ug/m3", 25.0f, 25.0f, "Contaminante vehicular", "Disminuir el trafico vehicular y priorizar transporte limpio.");
    inicializarContaminante(&referencia->contaminantes[4], "Ozono Troposferico", "O3", "ug/m3", 100.0f, 100.0f, "Contaminante fotoquimico", "Evitar actividad fisica intensa en exteriores.");
    inicializarContaminante(&referencia->contaminantes[5], "Material Particulado Fino", "PM2.5", "ug/m3", 15.0f, 15.0f, "Particulas menores a 2.5 micras", "Evitar actividades al aire libre y utilizar mascarilla.");
    inicializarContaminante(&referencia->contaminantes[6], "Material Particulado Grueso", "PM10", "ug/m3", 45.0f, 45.0f, "Particulas menores a 10 micras", "Reducir exposicion al polvo y mantener ventanas cerradas.");

    referencia->umbralTemperaturaBaja = 10.0f;
    referencia->umbralTemperaturaNormal = 25.0f;
    referencia->umbralTemperaturaAlta = 35.0f;
    referencia->umbralTemperaturaMuyAlta = 35.0f;
    referencia->umbralHumedadBaja = 30.0f;
    referencia->umbralHumedadNormal = 70.0f;
    referencia->umbralHumedadAlta = 90.0f;
    referencia->umbralHumedadMuyAlta = 90.0f;
    referencia->umbralVientoCalma = 5.0f;
    referencia->umbralVientoDebil = 20.0f;
    referencia->umbralVientoModerado = 40.0f;
    referencia->umbralVientoFuerte = 40.0f;

    fp = fopen(archivo, "r");
    if (fp != NULL) {
        while (fgets(linea, sizeof(linea), fp) != NULL) {
            char *lineaLimpia = linea;
            while (*lineaLimpia == ' ' || *lineaLimpia == '\t') {
                lineaLimpia++;
            }
            lineaLimpia[strcspn(lineaLimpia, "\r\n")] = '\0';

            if (strstr(lineaLimpia, "Abreviatura: CO2") != NULL || strstr(lineaLimpia, "Abreviatura: CO₂") != NULL) {
                indiceActual = 0;
            } else if (strstr(lineaLimpia, "Abreviatura: CO") != NULL) {
                indiceActual = 1;
            } else if (strstr(lineaLimpia, "Abreviatura: SO2") != NULL) {
                indiceActual = 2;
            } else if (strstr(lineaLimpia, "Abreviatura: NO2") != NULL) {
                indiceActual = 3;
            } else if (strstr(lineaLimpia, "Abreviatura: O3") != NULL || strstr(lineaLimpia, "Abreviatura: O₃") != NULL) {
                indiceActual = 4;
            } else if (strstr(lineaLimpia, "Abreviatura: PM2.5") != NULL) {
                indiceActual = 5;
            } else if (strstr(lineaLimpia, "Abreviatura: PM10") != NULL) {
                indiceActual = 6;
            } else if (indiceActual >= 0 && (strstr(lineaLimpia, "mg/m") != NULL || strstr(lineaLimpia, "µg/m") != NULL || strstr(lineaLimpia, "μg/m") != NULL || strstr(lineaLimpia, "ug/m") != NULL)) {
                float valor = 0.0f;
                if (extraerUltimoFloatDeLinea(lineaLimpia, &valor)) {
                    referencia->contaminantes[indiceActual].limite = valor;
                    referencia->contaminantes[indiceActual].limiteAlerta = valor;
                }
            }

            if (strstr(lineaLimpia, "Baja:") != NULL && strstr(lineaLimpia, "°C") != NULL) {
                float valor = 0.0f;
                if (extraerUltimoFloatDeLinea(lineaLimpia, &valor)) {
                    referencia->umbralTemperaturaBaja = valor;
                }
            } else if (strstr(lineaLimpia, "Normal:") != NULL && strstr(lineaLimpia, "°C") != NULL) {
                float valor = 0.0f;
                if (extraerUltimoFloatDeLinea(lineaLimpia, &valor)) {
                    referencia->umbralTemperaturaNormal = valor;
                }
            } else if (strstr(lineaLimpia, "Alta:") != NULL && strstr(lineaLimpia, "°C") != NULL) {
                float valor = 0.0f;
                if (extraerUltimoFloatDeLinea(lineaLimpia, &valor)) {
                    referencia->umbralTemperaturaAlta = valor;
                }
            } else if (strstr(lineaLimpia, "Muy Alta:") != NULL && strstr(lineaLimpia, "°C") != NULL) {
                float valor = 0.0f;
                if (extraerUltimoFloatDeLinea(lineaLimpia, &valor)) {
                    referencia->umbralTemperaturaMuyAlta = valor;
                }
            }

            if (strstr(lineaLimpia, "PM2.5 alto") != NULL || strstr(lineaLimpia, "PM10 alto") != NULL || strstr(lineaLimpia, "CO alto") != NULL || strstr(lineaLimpia, "SO2 alto") != NULL || strstr(lineaLimpia, "NO2 alto") != NULL || strstr(lineaLimpia, "O3 alto") != NULL || strstr(lineaLimpia, "CO2 alto") != NULL) {
                recomendacionActiva = 1;
                continue;
            }

            if (recomendacionActiva && (lineaLimpia[0] == '-' || lineaLimpia[0] == '*')) {
                char *texto = lineaLimpia;
                if (*texto == '-' || *texto == '*') {
                    texto++;
                }
                while (*texto == ' ' || *texto == '\t') {
                    texto++;
                }
                if (indiceActual >= 0 && strlen(texto) > 0) {
                    size_t restante = sizeof(referencia->contaminantes[indiceActual].recomendacion) - strlen(referencia->contaminantes[indiceActual].recomendacion) - 1;
                    if (restante > 0) {
                        if (strlen(referencia->contaminantes[indiceActual].recomendacion) > 0) {
                            strncat(referencia->contaminantes[indiceActual].recomendacion, " | ", restante);
                            restante = sizeof(referencia->contaminantes[indiceActual].recomendacion) - strlen(referencia->contaminantes[indiceActual].recomendacion) - 1;
                        }
                        strncat(referencia->contaminantes[indiceActual].recomendacion, texto, restante);
                    }
                }
            } else if (recomendacionActiva && lineaLimpia[0] == '\0') {
                recomendacionActiva = 0;
            }
        }
        fclose(fp);
    }

    g_referencia = *referencia;
}

void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void trim(char *cadena) {
    char *inicio = cadena;
    char *fin;

    while (*inicio == ' ') {
        inicio++;
    }

    if (*inicio == '\0') {
        cadena[0] = '\0';
        return;
    }

    fin = inicio + strlen(inicio);
    while (fin > inicio && *(fin - 1) == ' ') {
        fin--;
    }

    *fin = '\0';
    memmove(cadena, inicio, (size_t)(fin - inicio) + 1);
}

void normalizarTexto(char *cadena) {
    for (int i = 0; cadena[i] != '\0'; i++) {
        cadena[i] = (char)tolower((unsigned char)cadena[i]);
    }
}

int validarVacio(const char *cadena) {
    if (cadena == NULL) {
        return 0;
    }

    if (cadena[0] == '\0') {
        return 0;
    }

    for (int i = 0; cadena[i] != '\0'; i++) {
        if (cadena[i] != ' ') {
            return 1;
        }
    }

    return 0;
}

int validarSoloLetras(const char *cadena) {
    if (!validarVacio(cadena)) {
        return 0;
    }

    for (int i = 0; cadena[i] != '\0'; i++) {
        unsigned char c = (unsigned char)cadena[i];
        if (!isalpha(c) && c != ' ') {
            return 0;
        }
    }

    return 1;
}

int validarSoloNumeros(const char *cadena) {
    if (!validarVacio(cadena)) {
        return 0;
    }

    for (int i = 0; cadena[i] != '\0'; i++) {
        if (!isdigit((unsigned char)cadena[i])) {
            return 0;
        }
    }

    return 1;
}

int validarNumeroDecimal(const char *cadena) {
    int i = 0;
    int puntos = 0;

    if (!validarVacio(cadena)) {
        return 0;
    }

    if (cadena[0] == '+' || cadena[0] == '-') {
        i = 1;
    }

    for (; cadena[i] != '\0'; i++) {
        if (isdigit((unsigned char)cadena[i])) {
            continue;
        }

        if (cadena[i] == '.' && puntos == 0) {
            puntos = 1;
            continue;
        }

        return 0;
    }

    return 1;
}

int validarRangoReal(float valor, float min, float max) {
    return valor >= min && valor <= max;
}

int validarRangoEntero(int valor, int min, int max) {
    return valor >= min && valor <= max;
}

static void inicializarRegistroBase(DailyRecord *registro, int indiceZona) {
    static const float valoresBase[5][10] = {
        {15.0f, 20.0f, 2.1f, 10.0f, 2.5f, 18.0f, 45.0f, 22.0f, 15.0f, 65.0f},
        {30.0f, 35.0f, 3.8f, 22.0f, 3.2f, 28.0f, 65.0f, 20.0f, 18.0f, 58.0f},
        {18.0f, 28.0f, 2.8f, 14.0f, 2.8f, 20.0f, 55.0f, 23.0f, 14.0f, 68.0f},
        {14.0f, 22.0f, 2.0f, 8.0f, 2.2f, 16.0f, 40.0f, 24.0f, 12.0f, 70.0f},
        {20.0f, 30.0f, 3.0f, 16.0f, 2.9f, 22.0f, 60.0f, 21.0f, 16.0f, 62.0f}
    };

    memset(registro, 0, sizeof(*registro));
    strncpy(registro->fecha, "28/06/2026", MAX_FECHA - 1);
    registro->fecha[MAX_FECHA - 1] = '\0';
    registro->co2 = valoresBase[indiceZona][0];
    registro->so2 = valoresBase[indiceZona][1];
    registro->pm25 = valoresBase[indiceZona][2];
    registro->pm10 = valoresBase[indiceZona][3];
    registro->co = valoresBase[indiceZona][4];
    registro->no2 = valoresBase[indiceZona][5];
    registro->o3 = valoresBase[indiceZona][6];
    registro->temperatura = valoresBase[indiceZona][7];
    registro->viento = valoresBase[indiceZona][8];
    registro->humedad = valoresBase[indiceZona][9];
}

void cargarZonasPorDefecto(Zone zonas[], int *cantidad) {
    const char *codigos[] = {"Z001", "Z002", "Z003", "Z004", "Z005"};
    const char *nombres[] = {"Carcelen", "Calderon", "Valle de los Chillos", "Tumbaco", "Guamani"};
    const char *tipos[] = {"Residencial", "Industrial", "Residencial", "Residencial", "Comercial"};
    const float habitantes[] = {85000.0f, 95000.0f, 120000.0f, 75000.0f, 60000.0f};

    *cantidad = 5;
    for (int i = 0; i < *cantidad; i++) {
        memset(&zonas[i], 0, sizeof(Zone));
        strncpy(zonas[i].codigo, codigos[i], MAX_CODIGO - 1);
        zonas[i].codigo[MAX_CODIGO - 1] = '\0';
        strncpy(zonas[i].nombre, nombres[i], MAX_NOMBRE);
        zonas[i].nombre[MAX_NOMBRE] = '\0';
        strncpy(zonas[i].tipo, tipos[i], MAX_TIPO - 1);
        zonas[i].tipo[MAX_TIPO - 1] = '\0';
        zonas[i].habitantes = habitantes[i];
        inicializarRegistroBase(&zonas[i].registros[0], i);
        zonas[i].registros_count = 1;
    }
}

void cargarDatosDesdeArchivo(Zone zonas[], int *cantidad, Alerta alertas[], int *alertas_count, Prediction predicciones[], int *predicciones_count, const char *zonas_file, const char *alertas_file, const char *predicciones_file) {
    FILE *fp = fopen(zonas_file, "rb");
    if (fp != NULL) {
        int count_temp = 0;
        if (fread(&count_temp, sizeof(int), 1, fp) == 1 && count_temp > 0 && count_temp <= MAX_ZONAS) {
            if (fread(zonas, sizeof(Zone), count_temp, fp) == (size_t)count_temp) {
                *cantidad = count_temp;
            }
        }
        fclose(fp);
    }

    fp = fopen(alertas_file, "rb");
    if (fp != NULL) {
        int count_temp = 0;
        if (fread(&count_temp, sizeof(int), 1, fp) == 1 && count_temp >= 0 && count_temp <= MAX_ALERTAS) {
            if (fread(alertas, sizeof(Alerta), count_temp, fp) == (size_t)count_temp) {
                *alertas_count = count_temp;
            }
        }
        fclose(fp);
    }

    fp = fopen(predicciones_file, "rb");
    if (fp != NULL) {
        int count_temp = 0;
        if (fread(&count_temp, sizeof(int), 1, fp) == 1 && count_temp >= 0 && count_temp <= MAX_PREDICCIONES) {
            if (fread(predicciones, sizeof(Prediction), count_temp, fp) == (size_t)count_temp) {
                *predicciones_count = count_temp;
            }
        }
        fclose(fp);
    }
}

void guardarDatosEnArchivo(const Zone zonas[], int cantidad, const Alerta alertas[], int alertas_count, const Prediction predicciones[], int predicciones_count, const char *zonas_file, const char *alertas_file, const char *predicciones_file) {
    FILE *fp = fopen(zonas_file, "wb");
    if (fp != NULL) {
        fwrite(&cantidad, sizeof(int), 1, fp);
        fwrite(zonas, sizeof(Zone), cantidad, fp);
        fclose(fp);
    }

    fp = fopen(alertas_file, "wb");
    if (fp != NULL) {
        fwrite(&alertas_count, sizeof(int), 1, fp);
        fwrite(alertas, sizeof(Alerta), alertas_count, fp);
        fclose(fp);
    }

    fp = fopen(predicciones_file, "wb");
    if (fp != NULL) {
        fwrite(&predicciones_count, sizeof(int), 1, fp);
        fwrite(predicciones, sizeof(Prediction), predicciones_count, fp);
        fclose(fp);
    }
}

void exportarReporte(const Zone zonas[], int cantidad, const Alerta alertas[], int alertas_count, const Prediction predicciones[], int predicciones_count, const char *archivo) {
    FILE *fp = fopen(archivo, "w");
    if (fp == NULL) {
        printf("No fue posible exportar el reporte.\n");
        return;
    }

    fprintf(fp, "+==========================================================================+\n");
    fprintf(fp, "|                     REPORTE GENERAL DEL SISTEMA AMBIENTAL                |\n");
    fprintf(fp, "+==========================================================================+\n\n");

    fprintf(fp, "+--------------------------------------------------------------------------+\n");
    fprintf(fp, "|                    GLOSARIO DE COMPONENTES AMBIENTALES                   |\n");
    fprintf(fp, "+------------+----------------------------------+--------------------------+\n");
    fprintf(fp, "| Componente | Nombre Completo                  | Unidad de Medida         |\n");
    fprintf(fp, "+------------+----------------------------------+--------------------------+\n");
    fprintf(fp, "| CO2        | Dioxido de Carbono               | mg/m3 (Miligramos)       |\n");
    fprintf(fp, "| SO2        | Dioxido de Azufre                | ug/m3 (Microgramos)      |\n");
    fprintf(fp, "| PM2.5      | Material Particulado Fino        | ug/m3 (Microgramos)      |\n");
    fprintf(fp, "| PM10       | Material Particulado Grueso      | ug/m3 (Microgramos)      |\n");
    fprintf(fp, "| CO         | Monoxido de Carbono              | mg/m3 (Miligramos)       |\n");
    fprintf(fp, "| NO2        | Dioxido de Nitrogeno             | ug/m3 (Microgramos)      |\n");
    fprintf(fp, "| O3         | Ozono Troposferico               | ug/m3 (Microgramos)      |\n");
    fprintf(fp, "+------------+----------------------------------+--------------------------+\n\n");

    fprintf(fp, "Resumen General:\n");
    fprintf(fp, "  - Zonas monitoreadas   : %d\n", cantidad);
    fprintf(fp, "  - Alertas acumuladas   : %d\n", alertas_count);
    fprintf(fp, "  - Predicciones activas : %d\n\n", predicciones_count);

    fprintf(fp, "+--------------------------------------------------------------------------------------------------------------------+\n");
    fprintf(fp, "|                                           ESTADO ACTUAL DE LAS ZONAS URBANAS                                       |\n");
    fprintf(fp, "+----------+----------------------+-------------+------------+--------+--------+--------+--------+--------+--------+--------+\n");
    fprintf(fp, "| Codigo   | Nombre Zona          | Tipo        | Habitantes | CO2    | SO2    | PM2.5  | PM10   | CO     | NO2    | O3     |\n");
    fprintf(fp, "+----------+----------------------+-------------+------------+--------+--------+--------+--------+--------+--------+--------+\n");
    for (int i = 0; i < cantidad; i++) {
        const Zone *zona = &zonas[i];
        const DailyRecord *ultimo = (zona->registros_count > 0) ? &zona->registros[zona->registros_count - 1] : NULL;
        fprintf(fp, "| %-8s | %-20s | %-11s | %-10.0f | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-6.1f |\n",
                zona->codigo, zona->nombre, zona->tipo, zona->habitantes,
                ultimo != NULL ? ultimo->co2 : 0.0f, ultimo != NULL ? ultimo->so2 : 0.0f,
                ultimo != NULL ? ultimo->pm25 : 0.0f, ultimo != NULL ? ultimo->pm10 : 0.0f,
                ultimo != NULL ? ultimo->co : 0.0f, ultimo != NULL ? ultimo->no2 : 0.0f,
                ultimo != NULL ? ultimo->o3 : 0.0f);
    }
    fprintf(fp, "+----------+----------------------+-------------+------------+--------+--------+--------+--------+--------+--------+--------+\n\n");

    fprintf(fp, "+--------------------------------------------------------------------------------------------------+\n");
    fprintf(fp, "|                                   ALERTAS PREVENTIVAS REGISTRADAS                               |\n");
    fprintf(fp, "+------------+----------+----------------------+--------------+--------+--------+----------------------+\n");
    fprintf(fp, "| Fecha      | Hora     | Zona                 | Contaminante | Valor  | Limite | Categoria            |\n");
    fprintf(fp, "+------------+----------+----------------------+--------------+--------+--------+----------------------+\n");
    if (alertas_count == 0) {
        fprintf(fp, "|                                     Sin alertas activas registradas                              |\n");
    } else {
        for (int i = 0; i < alertas_count; i++) {
            fprintf(fp, "| %-10s | %-8s | %-20s | %-12s | %-6.1f | %-6.1f | %-20s |\n",
                    alertas[i].fecha, alertas[i].hora, alertas[i].nombreZona,
                    alertas[i].abreviatura, alertas[i].valor, alertas[i].limite, alertas[i].categoria);
        }
    }
    fprintf(fp, "+------------+----------+----------------------+--------------+--------+--------+----------------------+\n\n");

    fprintf(fp, "+--------------------------------------------------------------------------------------------------------------------+\n");
    fprintf(fp, "|                                        PREDICCIONES DE CALIDAD DEL AIRE (24 HORAS)                                 |\n");
    fprintf(fp, "+----------+----------------------+--------+--------+--------+--------+--------+--------+--------+-----------------------+\n");
    fprintf(fp, "| Codigo   | Nombre Zona          | CO2    | SO2    | PM2.5  | PM10   | CO     | NO2    | O3     | Nivel Estimado        |\n");
    fprintf(fp, "+----------+----------------------+--------+--------+--------+--------+--------+--------+--------+-----------------------+\n");
    for (int i = 0; i < predicciones_count; i++) {
        const Prediction *pred = &predicciones[i];
        fprintf(fp, "| %-8s | %-20s | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-6.1f | %-21s |\n",
                pred->codigoZona, pred->nombreZona, pred->co2, pred->so2, pred->pm25, pred->pm10, pred->co, pred->no2, pred->o3, pred->nivel);
    }
    fprintf(fp, "+----------+----------------------+--------+--------+--------+--------+--------+--------+--------+-----------------------+\n");

    fclose(fp);
    printf("\n✔ Reporte exportado correctamente a '%s'\n", archivo);
}

void leerTexto(char *dest, size_t tam, const char *mensaje) {
    char entrada[100];
    while (1) {
        printf("%s", mensaje);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("Error de lectura.\n");
            exit(1);
        }

        entrada[strcspn(entrada, "\r\n")] = '\0';
        trim(entrada);

        if (!validarVacio(entrada) || !validarSoloLetras(entrada) || strlen(entrada) >= tam) {
            printf("   Entrada invalida. Solo se aceptan letras.\n");
            continue;
        }

        strncpy(dest, entrada, tam - 1);
        dest[tam - 1] = '\0';
        break;
    }
}

void leerFecha(char *dest, size_t tam, const char *mensaje) {
    char entrada[20];
    int d, m, a;
    while (1) {
        printf("%s", mensaje);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("Error de lectura.\n");
            exit(1);
        }

        entrada[strcspn(entrada, "\r\n")] = '\0';
        trim(entrada);
        if (sscanf(entrada, "%2d/%2d/%4d", &d, &m, &a) == 3 && strlen(entrada) == 10) {
            if (d >= 1 && d <= 31 && m >= 1 && m <= 12 && a >= 2000 && a <= 2100) {
                strncpy(dest, entrada, tam - 1);
                dest[tam - 1] = '\0';
                break;
            }
        }
        printf("   Fecha invalida. Use dd/mm/aaaa.\n");
    }
}

void leerNumero(float *dest, float min, float max, const char *mensaje) {
    char entrada[100];
    char *fin = NULL;
    float valor;

    while (1) {
        printf("%s", mensaje);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("Error de lectura.\n");
            exit(1);
        }

        entrada[strcspn(entrada, "\r\n")] = '\0';
        trim(entrada);

        if (!validarVacio(entrada) || !validarNumeroDecimal(entrada)) {
            printf("   Debe ingresar un numero valido.\n");
            continue;
        }

        valor = strtof(entrada, &fin);
        if (fin == entrada || *fin != '\0') {
            printf("   Valor numerico invalido.\n");
            continue;
        }

        if (!validarRangoReal(valor, min, max)) {
            printf("   El valor debe estar entre %.2f y %.2f.\n", min, max);
            continue;
        }

        *dest = valor;
        break;
    }
}

void leerEntero(int *dest, int min, int max, const char *mensaje) {
    char entrada[100];
    char *fin = NULL;
    long valor;

    while (1) {
        printf("%s", mensaje);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("Error de lectura.\n");
            exit(1);
        }

        entrada[strcspn(entrada, "\r\n")] = '\0';
        trim(entrada);

        if (!validarVacio(entrada) || !validarSoloNumeros(entrada)) {
            printf("  ⚠ Debe ingresar solo numeros enteros.\n");
            continue;
        }

        valor = strtol(entrada, &fin, 10);
        if (fin == entrada || *fin != '\0') {
            printf("  ⚠ Valor numerico invalido.\n");
            continue;
        }

        if (!validarRangoEntero((int)valor, min, max)) {
            printf("  ⚠ El valor debe estar entre %d y %d.\n", min, max);
            continue;
        }

        *dest = (int)valor;
        break;
    }
}

void agregarZona(Zone zonas[], int *cantidad) {
    char tipo[20];
    char codigo_temp[20];
    if (*cantidad >= MAX_ZONAS) {
        printf("No se pueden agregar mas zonas. Limite alcanzado.\n");
        return;
    }

    Zone *nueva = &zonas[*cantidad];
    memset(nueva, 0, sizeof(Zone));

    mostrarEncabezado("AGREGAR NUEVA ZONA");
    while (1) {
        printf("  Codigo (ej. Z006): ");
        if (fgets(codigo_temp, sizeof(codigo_temp), stdin) == NULL) {
            printf("Error de lectura.\n");
            exit(1);
        }
        codigo_temp[strcspn(codigo_temp, "\r\n")] = '\0';
        trim(codigo_temp);
        if (validarVacio(codigo_temp)) {
            int valido = 1;
            for (int i = 0; codigo_temp[i] != '\0'; i++) {
                if (!isalnum((unsigned char)codigo_temp[i])) {
                    valido = 0;
                    break;
                }
            }
            if (valido) {
                strncpy(nueva->codigo, codigo_temp, MAX_CODIGO - 1);
                nueva->codigo[MAX_CODIGO - 1] = '\0';
                break;
            }
        }
        printf("  ⚠ Codigo invalido. Use solo letras y numeros.\n");
    }

    leerTexto(nueva->nombre, sizeof(nueva->nombre), "  Nombre: ");
    while (1) {
        printf("  Tipo (Residencial / Industrial / Comercial): ");
        if (fgets(tipo, sizeof(tipo), stdin) == NULL) {
            printf("Error de lectura.\n");
            exit(1);
        }
        tipo[strcspn(tipo, "\r\n")] = '\0';
        trim(tipo);
        normalizarTexto(tipo);
        if (strcmp(tipo, "residencial") == 0 || strcmp(tipo, "industrial") == 0 || strcmp(tipo, "comercial") == 0) {
            strncpy(nueva->tipo, tipo, MAX_TIPO - 1);
            nueva->tipo[MAX_TIPO - 1] = '\0';
            break;
        }
        printf("  ⚠ Tipo invalido.\n");
    }
    leerNumero(&nueva->habitantes, 1.0f, 10000000.0f, "  Habitantes aproximados: ");

    (*cantidad)++;
    printf("\n✔ Zona agregada correctamente.\n");
}

void ingresarRegistroDiario(Zone *zona) {
    DailyRecord *registro;
    if (zona->registros_count >= MAX_REGISTROS) {
        printf("La zona ya tiene el limite de registros.\n");
        return;
    }

    registro = &zona->registros[zona->registros_count];
    memset(registro, 0, sizeof(DailyRecord));

    printf("\n--- Registrar Datos para: %s ---\n", zona->nombre);
    leerFecha(registro->fecha, sizeof(registro->fecha), "  Fecha (dd/mm/aaaa): ");
    
    leerNumero(&registro->co2, 0.0f, 50000.0f, "  Dioxido de Carbono       (CO2)   (mg/m3): ");
    leerNumero(&registro->so2, 0.0f, 300.0f,  "  Dioxido de Azufre        (SO2)   (ug/m3): ");
    leerNumero(&registro->pm25, 0.0f, 500.0f, "  Material Particulado Fino(PM2.5) (ug/m3): ");
    leerNumero(&registro->pm10, 0.0f, 600.0f, "  Material Particulado Grue(PM10)  (ug/m3): ");
    leerNumero(&registro->co, 0.0f, 20.0f,    "  Monoxido de Carbono      (CO)    (mg/m3): ");
    leerNumero(&registro->no2, 0.0f, 300.0f,  "  Dioxido de Nitrogeno     (NO2)   (ug/m3): ");
    leerNumero(&registro->o3, 0.0f, 400.0f,   "  Ozono Troposferico       (O3)    (ug/m3): ");
    leerNumero(&registro->temperatura, -50.0f, 60.0f, "  Temperatura Ambiente             (°C):    ");
    leerNumero(&registro->viento, 0.0f, 200.0f, "  Velocidad del viento             (km/h):  ");
    leerNumero(&registro->humedad, 0.0f, 100.0f, "  Humedad Relativa                 (%%):    ");

    zona->registros_count++;
    printf("\n✔ Registro agregado exitosamente.\n");
}

float calcularPromedioPonderado(const Zone *zona, int tipo_contaminante) {
    if (zona->registros_count <= 0) {
        return 0.0f;
    }

    double suma = 0.0;
    double pesos = 0.0;
    const DailyRecord *ptr = zona->registros;

    for (int i = 0; i < zona->registros_count; i++, ptr++) {
        double valor = 0.0;
        switch (tipo_contaminante) {
            case 0: valor = ptr->pm25; break;
            case 1: valor = ptr->pm10; break;
            case 2: valor = ptr->co; break;
            case 3: valor = ptr->no2; break;
            case 4: valor = ptr->o3; break;
            case 5: valor = ptr->co2; break;
            case 6: valor = ptr->so2; break;
            default: valor = 0.0; break;
        }
        double peso = (double)(i + 1);
        suma += valor * peso;
        pesos += peso;
    }

    return (float)(suma / pesos);
}

float calcularIndiceCompuesto(const Zone *zona) {
    float valores[7];
    float suma = 0.0f;
    const char *abrev[7] = {"PM2.5", "PM10", "CO", "NO2", "O3", "CO2", "SO2"};

    if (zona->registros_count > 0) {
        const DailyRecord *registro = &zona->registros[zona->registros_count - 1];
        valores[0] = registro->pm25;
        valores[1] = registro->pm10;
        valores[2] = registro->co;
        valores[3] = registro->no2;
        valores[4] = registro->o3;
        valores[5] = registro->co2;
        valores[6] = registro->so2;
    } else {
        for (int i = 0; i < 7; i++) {
            valores[i] = 0.0f;
        }
    }

    for (int i = 0; i < 7; i++) {
        float limite = obtenerLimiteContaminante(abrev[i]);
        suma += limite > 0.0f ? valores[i] / limite : 0.0f;
    }

    return suma / 7.0f;
}

const char *clasificarCalidad(float indice) {
    if (indice < 0.5f) return g_referencia.clasificaciones[0];
    if (indice < 1.0f) return g_referencia.clasificaciones[1];
    if (indice < 1.5f) return g_referencia.clasificaciones[2];
    if (indice < 2.0f) return g_referencia.clasificaciones[3];
    return g_referencia.clasificaciones[4];
}

void mostrarMonitoreoActual(const Zone zonas[], int cantidad) {
    mostrarEncabezado("MONITOREO ACTUAL POR ZONA");
    printf("%-22s %-10s %-10s %-8s %-8s %-8s %-8s %-8s %-14s\n",
           "Zona", "CO2", "SO2", "PM2.5", "PM10", "CO", "NO2", "O3", "Calidad");
    mostrarSeparador('-', 120);
    for (int i = 0; i < cantidad; i++) {
        const Zone *zona = &zonas[i];
        const DailyRecord *registro = (zona->registros_count > 0) ? &zona->registros[zona->registros_count - 1] : NULL;
        float indice = 0.0f;
        float co2 = 0.0f;
        float so2 = 0.0f;
        float pm25 = 0.0f;
        float pm10 = 0.0f;
        float co = 0.0f;
        float no2 = 0.0f;
        float o3 = 0.0f;

        if (registro != NULL) {
            co2 = registro->co2;
            so2 = registro->so2;
            pm25 = registro->pm25;
            pm10 = registro->pm10;
            co = registro->co;
            no2 = registro->no2;
            o3 = registro->o3;
            indice = calcularIndiceCompuesto(zona);
        }

        printf("%-22s %-10.2f %-10.2f %-8.2f %-8.2f %-8.2f %-8.2f %-8.2f %-14s\n",
               zona->nombre, co2, so2, pm25, pm10, co, no2, o3, clasificarCalidad(indice));
    }
}

void mostrarPromediosHistoricos(const Zone zonas[], int cantidad) {
    mostrarEncabezado("PROMEDIOS HISTORICOS PONDERADOS");
    printf("%-22s %-10s %-10s %-8s %-8s %-8s %-8s %-8s\n", "Zona", "CO2", "SO2", "PM2.5", "PM10", "CO", "NO2", "O3");
    mostrarSeparador('-', 112);
    for (int i = 0; i < cantidad; i++) {
        const Zone *zona = &zonas[i];
        printf("%-22s %-10.2f %-10.2f %-8.2f %-8.2f %-8.2f %-8.2f %-8.2f\n",
               zona->nombre,
               calcularPromedioPonderado(zona, 5),
               calcularPromedioPonderado(zona, 6),
               calcularPromedioPonderado(zona, 0),
               calcularPromedioPonderado(zona, 1),
               calcularPromedioPonderado(zona, 2),
               calcularPromedioPonderado(zona, 3),
               calcularPromedioPonderado(zona, 4));
    }
}

void generarPrediccion(const Zone *zona, Prediction *prediccion) {
    strncpy(prediccion->codigoZona, zona->codigo, MAX_CODIGO - 1);
    prediccion->codigoZona[MAX_CODIGO - 1] = '\0';
    strncpy(prediccion->nombreZona, zona->nombre, MAX_NOMBRE);
    prediccion->nombreZona[MAX_NOMBRE] = '\0';

    if (zona->registros_count > 0) {
        const DailyRecord *ultimo = &zona->registros[zona->registros_count - 1];
        prediccion->co2 = ultimo->co2 + (ultimo->temperatura > g_referencia.umbralTemperaturaAlta ? 1.5f : 0.0f);
        prediccion->so2 = ultimo->so2 + (ultimo->viento < g_referencia.umbralVientoCalma ? 2.0f : 0.0f);
        prediccion->pm25 = ultimo->pm25 * 1.05f + (ultimo->temperatura > g_referencia.umbralTemperaturaAlta ? 2.0f : 0.0f) - (ultimo->viento > g_referencia.umbralVientoDebil ? 1.5f : 0.0f);
        prediccion->pm10 = ultimo->pm10 * 1.04f + (ultimo->humedad > g_referencia.umbralHumedadNormal ? 3.0f : 0.0f);
        prediccion->co = ultimo->co * 1.03f + (ultimo->temperatura > g_referencia.umbralTemperaturaAlta ? 0.3f : 0.0f);
        prediccion->no2 = ultimo->no2 * 1.06f + (ultimo->viento < g_referencia.umbralVientoCalma ? 2.0f : 0.0f);
        prediccion->o3 = ultimo->o3 * 1.04f + (ultimo->temperatura > g_referencia.umbralTemperaturaMuyAlta ? 4.0f : 0.0f);
        prediccion->temperatura = ultimo->temperatura + 0.5f;
        prediccion->viento = ultimo->viento;
        prediccion->humedad = ultimo->humedad;
    } else {
        prediccion->co2 = obtenerLimiteContaminante("CO2") * 0.8f;
        prediccion->so2 = obtenerLimiteContaminante("SO2") * 0.6f;
        prediccion->pm25 = obtenerLimiteContaminante("PM2.5") * 0.7f;
        prediccion->pm10 = obtenerLimiteContaminante("PM10") * 0.4f;
        prediccion->co = obtenerLimiteContaminante("CO") * 0.25f;
        prediccion->no2 = obtenerLimiteContaminante("NO2") * 0.5f;
        prediccion->o3 = obtenerLimiteContaminante("O3") * 0.5f;
        prediccion->temperatura = 22.0f;
        prediccion->viento = 10.0f;
        prediccion->humedad = 60.0f;
    }

    float max_ratio = 0.0f;
    const char *abrev[7] = {"PM2.5", "PM10", "CO", "NO2", "O3", "CO2", "SO2"};
    float valores[7] = {prediccion->pm25, prediccion->pm10, prediccion->co, prediccion->no2, prediccion->o3, prediccion->co2, prediccion->so2};
    for (int i = 0; i < 7; i++) {
        float limite = obtenerLimiteContaminante(abrev[i]);
        float ratio = limite > 0.0f ? valores[i] / limite : 0.0f;
        if (ratio > max_ratio) {
            max_ratio = ratio;
        }
    }

    strncpy(prediccion->nivel, clasificarCalidad(max_ratio), sizeof(prediccion->nivel) - 1);
    prediccion->nivel[sizeof(prediccion->nivel) - 1] = '\0';

    if (strcmp(zona->tipo, "Industrial") == 0) {
        snprintf(prediccion->mensaje, sizeof(prediccion->mensaje), "Se recomienda reducir emisiones industriales y activar filtros.");
    } else if (strcmp(zona->tipo, "Residencial") == 0) {
        snprintf(prediccion->mensaje, sizeof(prediccion->mensaje), "Se recomienda evitar actividades al aire libre y mantener ventanas cerradas.");
    } else {
        snprintf(prediccion->mensaje, sizeof(prediccion->mensaje), "Se recomienda restringir la circulacion de vehiculos e informar al publico.");
    }
}

void generarAlertas(const Zone zonas[], int cantidad, Alerta alertas[], int *alertas_count, Prediction predicciones[], int *predicciones_count) {
    *alertas_count = 0;
    *predicciones_count = 0;
    for (int i = 0; i < cantidad; i++) {
        Prediction pred;
        generarPrediccion(&zonas[i], &pred);
        predicciones[*predicciones_count] = pred;
        (*predicciones_count)++;

        const char *abrev[7] = {"PM2.5", "PM10", "CO", "NO2", "O3", "CO2", "SO2"};
        float valores[7] = {pred.pm25, pred.pm10, pred.co, pred.no2, pred.o3, pred.co2, pred.so2};
        int indiceMax = -1;
        float maxRatio = 0.0f;

        for (int j = 0; j < 7; j++) {
            float limite = obtenerLimiteContaminante(abrev[j]);
            float ratio = limite > 0.0f ? valores[j] / limite : 0.0f;
            if (ratio > maxRatio) {
                maxRatio = ratio;
                indiceMax = j;
            }
        }

        if (indiceMax >= 0 && maxRatio > 1.0f) {
            Alerta *alerta = &alertas[*alertas_count];
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char fecha[11];
            char hora[9];
            strftime(fecha, sizeof(fecha), "%d/%m/%Y", tm_info);
            strftime(hora, sizeof(hora), "%H:%M:%S", tm_info);

            strncpy(alerta->codigoZone, zonas[i].codigo, MAX_CODIGO - 1);
            alerta->codigoZone[MAX_CODIGO - 1] = '\0';
            strncpy(alerta->nombreZona, zonas[i].nombre, MAX_NOMBRE);
            alerta->nombreZona[MAX_NOMBRE] = '\0';
            strncpy(alerta->fecha, fecha, MAX_FECHA - 1);
            alerta->fecha[MAX_FECHA - 1] = '\0';
            strncpy(alerta->hora, hora, MAX_HORA - 1);
            alerta->hora[MAX_HORA - 1] = '\0';
            strncpy(alerta->contaminante, obtenerNombreContaminante(abrev[indiceMax]), sizeof(alerta->contaminante) - 1);
            alerta->contaminante[sizeof(alerta->contaminante) - 1] = '\0';
            strncpy(alerta->abreviatura, abrev[indiceMax], sizeof(alerta->abreviatura) - 1);
            alerta->abreviatura[sizeof(alerta->abreviatura) - 1] = '\0';
            alerta->valor = valores[indiceMax];
            alerta->limite = obtenerLimiteContaminante(abrev[indiceMax]);
            strncpy(alerta->categoria, pred.nivel, sizeof(alerta->categoria) - 1);
            alerta->categoria[sizeof(alerta->categoria) - 1] = '\0';
            strncpy(alerta->recomendacion, obtenerRecomendacion(zonas[i].tipo, abrev[indiceMax]), MAX_MENSAJE - 1);
            alerta->recomendacion[MAX_MENSAJE - 1] = '\0';
            (*alertas_count)++;
        }
    }
}

void mostrarPredicciones(const Zone zonas[], int cantidad, const Prediction predicciones[], int predicciones_count) {
    (void)zonas;
    (void)cantidad;
    mostrarEncabezado("PREDICCION A 24 HORAS");
    printf("%-12s %-22s %-8s %-8s %-8s %-8s %-8s %-8s %-14s\n", "Codigo", "Zona", "CO2", "SO2", "PM2.5", "PM10", "CO", "NO2", "Nivel");
    mostrarSeparador('-', 128);
    for (int i = 0; i < predicciones_count; i++) {
        const Prediction *pred = &predicciones[i];
        printf("%-12s %-22s %-8.2f %-8.2f %-8.2f %-8.2f %-8.2f %-8.2f %-14s\n",
               pred->codigoZona, pred->nombreZona, pred->co2, pred->so2, pred->pm25, pred->pm10, pred->co, pred->no2, pred->nivel);
    }
}

void mostrarAlertas(const Alerta alertas[], int alertas_count) {
    mostrarEncabezado("ALERTAS PREVENTIVAS ACTIVAS");
    printf("Total de alertas registradas: %d\n", alertas_count);
    mostrarSeparador('-', 120);
    for (int i = 0; i < alertas_count; i++) {
        printf("[%s %s] %-22s %-10s valor: %-8.2f limite: %-8.2f estado: %s\n",
               alertas[i].fecha, alertas[i].hora, alertas[i].nombreZona, alertas[i].abreviatura, alertas[i].valor, alertas[i].limite, alertas[i].categoria);
    }
}

void consultarZona(const Zone zonas[], int cantidad, const Prediction predicciones[], int predicciones_count) {
    int opcion;
    mostrarEncabezado("CONSULTAR DETALLES DE ZONA");
    for (int i = 0; i < cantidad; i++) {
        printf("  %d. %s\n", i + 1, zonas[i].nombre);
    }
    leerEntero(&opcion, 1, cantidad, "Seleccione una zona: ");

    const Zone *zona = &zonas[opcion - 1];
    const Prediction *pred = NULL;
    for (int i = 0; i < predicciones_count; i++) {
        if (strcmp(predicciones[i].codigoZona, zona->codigo) == 0) {
            pred = &predicciones[i];
            break;
        }
    }

    printf("\n+------------------------------------------------------------+\n");
    printf("|              GUIA DE COMPONENTES EVALUADOS                 |\n");
    printf("+------------+-----------------------------------------------+\n");
    printf("| CO2        | Dioxido de Carbono (Gas combustion industrial)|\n");
    printf("| SO2        | Dioxido de Azufre (Emisiones de combustibles) |\n");
    printf("| PM2.5      | Material Particulado Fino (Polvo, hollin fino)|\n");
    printf("| PM10       | Material Particulado Grueso (Polvo, cenizas)  |\n");
    printf("| CO         | Monoxido de Carbono (Gases vehiculares)       |\n");
    printf("| NO2        | Dioxido de Nitrogeno (Gases de escape)        |\n");
    printf("| O3         | Ozono Troposferico (Reacciones fotoquimicas)  |\n");
    printf("+------------+-----------------------------------------------+\n");

    printf("\n+------------------------------------------------------------+\n");
    printf("|                FICHA DE MONITOREO DE LA ZONA               |\n");
    printf("+----------------------+-------------------------------------+\n");
    printf("| Campo                | Valor Actual / Parametro            |\n");
    printf("+----------------------+-------------------------------------+\n");
    printf("| Nombre de la Zona    | %-35s |\n", zona->nombre);
    printf("| Codigo Identificador | %-35s |\n", zona->codigo);
    printf("| Tipo de Entorno      | %-35s |\n", zona->tipo);
    printf("| Habitantes Estimados | %-35.0f |\n", zona->habitantes);
    
    if (zona->registros_count > 0) {
        const DailyRecord *ultimo = &zona->registros[zona->registros_count - 1];
        printf("+----------------------+-------------------------------------+\n");
        printf("| LECTURAS REGISTRADAS | Fecha de toma: %-20s |\n", ultimo->fecha);
        printf("+----------------------+-------------------------------------+\n");
        printf("| CO2  (Dioxido Carb.) | %-31.2f mg/m3 |\n", ultimo->co2);
        printf("| SO2  (Dioxido Azuf.) | %-31.2f ug/m3 |\n", ultimo->so2);
        printf("| PM2.5(Partic. Finas) | %-31.2f ug/m3 |\n", ultimo->pm25);
        printf("| PM10 (Partic. Grues) | %-31.2f ug/m3 |\n", ultimo->pm10);
        printf("| CO   (Monoxido Carb.)| %-31.2f mg/m3 |\n", ultimo->co);
        printf("| NO2  (Dioxido Nitr.) | %-31.2f ug/m3 |\n", ultimo->no2);
        printf("| O3   (Ozono Tropos.) | %-31.2f ug/m3 |\n", ultimo->o3);
        printf("| Temperatura Ambiente | %-31.2f *C    |\n", ultimo->temperatura);
        printf("| Velocidad del Viento | %-31.2f km/h  |\n", ultimo->viento);
        printf("| Humedad Relativa     | %-31.2f %%     |\n", ultimo->humedad);
    } else {
        printf("| Datos Ambientales    | Sin lecturas registradas aun        |\n");
    }

    if (pred != NULL) {
        printf("+----------------------+-------------------------------------+\n");
        printf("| PROYECCION A 24 HORAS| Estimaciones Basadas en Tendencias  |\n");
        printf("+----------------------+-------------------------------------+\n");
        printf("| CO2 Proyectado       | %-31.2f mg/m3 |\n", pred->co2);
        printf("| SO2 Proyectado       | %-31.2f ug/m3 |\n", pred->so2);
        printf("| PM2.5 Proyectado     | %-31.2f ug/m3 |\n", pred->pm25);
        printf("| PM10 Proyectado      | %-31.2f ug/m3 |\n", pred->pm10);
        printf("| Calidad Estimada     | %-35s |\n", pred->nivel);
        printf("+----------------------+-------------------------------------+\n");
        printf("| RECOMENDACION        | %-35.35s |\n", pred->mensaje);
        if (strlen(pred->mensaje) > 35) {
            printf("|                      | %-35.35s |\n", pred->mensaje + 35);
        }
    }
    printf("+----------------------+-------------------------------------+\n");
}

void mostrarTodasZonasOrdenadas(const Zone zonas[], int cantidad) {
    int indices[MAX_ZONAS];
    for (int i = 0; i < cantidad; i++) {
        indices[i] = i;
    }

    for (int i = 0; i < cantidad - 1; i++) {
        int max_pos = i;
        for (int j = i + 1; j < cantidad; j++) {
            if (calcularIndiceCompuesto(&zonas[indices[j]]) > calcularIndiceCompuesto(&zonas[indices[max_pos]])) {
                max_pos = j;
            }
        }
        if (max_pos != i) {
            int temp = indices[i];
            indices[i] = indices[max_pos];
            indices[max_pos] = temp;
        }
    }

    mostrarEncabezado("ZONAS ORDENADAS POR CONTAMINACION");
    printf("%-3s %-22s %-12s %-14s\n", "#", "Zona", "Indice", "Calidad");
    mostrarSeparador('-', 55);
    for (int i = 0; i < cantidad; i++) {
        const Zone *zona = &zonas[indices[i]];
        printf("%-3d %-22s %-12.2f %-14s\n", i + 1, zona->nombre, calcularIndiceCompuesto(zona), clasificarCalidad(calcularIndiceCompuesto(zona)));
    }
}

void mostrarEstadisticas(const Zone zonas[], int cantidad, const Alerta alertas[], int alertas_count) {
    float max_indice = -1.0f;
    float min_indice = 999999.0f;
    float suma = 0.0f;
    int max_pos = 0;
    int min_pos = 0;

    for (int i = 0; i < cantidad; i++) {
        float indice = calcularIndiceCompuesto(&zonas[i]);
        suma += indice;
        if (indice > max_indice) {
            max_indice = indice;
            max_pos = i;
        }
        if (indice < min_indice) {
            min_indice = indice;
            min_pos = i;
        }
    }

    mostrarEncabezado("ESTADISTICAS RESUMIDAS");
    printf("  Zonas activas en sistema : %d\n", cantidad);
    printf("  Zona Critica (Maxima)    : %s\n", zonas[max_pos].nombre);
    printf("  Zona Stable (Minima)    : %s\n", zonas[min_pos].nombre);
    printf("  Promedios de contaminacion: %.2f\n", cantidad > 0 ? (suma / cantidad) : 0.0f);
    printf("  Alertas Totales Emitidas : %d\n", alertas_count);
}

void modificarZona(Zone zonas[], int cantidad) {
    if (cantidad == 0) {
        printf("\n⚠ No hay zonas registradas.\n");
        return;
    }

    mostrarEncabezado("MODIFICAR PARAMETRO ESPECIFICO");
    for (int i = 0; i < cantidad; i++) {
        printf("  %d. %s (%s)\n", i + 1, zonas[i].nombre, zonas[i].codigo);
    }
    
    int indice_zona = 0;
    leerEntero(&indice_zona, 1, cantidad, "Seleccione el numero de la zona: ");
    indice_zona--;
    
    Zone *zona_a_modificar = &zonas[indice_zona];
    DailyRecord *registro = (zona_a_modificar->registros_count > 0) ? &zona_a_modificar->registros[zona_a_modificar->registros_count - 1] : NULL;
    
    if (registro == NULL) {
        printf("\n⚠ Error: La zona seleccionada no contiene registros actuales para modificar.\n");
        return;
    }
    
    printf("\n--- Seleccione el parametro unico a editar ---\n");
    printf("  1. Dioxido de Carbono       (CO2)   (Unidad: mg/m3)\n");
    printf("  2. Dioxido de Azufre        (SO2)   (Unidad: ug/m3)\n");
    printf("  3. Material Particulado Fino(PM2.5) (Unidad: ug/m3)\n");
    printf("  4. Material Particulado Grue(PM10)  (Unidad: ug/m3)\n");
    printf("  5. Monoxido de Carbono      (CO)    (Unidad: mg/m3)\n");
    printf("  6. Dioxido de Nitrogeno     (NO2)   (Unidad: ug/m3)\n");
    printf("  7. Ozono Troposferico       (O3)    (Unidad: ug/m3)\n");
    printf("  8. Temperatura Ambiente             (C)\n");
    printf("  9. Velocidad del viento             (km/h)\n");
    printf("  10. Humedad relativa                (%)\n");
    
    int indice_parametro = 0;
    leerEntero(&indice_parametro, 1, 10, "Seleccione el numero de la variable: ");
    
    float nuevo_valor = 0.0f;
    printf("\n");
    switch (indice_parametro) {
        case 1:
            printf("  [Actual Dioxido de Carbono (CO2): %.2f mg/m3]\n", registro->co2);
            leerNumero(&nuevo_valor, 0.0f, 50000.0f, "  Ingrese nuevo valor: ");
            registro->co2 = nuevo_valor;
            break;
        case 2:
            printf("  [Actual Dioxido de Azufre (SO2): %.2f ug/m3]\n", registro->so2);
            leerNumero(&nuevo_valor, 0.0f, 300.0f, "  Ingrese nuevo valor: ");
            registro->so2 = nuevo_valor;
            break;
        case 3:
            printf("  [Actual Material Particulado Fino (PM2.5): %.2f ug/m3]\n", registro->pm25);
            leerNumero(&nuevo_valor, 0.0f, 500.0f, "  Ingrese nuevo valor: ");
            registro->pm25 = nuevo_valor;
            break;
        case 4:
            printf("  [Actual Material Particulado Grueso (PM10): %.2f ug/m3]\n", registro->pm10);
            leerNumero(&nuevo_valor, 0.0f, 600.0f, "  Ingrese nuevo valor: ");
            registro->pm10 = nuevo_valor;
            break;
        case 5:
            printf("  [Actual Monoxido de Carbono (CO): %.2f mg/m3]\n", registro->co);
            leerNumero(&nuevo_valor, 0.0f, 20.0f, "  Ingrese nuevo valor: ");
            registro->co = nuevo_valor;
            break;
        case 6:
            printf("  [Actual Dioxido de Nitrogeno (NO2): %.2f ug/m3]\n", registro->no2);
            leerNumero(&nuevo_valor, 0.0f, 300.0f, "  Ingrese nuevo valor: ");
            registro->no2 = nuevo_valor;
            break;
        case 7:
            printf("  [Actual Ozono Troposferico (O3): %.2f ug/m3]\n", registro->o3);
            leerNumero(&nuevo_valor, 0.0f, 400.0f, "  Ingrese nuevo valor: ");
            registro->o3 = nuevo_valor;
            break;
        case 8:
            printf("  [Actual Temperatura Ambiente: %.2f C]\n", registro->temperatura);
            leerNumero(&nuevo_valor, -50.0f, 60.0f, "  Ingrese nuevo valor: ");
            registro->temperatura = nuevo_valor;
            break;
        case 9:
            printf("  [Actual Velocidad del Viento: %.2f km/h]\n", registro->viento);
            leerNumero(&nuevo_valor, 0.0f, 200.0f, "  Ingrese nuevo valor: ");
            registro->viento = nuevo_valor;
            break;
        case 10:
            printf("  [Actual Humedad Relativa: %.2f %]\n", registro->humedad);
            leerNumero(&nuevo_valor, 0.0f, 100.0f, "  Ingrese nuevo valor: ");
            registro->humedad = nuevo_valor;
            break;
    }
    printf("\n✔ Parametro actualizado con exito para la zona %s.\n", zona_a_modificar->nombre);
}

void eliminarZona(Zone zonas[], int *cantidad) {
    if (*cantidad == 0) {
        printf("\n⚠ No hay zonas en el sistema para eliminar.\n");
        return;
    }

    mostrarEncabezado("ELIMINAR REGISTROS DE ZONA");
    for (int i = 0; i < *cantidad; i++) {
        printf("  %d. %s (%s)\n", i + 1, zonas[i].nombre, zonas[i].codigo);
    }

    int seleccion = 0;
    leerEntero(&seleccion, 1, *cantidad, "Seleccione la zona a remover: ");
    seleccion--;

    printf("\n¿Está seguro de eliminar completamente la zona %s? (1: Si / 2: No): ", zonas[seleccion].nombre);
    int confirmacion;
    leerEntero(&confirmacion, 1, 2, "");

    if (confirmacion == 1) {
        for (int i = seleccion; i < (*cantidad) - 1; i++) {
            zonas[i] = zonas[i + 1];
        }
        (*cantidad)--;
        printf("\n✔ La zona ha sido removida del sistema exitosamente.\n");
    } else {
        printf("\nOperacion cancelada.\n");
    }
}

void mostrarMenu(void) {
    printf("\n---------------------------------------------------------\n");
    printf("  MENU PRINCIPAL - SEGUIMIENTO AMBIENTAL\n");
    printf("---------------------------------------------------------\n");
    printf("  1. Agregar nueva zona al sistema\n");
    printf("  2. Registrar entrada de lecturas diarias (Por lote)\n");
    printf("  3. Ver monitoreo del estado actual general\n");
    printf("  4. Consultar promedios historicos (Ponderados)\n");
    printf("  5. Ver predicciones meteorologicas de 24 horas\n");
    printf("  6. Mostrar alertas preventivas activas\n");
    printf("  7. Consultar informacion especifica por zona\n");
    printf("  8. Visualizar ranking de zonas mas contaminadas\n");
    printf("  9. Revisar estadisticas generales resumidas\n");
    printf("  10. Modificar un solo dato especifico de una zona\n");
    printf("  11. Eliminar/Dar de baja una zona\n");
    printf("  12. Exportar reporte general (Archivo de texto)\n");
    printf("  0. Salir del sistema\n");
    printf("---------------------------------------------------------\n");
    printf("  Seleccione una opcion: ");
}

void ejecutarMenu(Zone zonas[], int *cantidad, Alerta alertas[], int *alertas_count, Prediction predicciones[], int *predicciones_count) {
    int opcion;
    while (1) {
        mostrarMenu();
        leerEntero(&opcion, 0, 12, "");

        switch (opcion) {
            case 1:
                agregarZona(zonas, cantidad);
                break;
            case 2: {
                if (*cantidad == 0) {
                    printf("\n⚠ No hay zonas configuradas.\n");
                    break;
                }
                int indice;
                for (int i = 0; i < *cantidad; i++) {
                    printf("  %d. %s\n", i + 1, zonas[i].nombre);
                }
                leerEntero(&indice, 1, *cantidad, "Seleccione la zona para registrar datos: ");
                ingresarRegistroDiario(&zonas[indice - 1]);
                break;
            }
            case 3:
                mostrarMonitoreoActual(zonas, *cantidad);
                break;
            case 4:
                mostrarPromediosHistoricos(zonas, *cantidad);
                break;
            case 5:
                generarAlertas(zonas, *cantidad, alertas, alertas_count, predicciones, predicciones_count);
                mostrarPredicciones(zonas, *cantidad, predicciones, *predicciones_count);
                break;
            case 6:
                generarAlertas(zonas, *cantidad, alertas, alertas_count, predicciones, predicciones_count);
                mostrarAlertas(alertas, *alertas_count);
                break;
            case 7:
                consultarZona(zonas, *cantidad, predicciones, *predicciones_count);
                break;
            case 8:
                mostrarTodasZonasOrdenadas(zonas, *cantidad);
                break;
            case 9:
                mostrarEstadisticas(zonas, *cantidad, alertas, *alertas_count);
                break;
            case 10:
                modificarZona(zonas, *cantidad);
                break;
            case 11:
                eliminarZona(zonas, cantidad);
                break;
            case 12:
                generarAlertas(zonas, *cantidad, alertas, alertas_count, predicciones, predicciones_count);
                exportarReporte(zonas, *cantidad, alertas, *alertas_count, predicciones, *predicciones_count, "reporte_sistema.txt");
                break;
            case 0:
                guardarDatosEnArchivo(zonas, *cantidad, alertas, *alertas_count, predicciones, *predicciones_count, "zonas.dat", "alertas.dat", "predicciones.dat");
                printf("\nDatos guardados. Cerrando el sistema. Hasta luego!\n");
                return;
        }
    }
}