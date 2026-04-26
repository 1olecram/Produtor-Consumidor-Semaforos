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

---

## Análise Crítica e Conclusões

Como requisitado pelo escopo do Trabalho Prático 1, seguem as análises sobre o comportamento observado a partir dos gráficos e execuções:

### 1. Efeito do Tamanho do Buffer (N)
Quando `N = 1`, o buffer atua essencialmente como uma catraca simples, obrigando uma estrita sincronização "um-pra-um" (um produz, avisa, outro consome, avisa). Essa dependência severa leva a um **grande número de trocas de contexto (context switches)** e esperas em bloqueio (nos semáforos `sem_empty` e `sem_full`). 
Ao aumentar `N` para 10, 100 ou 1000, o grau de concorrência se eleva. Produtores podem enfileirar diversos itens antes de precisarem bloquear por falta de espaço, resultando em curvas de tempo médio de execução consideravelmente menores e mais otimizadas (menos bloqueios e melhor localidade de execução).

### 2. Proporção de Produtores e Consumidores (Np / Nc)
Observa-se que o tempo médio reage de forma sensível às combinações de `(Np, Nc)`.
- A verificação de números primos em valores altos (até $10^7$) consome processamento pesado por parte dos **Consumidores**, enquanto os **Produtores** realizam uma operação computacionalmente mais leve (geração de números aleatórios).
- Por essa razão matemática (função `isPrime` sendo o gargalo em O(sqrt(n))), as configurações com escassez de consumidores e excesso de produtores (ex: `8p/1c`) costumam apresentar uma ocupação de buffer mantida consistentemente cheia (ponto máximo de N), pois os consumidores não dão vazão à fila tão rapidamente quanto ela é preenchida. 
- Inversamente, proporções com mais consumidores ajudam a reduzir drasticamente o tempo total do processo, uma vez que paralelizam o gargalo principal (o cálculo matemático) permitindo que o processamento esvazie a fila rapidamente.

### 3. Exclusão Mútua (Contenção de Lock)
Apesar de mais threads (8p/1c ou 4p/1c) distribuírem tarefas, o acesso individual à Seção Crítica (inserir no `buffer` de memória compartilhada) é protegido por um único `sem_mutex`. À medida que aumentamos excessivamente o número total de threads sem aumentar também o espaço do buffer, parte do tempo ganho é perdido pela pura contenção do semáforo `sem_mutex` (concorrência agressiva). Logo, escalar o número de threads exige escalar a disponibilidade do buffer para observar um verdadeiro ganho de throughput.
