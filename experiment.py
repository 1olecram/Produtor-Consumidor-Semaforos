import os
import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import time

# Create output directories
os.makedirs("graficos_ocupacao", exist_ok=True)
os.makedirs("dados_ocupacao", exist_ok=True)

# Configurations
N_values = [1, 10, 100, 1000]
combinations = [(1, 1), (1, 2), (1, 4), (1, 8), (2, 1), (4, 1), (8, 1)]
repeats = 10

# Compile C++ code
print("Compilando semaforos.cpp...")
compile_res = subprocess.run(["g++", "-O3", "semaforos.cpp", "-o", "semaforos.out", "-pthread"])
if compile_res.returncode != 0:
    print("Erro na compilação!")
    exit(1)

# Results dictionary for execution times
# format: results[N][comb] = average_time
results = {N: {} for N in N_values}

print("Iniciando experimentos...")

for N in N_values:
    for Np, Nc in combinations:
        comb_str = f"{Np}p_{Nc}c"
        print(f"\n--- Cenário: N={N}, {Np} produtor(es), {Nc} consumidor(es) ---")
        
        # 1. Run 10 times to get average execution time (without logging CSV to avoid IO overhead)
        total_time = 0.0
        for i in range(repeats):
            res = subprocess.run(["./semaforos.out", str(N), str(Np), str(Nc), "-q"], capture_output=True, text=True)
            if res.returncode != 0:
                print(f"Erro rodando cenário (Tempo): {res.stderr}")
                continue
            # Output only contains the time (as a string)
            exec_time = float(res.stdout.strip())
            total_time += exec_time
        
        avg_time = total_time / repeats
        results[N][comb_str] = avg_time
        print(f"Tempo médio ({repeats} execuções): {avg_time:.4f} s")
        
        # 2. Run 1 time WITH logging to generate the occupancy graph
        csv_filename = f"dados_ocupacao/ocupacao_N{N}_{comb_str}.csv"
        print(f"Gerando log de ocupação em {csv_filename}...")
        res = subprocess.run(["./semaforos.out", str(N), str(Np), str(Nc), csv_filename, "-q"], capture_output=True, text=True)
        if res.returncode != 0:
            print(f"Erro gerando CSV: {res.stderr}")
            continue
            
        # Read the CSV and generate graph
        if os.path.exists(csv_filename):
            df = pd.read_csv(csv_filename)
            
            plt.figure(figsize=(10, 5))
            plt.plot(df['Operacao'], df['Ocupacao'], alpha=0.5, color='blue', linewidth=0.5)
            plt.title(f"Ocupação do Buffer (N={N}, Np={Np}, Nc={Nc})")
            plt.xlabel("Operação (Produção ou Consumo)")
            plt.ylabel("Itens no Buffer")
            plt.ylim(0, N if N > 0 else 10)
            
            out_img = f"graficos_ocupacao/grafico_ocupacao_N{N}_{comb_str}.png"
            plt.savefig(out_img, dpi=300)
            plt.close()
            print(f"Gráfico de ocupação salvo em {out_img}")

# 3. Generate the Execution Time Graph
print("\nGerando gráfico de tempos de execução...")

plt.figure(figsize=(12, 6))

x_labels = [f"{p}p/{c}c" for p, c in combinations]
x_positions = range(len(combinations))

for N in N_values:
    y_values = [results[N][f"{p}p_{c}c"] for p, c in combinations]
    plt.plot(x_positions, y_values, marker='o', label=f"N={N}")

plt.xticks(x_positions, x_labels)
plt.title("Tempo Médio de Execução vs Combinação de Threads")
plt.xlabel("Combinação (Produtores / Consumidores)")
plt.ylabel("Tempo Médio (s)")
plt.legend()
plt.grid(True, linestyle='--', alpha=0.7)

exec_plot_filename = "tempo_execucao_vs_threads.png"
plt.savefig(exec_plot_filename, dpi=300)
plt.close()

print(f"Gráfico de tempo de execução salvo em {exec_plot_filename}")
print("\nTodos os experimentos concluídos com sucesso!")
