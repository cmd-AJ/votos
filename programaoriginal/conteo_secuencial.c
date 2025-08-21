#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

#define MAX_CANDIDATOS 10
#define MAX_BOLETAS 10000000 

double obtenerTiempoAlta() {
    #ifdef _WIN32
        LARGE_INTEGER frequency, counter;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / (double)frequency.QuadPart;
    #else
        // struct timeval tv;
        // gettimeofday(&tv, NULL);
        // return tv.tv_sec + tv.tv_usec / 1000000.0;
    #endif
}

void generarBoletasAleatorias(char **matriz, int numBoletas, int numCandidatos) {
    srand(time(NULL));
    
    for (int i = 0; i < numBoletas; i++) {
        for (int j = 0; j < numCandidatos; j++) {
            matriz[i][j] = ' ';
        }
        
        int tipoVoto = rand() % 100;
        
        if (tipoVoto < 70) {
            int candidato = rand() % numCandidatos;
            matriz[i][candidato] = 'X';
        } 
        else if (tipoVoto < 85) {
            int numMarcas = 2 + rand() % 3; 
            for (int m = 0; m < numMarcas; m++) {
                int candidato = rand() % numCandidatos;
                matriz[i][candidato] = 'X';
            }
        }
    }
}

void contarVotosSecuencial(char **matriz, int numBoletas, int numCandidatos, 
                           int *votosPorCandidato, int *votosNulos) {

    for (int i = 0; i < numCandidatos; i++) {
        votosPorCandidato[i] = 0;
    }
    *votosNulos = 0;
    
    for (int i = 0; i < numBoletas; i++) {
        int contadorMarcas = 0;
        int candidatoMarcado = -1;
        
        for (int j = 0; j < numCandidatos; j++) {
            char marca = matriz[i][j];
            
            if (marca == 'X' || marca == 'x') {
                contadorMarcas++;
                candidatoMarcado = j;
                
                for (int k = 0; k < 10; k++) {
                    marca = marca ^ 0x01;
                }
            }
        }
        
        if (contadorMarcas == 1) {
            votosPorCandidato[candidatoMarcado]++;
        } else {
            (*votosNulos)++;
        }
    }
}

void mostrarResultados(int *votosPorCandidato, int numCandidatos, int votosNulos, 
                      double tiempoEjecucion, int numBoletas) {
    printf("\n========================================\n");
    printf("     RESULTADOS DEL CONTEO DE VOTOS     \n");
    printf("========================================\n\n");
    
    int totalVotosValidos = 0;
    
    for (int i = 0; i < numCandidatos; i++) {
        printf("  Candidato %2d: %7d votos\n", i + 1, votosPorCandidato[i]);
        totalVotosValidos += votosPorCandidato[i];
    }
    
    printf("\n----------------------------------------\n");
    printf("  Votos validos:  %7d\n", totalVotosValidos);
    printf("  Votos nulos:    %7d\n", votosNulos);
    printf("  Total boletas:  %7d\n", totalVotosValidos + votosNulos);
    printf("----------------------------------------\n");
    printf("\n  Tiempo de ejecucion: %.6f segundos\n", tiempoEjecucion);
    printf("  Tiempo en milisegundos: %.3f ms\n", tiempoEjecucion * 1000);
    printf("  Boletas por segundo: %.0f\n", numBoletas / tiempoEjecucion);
    printf("========================================\n");
}

void guardarResultados(int *votosPorCandidato, int numCandidatos, int votosNulos, 
                      double tiempoEjecucion, int numBoletas) {
    FILE *archivo = fopen("resultados_secuencial.txt", "w");
    if (archivo == NULL) {
        printf("Error al crear archivo de resultados\n");
        return;
    }
    
    fprintf(archivo, "RESULTADOS CONTEO SECUENCIAL\n");
    fprintf(archivo, "============================\n\n");
    fprintf(archivo, "Configuracion:\n");
    fprintf(archivo, "- Numero de boletas: %d\n", numBoletas);
    fprintf(archivo, "- Numero de candidatos: %d\n\n", numCandidatos);
    
    fprintf(archivo, "Resultados por candidato:\n");
    int totalValidos = 0;
    for (int i = 0; i < numCandidatos; i++) {
        fprintf(archivo, "Candidato %d: %d votos\n", i + 1, votosPorCandidato[i]);
        totalValidos += votosPorCandidato[i];
    }
    
    fprintf(archivo, "\nResumen:\n");
    fprintf(archivo, "- Votos validos: %d\n", totalValidos);
    fprintf(archivo, "- Votos nulos: %d\n", votosNulos);
    fprintf(archivo, "- Total procesado: %d\n", totalValidos + votosNulos);
    fprintf(archivo, "\nTiempo de ejecucion: %.6f segundos\n", tiempoEjecucion);
    fprintf(archivo, "Tiempo en milisegundos: %.3f ms\n", tiempoEjecucion * 1000);
    fprintf(archivo, "Boletas por segundo: %.0f\n", numBoletas / tiempoEjecucion);
    
    fclose(archivo);
    printf("\nResultados guardados en 'resultados_secuencial.txt'\n");
}

