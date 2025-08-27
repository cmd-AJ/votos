#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

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
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
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

void contarVotosParalelo(char **matriz, int numBoletas, int numCandidatos, 
                         int *votosPorCandidato, int *votosNulos, int numHilos) {
    for (int i = 0; i < numCandidatos; i++) {
        votosPorCandidato[i] = 0;
    }
    *votosNulos = 0;
    
    omp_set_num_threads(numHilos);
    
    int totalNulos = 0;
    
    #pragma omp parallel reduction(+:totalNulos)
    {
        int idHilo = omp_get_thread_num();
        int totalHilos = omp_get_num_threads();
        int votosLocales[MAX_CANDIDATOS] = {0};
        
        #pragma omp single
        {
            printf("  Procesando con %d hilos...\n", totalHilos);
        }
        
        #pragma omp for schedule(static)
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
                votosLocales[candidatoMarcado]++;
            } else {
                totalNulos++;
            }
        }
        
        #pragma omp critical
        {
            for (int i = 0; i < numCandidatos; i++) {
                votosPorCandidato[i] += votosLocales[i];
            }
        }
    }
    
    *votosNulos = totalNulos;
}

void mostrarResultados(int *votosPorCandidato, int numCandidatos, int votosNulos, 
                      double tiempoEjecucion, int numHilos, int numBoletas) {
    printf("\n========================================\n");
    printf("     RESULTADOS DEL CONTEO PARALELO     \n");
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
    printf("  Numero de hilos utilizados: %d\n", numHilos);
    printf("========================================\n");
}

void guardarResultados(int *votosPorCandidato, int numCandidatos, int votosNulos, 
                      double tiempoEjecucion, int numBoletas, int numHilos) {
    FILE *archivo = fopen("resultados_paralelo.txt", "w");
    if (archivo == NULL) {
        printf("Error al crear archivo de resultados\n");
        return;
    }
    
    fprintf(archivo, "RESULTADOS CONTEO PARALELO (OpenMP)\n");
    fprintf(archivo, "====================================\n\n");
    fprintf(archivo, "Configuracion:\n");
    fprintf(archivo, "- Numero de boletas: %d\n", numBoletas);
    fprintf(archivo, "- Numero de candidatos: %d\n", numCandidatos);
    fprintf(archivo, "- Numero de hilos: %d\n\n", numHilos);
    
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
    printf("\nResultados guardados en 'resultados_paralelo.txt'\n");
}

void compararTiempos(double tiempoParalelo, int numHilos) {
    FILE *archivoSec = fopen("resultados_secuencial.txt", "r");
    if (archivoSec != NULL) {
        char linea[256];
        double tiempoSecuencial = 0;
        
        while (fgets(linea, sizeof(linea), archivoSec)) {
            if (strstr(linea, "Tiempo de ejecucion:") != NULL) {
                sscanf(linea, "Tiempo de ejecucion: %lf", &tiempoSecuencial);
                break;
            }
        }
        fclose(archivoSec);
        
        if (tiempoSecuencial > 0) {
            double speedup = tiempoSecuencial / tiempoParalelo;
            double eficiencia = speedup / numHilos * 100;
            
            printf("\n========================================\n");
            printf("         COMPARACIoN DE TIEMPOS         \n");
            printf("========================================\n");
            printf("  Tiempo secuencial:  %.6f seg (%.3f ms)\n", 
                   tiempoSecuencial, tiempoSecuencial * 1000);
            printf("  Tiempo paralelo:    %.6f seg (%.3f ms)\n", 
                   tiempoParalelo, tiempoParalelo * 1000);
            printf("  Speedup obtenido:   %.2fx\n", speedup);
            printf("  Eficiencia:         %.1f%%\n", eficiencia);
            printf("  Mejora de tiempo:   %.3f ms\n", 
                   (tiempoSecuencial - tiempoParalelo) * 1000);
            
            if (speedup < 1.0) {
                printf("\n   El programa paralelo es mas lento.\n");
                printf("  Posibles causas:\n");
                printf("  - Pocas boletas (overhead > beneficio)\n");
                printf("  - Demasiados hilos para el trabajo\n");
                printf("  - Competencia por recursos del sistema\n");
                printf("\n  Sugerencias:\n");
                printf("  - Use al menos 1,000,000 boletas\n");
                printf("  - Use 4-8 hilos maximo\n");
            } else if (speedup > 1.0 && speedup < 1.5) {
                printf("\n  Mejora modesta. Para mejor rendimiento:\n");
                printf("  - Aumente el numero de boletas\n");
                printf("  - Ajuste el numero de hilos\n");
            } else {
                printf("\n  Buen speedup obtenido!\n");
            }
            
            printf("========================================\n");
        }
    }
}

