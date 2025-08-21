import time
import math
import os
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from typing import List, Tuple
import random
import argparse

MAX_CANDIDATOS = 10
MAX_BOLETAS = 10_000_000

@dataclass
class Resultados:
    votos_por_candidato: List[int]
    votos_nulos: int
    tiempo_ejecucion: float
    num_boletas: int
    num_candidatos: int
    num_hilos: int

def obtener_num_hilos_optimo(num_boletas: int) -> int:
    # heurística similar a la del C
    cores = os.cpu_count() or 4
    if num_boletas < 10_000:
        return min(2, cores)
    elif num_boletas < 100_000:
        return min(4, cores)
    else:
        return min(8, cores)

def cpu_noise(ch: int) -> int:
    # Simula el bucle XOR de 10 iteraciones del C para "costo" extra
    # (no afecta el conteo)
    for _ in range(10):
        ch ^= 0x01
    return ch

def generar_y_contar_rango(start: int, end: int, num_candidatos: int, seed: int) -> Tuple[List[int], int]:
    """
    Genera boletas en el rango [start, end) y cuenta:
      - votos por candidato (válidos si hay una sola marca)
      - votos nulos (0 marcas o 2-4 marcas)
    Misma distribución que el C:
      70%: 1 marca
      15%: 2-4 marcas
      15%: 0 marcas (implícito, nada se marca)
    """
    rng = random.Random(seed)
    votos_locales = [0] * num_candidatos
    nulos_locales = 0

    for _ in range(start, end):
        tipo = rng.randrange(100)
        if tipo < 70:
            # Un candidato marcado (válido)
            j = rng.randrange(num_candidatos)
            _ = cpu_noise(ord('X'))  # trabajo inútil como en el C
            votos_locales[j] += 1
        elif tipo < 85:
            # 2 a 4 marcas -> nulo
            num_marcas = 2 + rng.randrange(3)  # 2..4
            # Escogemos sin preocuparnos por duplicados porque igual es nulo
            for _m in range(num_marcas):
                _ = rng.randrange(num_candidatos)
                _ = cpu_noise(ord('X'))
            nulos_locales += 1
        else:
            # 0 marcas -> nulo
            nulos_locales += 1

    return votos_locales, nulos_locales

def contar_votos_paralelo(num_boletas: int, num_candidatos: int, num_hilos: int) -> Resultados:
    # Particiona el espacio de boletas entre hilos
    chunk = math.ceil(num_boletas / num_hilos)
    rangos = []
    for t in range(num_hilos):
        start = t * chunk
        end = min((t + 1) * chunk, num_boletas)
        if start < end:
            rangos.append((start, end))

    print(f"\nIniciando conteo paralelo con {len(rangos)} hilos...\n")
    t0 = time.perf_counter()

    # Usamos semillas distintas por hilo para RNG independiente
    base_seed = int(time.time())
    votos_global = [0] * num_candidatos
    nulos_global = 0

    with ThreadPoolExecutor(max_workers=len(rangos)) as ex:
        futs = []
        for idx, (start, end) in enumerate(rangos):
            seed = base_seed + 17 * idx
            futs.append(ex.submit(generar_y_contar_rango, start, end, num_candidatos, seed))

        for fu in as_completed(futs):
            votos_loc, nulos_loc = fu.result()
            # Reducción en el hilo principal
            for i in range(num_candidatos):
                votos_global[i] += votos_loc[i]
            nulos_global += nulos_loc

    t1 = time.perf_counter()
    return Resultados(
        votos_por_candidato=votos_global,
        votos_nulos=nulos_global,
        tiempo_ejecucion=t1 - t0,
        num_boletas=num_boletas,
        num_candidatos=num_candidatos,
        num_hilos=num_hilos
    )

def mostrar_resultados(res: Resultados) -> None:
    print("\n========================================")
    print("     RESULTADOS DEL CONTEO (Python)     ")
    print("========================================\n")
    total_validos = 0
    for i, v in enumerate(res.votos_por_candidato, start=1):
        print(f"  Candidato {i:2d}: {v:7d} votos")
        total_validos += v
    print("\n----------------------------------------")
    print(f"  Votos validos:  {total_validos:7d}")
    print(f"  Votos nulos:    {res.votos_nulos:7d}")
    print(f"  Total boletas:  {total_validos + res.votos_nulos:7d}")
    print("----------------------------------------")
    print(f"\n  Tiempo de ejecucion: {res.tiempo_ejecucion:.6f} s")
    print(f"  Tiempo en miliseg.:  {res.tiempo_ejecucion*1000:.3f} ms")
    if res.tiempo_ejecucion > 0:
        print(f"  Boletas por segundo: {res.num_boletas/res.tiempo_ejecucion:.0f}")
    print(f"  Numero de hilos:     {res.num_hilos}")
    print("========================================")

