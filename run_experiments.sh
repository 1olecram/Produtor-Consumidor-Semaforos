#!/bin/bash
set -e

# Compilar o código C++
g++ semaforos.cpp -o semaforos -pthread -O3

N=10 # Tamanho do buffer
combinations=("1 1" "1 2" "1 4" "1 8" "2 1" "4 1" "8 1")

echo "Iniciando execuções com N=$N..."

for comb in "${combinations[@]}"; do
    read -r Np Nc <<< "$comb"
    echo "====================================="
    echo "Executando: $Np produtores, $Nc consumidores"
    time ./semaforos $N $Np $Nc
    echo ""
done

echo "Gerando gráficos..."
python3 plot_occupancy.py
echo "Concluído!"