void ejecutarPruebaAutomatica() {
    printf("\n=== MODO PRUEBA AUTOMaTICA ===\n");
    printf("Ejecutando con valores optimizados...\n\n");
    
    int numBoletas = 1000000;  
    int numCandidatos = 10;
    int numHilos = omp_get_num_procs() > 8 ? 8 : omp_get_num_procs();
    
    printf("Configuracion automatica:\n");
    printf("- Boletas: %d\n", numBoletas);
    printf("- Candidatos: %d\n", numCandidatos);
    printf("- Hilos: %d\n\n", numHilos);
    
    char **matriz = (char **)malloc(numBoletas * sizeof(char *));
    for (int i = 0; i < numBoletas; i++) {
        matriz[i] = (char *)malloc(numCandidatos * sizeof(char));
    }
    
    int *votosPorCandidato = (int *)malloc(numCandidatos * sizeof(int));
    int votosNulos;
    
    printf("Generando %d boletas aleatorias...\n", numBoletas);
    double tiempoGen = obtenerTiempoAlta();
    generarBoletasAleatorias(matriz, numBoletas, numCandidatos);
    printf("Tiempo de generacion: %.3f segundos\n\n", obtenerTiempoAlta() - tiempoGen);
    
    printf("Iniciando conteo paralelo...\n");
    
    double inicio = obtenerTiempoAlta();
    contarVotosParalelo(matriz, numBoletas, numCandidatos, 
                       votosPorCandidato, &votosNulos, numHilos);
    double fin = obtenerTiempoAlta();
    double tiempoEjecucion = fin - inicio;
    
    mostrarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numHilos, numBoletas);
    guardarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numBoletas, numHilos);
    
    compararTiempos(tiempoEjecucion, numHilos);
    
    for (int i = 0; i < numBoletas; i++) {
        free(matriz[i]);
    }
    free(matriz);
    free(votosPorCandidato);
}

int obtenerNumeroHilosOptimo(int numBoletas) {
    int numCores = omp_get_num_procs();
    int hilosOptimos;
    
    if (numBoletas < 10000) {
        hilosOptimos = 2; 
    } else if (numBoletas < 100000) {
        hilosOptimos = (numCores > 4) ? 4 : numCores;
    } else {
        hilosOptimos = (numCores > 8) ? 8 : numCores;
    }
    
    return hilosOptimos;
}

int main(int argc, char *argv[]) {
    int numBoletas, numCandidatos, numHilos;
    double tiempoInicio, tiempoFin, tiempoEjecucion;
    
    printf("\n========================================\n");
    printf("  PROGRAMA PARALELO (OpenMP) - CONTEO   \n");
    printf("========================================\n\n");
    
    if (argc > 1 && strcmp(argv[1], "-test") == 0) {
        ejecutarPruebaAutomatica();
        return 0;
    }
    
    int maxHilos = omp_get_max_threads();
    int numCores = omp_get_num_procs();
    printf("Sistema: %d cores detectados, maximo %d hilos\n\n", numCores, maxHilos);
    
    printf("Ingrese numero de boletas a procesar: ");
    scanf("%d", &numBoletas);
    
    printf("Ingrese numero de candidatos: ");
    scanf("%d", &numCandidatos);
    
    int hilosRecomendados = obtenerNumeroHilosOptimo(numBoletas);
    printf("Ingrese numero de hilos (recomendado: %d): ", hilosRecomendados);
    scanf("%d", &numHilos);
    
    if (numBoletas > MAX_BOLETAS || numBoletas <= 0) {
        printf("Error: Numero de boletas debe estar entre 1 y %d\n", MAX_BOLETAS);
        printf("Sugerencia: Para ver beneficios del paralelismo use al menos 100,000 boletas\n");
        return 1;
    }
    
    if (numCandidatos > MAX_CANDIDATOS || numCandidatos <= 0) {
        printf("Error: Numero de candidatos debe estar entre 1 y %d\n", MAX_CANDIDATOS);
        return 1;
    }
    
    if (numHilos <= 0 || numHilos > maxHilos) {
        numHilos = hilosRecomendados;
        printf("Ajustando a %d hilos (optimo para %d boletas)\n", numHilos, numBoletas);
    }
    
    if (numBoletas < 100000) {
        printf("\n⚠ ADVERTENCIA: Con menos de 100,000 boletas,\n");
        printf("  el paralelismo puede no mostrar mejoras significativas.\n");
        printf("  Para mejores resultados, use:\n");
        printf("  - Al menos 1,000,000 boletas\n");
        printf("  - Entre 4 y 8 hilos\n\n");
    }
    
    if (numHilos > numCores) {
        printf("\nUsando mas hilos (%d) que cores fisicos (%d).\n", numHilos, numCores);
        printf("  Esto puede reducir el rendimiento.\n\n");
    }
    
    char **matriz = (char **)malloc(numBoletas * sizeof(char *));
    for (int i = 0; i < numBoletas; i++) {
        matriz[i] = (char *)malloc(numCandidatos * sizeof(char));
    }
    
    int *votosPorCandidato = (int *)malloc(numCandidatos * sizeof(int));
    int votosNulos;
    
    printf("\nGenerando %d boletas aleatorias...\n", numBoletas);
    generarBoletasAleatorias(matriz, numBoletas, numCandidatos);
    
    printf("\nIniciando conteo paralelo con %d hilos...\n\n", numHilos);
    
    tiempoInicio = obtenerTiempoAlta();
    
    contarVotosParalelo(matriz, numBoletas, numCandidatos, 
                       votosPorCandidato, &votosNulos, numHilos);
    
    tiempoFin = obtenerTiempoAlta();
    tiempoEjecucion = tiempoFin - tiempoInicio;
    
    mostrarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numHilos, numBoletas);
    guardarResultados(votosPorCandidato, numCandidatos, votosNulos, 
                     tiempoEjecucion, numBoletas, numHilos);
    
    compararTiempos(tiempoEjecucion, numHilos);
    
    for (int i = 0; i < numBoletas; i++) {
        free(matriz[i]);
    }
    free(matriz);
    free(votosPorCandidato);
    
    return 0;
}