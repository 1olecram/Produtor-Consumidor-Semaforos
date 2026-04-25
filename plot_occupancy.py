import pandas as pd
import matplotlib.pyplot as plt
import os

combinations = [(1, 1), (1, 2), (1, 4), (1, 8), (2, 1), (4, 1), (8, 1)]

for Np, Nc in combinations:
    filename = f"ocupacao_buffer_{Np}p_{Nc}c.csv"
    if os.path.exists(filename):
        df = pd.read_csv(filename)
        
        plt.figure(figsize=(10, 5))
        # Plotando apenas alguns pontos se o log for muito grande para não ficar um borrão
        # ou podemos plotar com menor alpha. Como são 200 mil pontos, scatter/plot é viável.
        plt.plot(df['Operacao'], df['Ocupacao'], alpha=0.5, color='blue', linewidth=0.5)
        
        plt.title(f"Ocupação do Buffer (Np={Np}, Nc={Nc})")
        plt.xlabel("Operação (Produção ou Consumo)")
        plt.ylabel("Itens no Buffer")
        plt.ylim(0, 10) # Assumindo buffer tamanho 10 nos testes
        
        out_img = f"grafico_ocupacao_{Np}p_{Nc}c.png"
        plt.savefig(out_img, dpi=300)
        plt.close()
        print(f"Gráfico salvo em {out_img}")
    else:
        print(f"Arquivo não encontrado: {filename}")