def guardar_resultados(res: Resultados, nombre="resultados_paralelo_py.txt") -> None:
    total_validos = sum(res.votos_por_candidato)
    with open(nombre, "w", encoding="utf-8") as f:
        f.write("RESULTADOS CONTEO PARALELO (Python Threads)\n")
        f.write("===========================================\n\n")
        f.write("Configuracion:\n")
        f.write(f"- Numero de boletas: {res.num_boletas}\n")
        f.write(f"- Numero de candidatos: {res.num_candidatos}\n")
        f.write(f"- Numero de hilos: {res.num_hilos}\n\n")
        f.write("Resultados por candidato:\n")
        for i, v in enumerate(res.votos_por_candidato, start=1):
            f.write(f"Candidato {i}: {v} votos\n")
        f.write("\nResumen:\n")
        f.write(f"- Votos validos: {total_validos}\n")
        f.write(f"- Votos nulos: {res.votos_nulos}\n")
        f.write(f"- Total procesado: {total_validos + res.votos_nulos}\n")
        f.write(f"\nTiempo de ejecucion: {res.tiempo_ejecucion:.6f} s\n")
        f.write(f"Tiempo en milisegundos: {res.tiempo_ejecucion*1000:.3f} ms\n")
        if res.tiempo_ejecucion > 0:
            f.write(f"Boletas por segundo: {res.num_boletas/res.tiempo_ejecucion:.0f}\n")
    print(f"\nResultados guardados en '{nombre}'")

def main():
    parser = argparse.ArgumentParser(description="Conteo de votos paralelo (Python threads)")
    parser.add_argument("--boletas", type=int, help="Número de boletas a procesar")
    parser.add_argument("--candidatos", type=int, help="Número de candidatos (<=10)")
    parser.add_argument("--hilos", type=int, help="Número de hilos")
    parser.add_argument("--auto", action="store_true", help="Modo automático similar a -test")
    args = parser.parse_args()

    if args.auto:
        num_boletas = 1_000_000
        num_candidatos = 10
        num_hilos = obtener_num_hilos_optimo(num_boletas)
        print("\n=== MODO AUTOMÁTICO ===")
        print(f"- Boletas: {num_boletas}")
        print(f"- Candidatos: {num_candidatos}")
        print(f"- Hilos: {num_hilos}\n")
    else:
        if args.boletas is not None:
            num_boletas = args.boletas
        else:
            num_boletas = int(input("Ingrese numero de boletas a procesar: "))
        if args.candidatos is not None:
            num_candidatos = args.candidatos
        else:
            num_candidatos = int(input("Ingrese numero de candidatos: "))
        if args.hilos is not None:
            num_hilos = args.hilos
        else:
            recomendado = obtener_num_hilos_optimo(num_boletas)
            num_hilos = int(input(f"Ingrese numero de hilos (recomendado: {recomendado}): "))

    # Validaciones (mismas ideas que en C)
    if num_boletas <= 0 or num_boletas > MAX_BOLETAS:
        raise SystemExit(f"Error: Numero de boletas debe estar entre 1 y {MAX_BOLETAS}")
    if num_candidatos <= 0 or num_candidatos > MAX_CANDIDATOS:
        raise SystemExit(f"Error: Numero de candidatos debe estar entre 1 y {MAX_CANDIDATOS}")
    if num_hilos <= 0:
        num_hilos = obtener_num_hilos_optimo(num_boletas)

    # Advertencias similares
    if num_boletas < 100_000:
        print("\n⚠ ADVERTENCIA: Con menos de 100,000 boletas, el paralelismo puede no mostrar mejoras claras (GIL y overhead).")

    res = contar_votos_paralelo(num_boletas, num_candidatos, num_hilos)
    mostrar_resultados(res)
    guardar_resultados(res)

if __name__ == "__main__":
    main()
