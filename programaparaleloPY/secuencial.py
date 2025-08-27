import random
import time
import sys

MAX_CANDIDATOS = 10
MAX_BOLETAS = 10_000_000


def obtener_tiempo_alta():
    return time.perf_counter()


def generar_boletas_aleatorias(num_boletas, num_candidatos):
    random.seed(time.time())
    matriz = [[' ' for _ in range(num_candidatos)] for _ in range(num_boletas)]

    for i in range(num_boletas):
        tipo_voto = random.randint(0, 99)

        if tipo_voto < 70:  # voto válido (1 candidato)
            candidato = random.randint(0, num_candidatos - 1)
            matriz[i][candidato] = 'X'

        elif tipo_voto < 85:  # voto nulo con múltiples marcas (2-4)
            num_marcas = 2 + random.randint(0, 2)
            for _ in range(num_marcas):
                candidato = random.randint(0, num_candidatos - 1)
                matriz[i][candidato] = 'X'

    return matriz


def contar_votos_secuencial(matriz, num_candidatos):
    votos_por_candidato = [0] * num_candidatos
    votos_nulos = 0

    for fila in matriz:
        contador_marcas = 0
        candidato_marcado = -1

        for j, marca in enumerate(fila):
            if marca in ('X', 'x'):
                contador_marcas += 1
                candidato_marcado = j
                # (El XOR del C no cambia nada real, lo omitimos en Python)

        if contador_marcas == 1:
            votos_por_candidato[candidato_marcado] += 1
        else:
            votos_nulos += 1

    return votos_por_candidato, votos_nulos


def mostrar_resultados(votos_por_candidato, votos_nulos, tiempo_ejecucion, num_boletas):
    print("\n========================================")
    print("     RESULTADOS DEL CONTEO DE VOTOS     ")
    print("========================================\n")

    total_validos = sum(votos_por_candidato)

    for i, votos in enumerate(votos_por_candidato, start=1):
        print(f"  Candidato {i:2d}: {votos:7d} votos")

    print("\n----------------------------------------")
    print(f"  Votos validos:  {total_validos:7d}")
    print(f"  Votos nulos:    {votos_nulos:7d}")
    print(f"  Total boletas:  {total_validos + votos_nulos:7d}")
    print("----------------------------------------")
    print(f"\n  Tiempo de ejecucion: {tiempo_ejecucion:.6f} segundos")
    print(f"  Tiempo en milisegundos: {tiempo_ejecucion * 1000:.3f} ms")
    print(f"  Boletas por segundo: {num_boletas / tiempo_ejecucion:.0f}")
    print("========================================")


def guardar_resultados(votos_por_candidato, votos_nulos, tiempo_ejecucion, num_boletas):
    with open("resultados_secuencial.txt", "w") as archivo:
        archivo.write("RESULTADOS CONTEO SECUENCIAL\n")
        archivo.write("============================\n\n")
        archivo.write("Configuracion:\n")
        archivo.write(f"- Numero de boletas: {num_boletas}\n")
        archivo.write(f"- Numero de candidatos: {len(votos_por_candidato)}\n\n")

        archivo.write("Resultados por candidato:\n")
        total_validos = 0
        for i, votos in enumerate(votos_por_candidato, start=1):
            archivo.write(f"Candidato {i}: {votos} votos\n")
            total_validos += votos

        archivo.write("\nResumen:\n")
        archivo.write(f"- Votos validos: {total_validos}\n")
        archivo.write(f"- Votos nulos: {votos_nulos}\n")
        archivo.write(f"- Total procesado: {total_validos + votos_nulos}\n")
        archivo.write(f"\nTiempo de ejecucion: {tiempo_ejecucion:.6f} segundos\n")
        archivo.write(f"Tiempo en milisegundos: {tiempo_ejecucion * 1000:.3f} ms\n")
        archivo.write(f"Boletas por segundo: {num_boletas / tiempo_ejecucion:.0f}\n")

    print("\nResultados guardados en 'resultados_secuencial.txt'")


def ejecutar_prueba_automatica():
    print("\n=== MODO PRUEBA AUTOMATICA ===")
    print("Ejecutando con valores predefinidos...\n")

    num_boletas = 1_000_000
    num_candidatos = 10

    print(f"Generando {num_boletas} boletas aleatorias...")
    t0 = obtener_tiempo_alta()
    matriz = generar_boletas_aleatorias(num_boletas, num_candidatos)
    print(f"Tiempo de generacion: {obtener_tiempo_alta() - t0:.3f} segundos")

    print("Iniciando conteo secuencial...")

    inicio = obtener_tiempo_alta()
    votos_por_candidato, votos_nulos = contar_votos_secuencial(matriz, num_candidatos)
    fin = obtener_tiempo_alta()
    tiempo_ejecucion = fin - inicio

    mostrar_resultados(votos_por_candidato, votos_nulos, tiempo_ejecucion, num_boletas)
    guardar_resultados(votos_por_candidato, votos_nulos, tiempo_ejecucion, num_boletas)


def main():
    print("\n========================================")
    print("   PROGRAMA SECUENCIAL - CONTEO VOTOS   ")
    print("========================================\n")

    if len(sys.argv) > 1 and sys.argv[1] == "-test":
        ejecutar_prueba_automatica()
        return

    try:
        num_boletas = int(input("Ingrese numero de boletas a procesar: "))
        num_candidatos = int(input("Ingrese numero de candidatos: "))
    except ValueError:
        print("Error: Entrada invalida.")
        return

    if num_boletas > MAX_BOLETAS or num_boletas <= 0:
        print(f"Error: Numero de boletas debe estar entre 1 y {MAX_BOLETAS}")
        print("Sugerencia: Para pruebas significativas use al menos 100,000 boletas")
        return

    if num_candidatos > MAX_CANDIDATOS or num_candidatos <= 0:
        print(f"Error: Numero de candidatos debe estar entre 1 y {MAX_CANDIDATOS}")
        return

    if num_boletas < 100000:
        print("\n⚠ ADVERTENCIA: Con menos de 100,000 boletas, las diferencias")
        print("  de tiempo entre secuencial y paralelo pueden no ser apreciables.")
        print("  Recomendado: 1,000,000 boletas para pruebas significativas.\n")

    print(f"\nGenerando {num_boletas} boletas aleatorias...")
    matriz = generar_boletas_aleatorias(num_boletas, num_candidatos)

    print("Iniciando conteo secuencial...")

    inicio = obtener_tiempo_alta()
    votos_por_candidato, votos_nulos = contar_votos_secuencial(matriz, num_candidatos)
    fin = obtener_tiempo_alta()
    tiempo_ejecucion = fin - inicio

    mostrar_resultados(votos_por_candidato, votos_nulos, tiempo_ejecucion, num_boletas)
    guardar_resultados(votos_por_candidato, votos_nulos, tiempo_ejecucion, num_boletas)


if __name__ == "__main__":
    main()