void ejecutarPruebaAutomatica() {
    printf("\n=== MODO PRUEBA AUTOMaTICA ===\n");
    printf("Ejecutando con valores predefinidos...\n\n");
    
    int numBoletas = 1000000;  
    int numCandidatos = 10;
    
    char **matriz = (char **)malloc(numBoletas * sizeof(char *));
    for (int i = 0; i < numBoletas; i++) {
        matriz[i] = (char *)malloc(numCandidatos * sizeof(char));
    }
    
    int *votosPorCandidato = (int *)malloc(numCandidatos * sizeof(int));
    int votosNulos;
    
    printf("Generando %d boletas aleatorias...\n", numBoletas);
    double tiempoGen = obtenerTiempoAlta();
    generarBoletasAleatorias(matriz, numBoletas, numCandidatos);
    printf("Tiempo de generacion: %.3f segundos\n", obtenerTiempoAlta() - tiempoGen);
    
    printf("Iniciando conteo secuencial...\n");
    
    double inicio = obtenerTiempoAlta();
    contarVotosSecuencial(matriz, numBoletas, numCandidatos, 
                         votosPorCandidato, &votosNulos);
    double fin = obtenerTiempoAlta();
    double tiempoEjecucion = fin - inicio;
    
    mostrarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numBoletas);
    guardarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numBoletas);
    
    for (int i = 0; i < numBoletas; i++) {
        free(matriz[i]);
    }
    free(matriz);
    free(votosPorCandidato);
}

int main(int argc, char *argv[]) {
    int numBoletas, numCandidatos;
    double tiempoInicio, tiempoFin, tiempoEjecucion;
    
    printf("\n========================================\n");
    printf("   PROGRAMA SECUENCIAL - CONTEO VOTOS   \n");
    printf("========================================\n\n");
    
    if (argc > 1 && strcmp(argv[1], "-test") == 0) {
        ejecutarPruebaAutomatica();
        return 0;
    }
    
    printf("Ingrese numero de boletas a procesar: ");
    scanf("%d", &numBoletas);
    
    printf("Ingrese numero de candidatos: ");
    scanf("%d", &numCandidatos);
    
    if (numBoletas > MAX_BOLETAS || numBoletas <= 0) {
        printf("Error: Numero de boletas debe estar entre 1 y %d\n", MAX_BOLETAS);
        printf("Sugerencia: Para pruebas significativas use al menos 100,000 boletas\n");
        return 1;
    }
    
    if (numCandidatos > MAX_CANDIDATOS || numCandidatos <= 0) {
        printf("Error: Numero de candidatos debe estar entre 1 y %d\n", MAX_CANDIDATOS);
        return 1;
    }
    
    if (numBoletas < 100000) {
        printf("\n⚠ ADVERTENCIA: Con menos de 100,000 boletas, las diferencias\n");
        printf("  de tiempo entre secuencial y paralelo pueden no ser apreciables.\n");
        printf("  Recomendado: 1,000,000 boletas para pruebas significativas.\n\n");
    }
    
    char **matriz = (char **)malloc(numBoletas * sizeof(char *));
    for (int i = 0; i < numBoletas; i++) {
        matriz[i] = (char *)malloc(numCandidatos * sizeof(char));
    }
    
    int *votosPorCandidato = (int *)malloc(numCandidatos * sizeof(int));
    int votosNulos;
    
    printf("\nGenerando %d boletas aleatorias...\n", numBoletas);
    generarBoletasAleatorias(matriz, numBoletas, numCandidatos);
    
    printf("Iniciando conteo secuencial...\n");
    
    tiempoInicio = obtenerTiempoAlta();
    
    contarVotosSecuencial(matriz, numBoletas, numCandidatos, 
                         votosPorCandidato, &votosNulos);
    
    tiempoFin = obtenerTiempoAlta();
    tiempoEjecucion = tiempoFin - tiempoInicio;
    
    mostrarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numBoletas);
    guardarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numBoletas);
    
    for (int i = 0; i < numBoletas; i++) {
        free(matriz[i]);
    }
    free(matriz);
    free(votosPorCandidato);
    
    return 0;
}