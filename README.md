# TP1: Produtor-Consumidor com Semáforos

Este projeto implementa o clássico problema Produtor-Consumidor em ambiente multithreaded na linguagem C++ utilizando exclusão mútua e coordenação de fluxo com semáforos POSIX (`semaphore.h`).

## Compilação e Execução

### Execução Automática (Recomendada para Geração de Gráficos)
Para rodar toda a bateria de testes definida no trabalho (combinações de N=1, 10, 100, 1000 com diferentes proporções de threads produtoras e consumidoras), calcular as médias de tempo e extrair os gráficos automáticos, utilize o script Python fornecido:

```bash
# Necessita Python 3 com bibliotecas pandas e matplotlib
python3 experiment.py
```
Esse script já compila o programa com a flag de otimização `-O3` e invoca o binário repassando a _flag silenciosa_ `-q` para medir com precisão o tempo real (sem gargalos de I/O de impressão no terminal).

### Execução Manual
Se desejar rodar o programa manualmente para observar as saídas individuais das threads (verificação de números primos) exigidas pelo documento, utilize:

```bash
# Compilar manualmente
g++ -O3 semaforos.cpp -o semaforos.out -pthread

# Executar (Ex: Buffer 10, 2 Produtores, 4 Consumidores)
./semaforos.out 10 2 4
```
A execução manual padrão imprimirá no terminal cada verificação realizada pelos consumidores (se o número gerado é primo ou não).
- Para salvar a ocupação do buffer num arquivo, adicione o nome do CSV como quarto parâmetro: `./semaforos.out 10 2 4 ocupacao.csv`
- Para ignorar as impressões na tela, adicione o argumento `-q`.
