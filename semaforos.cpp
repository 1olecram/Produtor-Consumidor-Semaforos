#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <thread>
#include <chrono>
#include <semaphore.h>
#include <fstream>
#include <cstdio>

int count = 0; // Itens no buffer
bool quiet_mode = false; // Flag para suprimir prints de stdout
int in = 0;    // Índice de produção
int out = 0;   // Índice de consumo

std::vector<int> buffer;        // Memória compartilhada (buffer)
std::vector<int> occupancy_log; // Log da ocupação do buffer após cada operação

sem_t sem_empty; // Semáforo contador para posições livres
sem_t sem_full;  // Semáforo contador para posições ocupadas
sem_t sem_mutex; // Semáforo para exclusão mútua

const int M = 1e5;


bool isPrime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

int numGeneration() {   
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 1e7);
    return distrib(gen); 
}

//thread produtor
void produtor(int id, int num_itens) {
    for (int i = 0; i < num_itens; ++i) {
        int item = numGeneration();

        sem_wait(&sem_empty);
        sem_wait(&sem_mutex);

        buffer[in] = item;
        in = (in + 1) % buffer.size();
        count++;
        
        occupancy_log.push_back(count);

        sem_post(&sem_mutex);
        sem_post(&sem_full);
    }
}

//thread consumidor
void consumidor(int id, int num_itens) {
    for (int i = 0; i < num_itens; ++i) {
        sem_wait(&sem_full);
        sem_wait(&sem_mutex);

        int item = buffer[out];
        bool is_p = isPrime(item);
        
        out = (out + 1) % buffer.size();
        count--;
        
        occupancy_log.push_back(count);

        sem_post(&sem_mutex);
        sem_post(&sem_empty);
        
        if (!quiet_mode) {
             printf("Consumidor %d verificou o numero %d: %s\n", id, item, is_p ? "eh primo" : "nao eh primo");
        }
    }
}

int main(int argc, char const *argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-q") {
            quiet_mode = true;
        } else {
            args.push_back(argv[i]);
        }
    }

    if (args.size() < 3) {
        std::cerr << "Uso: " << argv[0] << " <tamanho_do_buffer> <Np> <Nc> [arquivo_log.csv] [-q]\n";
        return 1;
    }

    int N, Np, Nc;
    try {
        N = std::stoi(args[0]);
        Np = std::stoi(args[1]);
        Nc = std::stoi(args[2]);
        if (N <= 0 || Np <= 0 || Nc <= 0) {
            std::cerr << "Erro: O tamanho do buffer, Np e Nc devem ser maiores que 0.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro: Argumentos invalidos fornecidos.\n";
        return 1;
    }

    buffer.resize(N);
    occupancy_log.reserve(2 * M);

    sem_init(&sem_empty, 0, N);
    sem_init(&sem_full, 0, 0);
    sem_init(&sem_mutex, 0, 1);

    std::vector<std::thread> produtores;
    std::vector<std::thread> consumidores;
    
    int itens_por_produtor = M / Np;
    int itens_por_consumidor = M / Nc;

    for (int i = 0; i < Np; ++i) {
        produtores.push_back(std::thread(produtor, i + 1, itens_por_produtor));
    }
    for (int i = 0; i < Nc; ++i) {
        consumidores.push_back(std::thread(consumidor, i + 1, itens_por_consumidor));
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    for (auto& p : produtores) p.join();
    for (auto& c : consumidores) c.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    
    // Imprime apenas o tempo para facilitar coleta automática pelo script
    std::cout << duration.count() << "\n";

    // Salvar log se um nome de arquivo foi passado como 4o argumento
    if (args.size() >= 4) {
        std::string filename = args[3];
        std::ofstream out_file(filename);
        if (out_file.is_open()) {
            out_file << "Operacao,Ocupacao\n";
            for (size_t i = 0; i < occupancy_log.size(); ++i) {
                out_file << i << "," << occupancy_log[i] << "\n";
            }
            out_file.close();
        }
    }

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);

    return 0;
}
