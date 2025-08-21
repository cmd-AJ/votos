# Makefile para compilar programas de conteo de votos
CC = gcc
CFLAGS = -Wall -O2
OPENMP_FLAGS = -fopenmp

# Nombres de los ejecutables
PROG_SEC = conteo_secuencial
PROG_PAR = conteo_paralelo

# Archivos fuente
SRC_SEC = conteo_secuencial.c
SRC_PAR = conteo_paralelo.c

# Regla por defecto: compilar ambos programas
all: $(PROG_SEC) $(PROG_PAR)

# Compilar programa secuencial
$(PROG_SEC): $(SRC_SEC)
	$(CC) $(CFLAGS) -o $(PROG_SEC) $(SRC_SEC) -lm
	@echo "Programa secuencial compilado exitosamente"

# Compilar programa paralelo con OpenMP
$(PROG_PAR): $(SRC_PAR)
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o $(PROG_PAR) $(SRC_PAR) -lm
	@echo "Programa paralelo compilado exitosamente"

# Ejecutar programa secuencial
run-sec: $(PROG_SEC)
	@echo "Ejecutando programa secuencial..."
	./$(PROG_SEC)

# Ejecutar programa paralelo
run-par: $(PROG_PAR)
	@echo "Ejecutando programa paralelo..."
	./$(PROG_PAR)

# Ejecutar ambos programas
run-all: all
	@echo "========== EJECUCIÓN SECUENCIAL =========="
	./$(PROG_SEC)
	@echo "\n========== EJECUCIÓN PARALELA =========="
	./$(PROG_PAR)

# Prueba rápida con valores predefinidos
test: all
	@echo "Ejecutando prueba con 10000 boletas, 4 candidatos..."
	@echo "10000\n4" | ./$(PROG_SEC)
	@echo "10000\n4\n4" | ./$(PROG_PAR)

# Prueba automática con 1 millón de boletas
test-big: all
	@echo "========== PRUEBA CON 1 MILLÓN DE BOLETAS =========="
	./$(PROG_SEC) -test
	./$(PROG_PAR) -test
	@echo "===================================================="

# Limpiar archivos compilados y resultados
clean:
	rm -f $(PROG_SEC) $(PROG_PAR) *.o
	rm -f resultados_secuencial.txt resultados_paralelo.txt
	@echo "Archivos limpiados"

# Limpiar solo archivos de resultados
clean-results:
	rm -f resultados_secuencial.txt resultados_paralelo.txt
	@echo "✓ Archivos de resultados eliminados"

# Ayuda
help:
	@echo "Comandos disponibles:"
	@echo "  make         - Compila ambos programas"
	@echo "  make run-sec - Ejecuta el programa secuencial"
	@echo "  make run-par - Ejecuta el programa paralelo"
	@echo "  make run-all - Ejecuta ambos programas"
	@echo "  make test    - Ejecuta prueba rápida con valores predefinidos"
	@echo "  make test-big - Ejecuta prueba con 1 millón de boletas (recomendado)"
	@echo "  make clean   - Elimina ejecutables y archivos de resultados"
	@echo "  make help    - Muestra esta ayuda"

.PHONY: all run-sec run-par run-all test test-big clean clean-results help