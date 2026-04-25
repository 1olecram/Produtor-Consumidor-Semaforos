#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <thread>
#include <chrono>
#include <semaphore.h>

int count = 0; // Itens no buffer
int in = 0;    // Índice de produção
int out = 0;   // Índice de consumo

std::vector<int> buffer; // Memória compartilhada (buffer)

sem_t sem_empty; // Semáforo contador para posições livres
sem_t sem_full;  // Semáforo contador para posições ocupadas
sem_t sem_mutex; // Semáforo para exclusão mútua

bool isPrime(int num)
{
    if (num <= 1)
    {
        return false;
    }
    
    // Conta os divisores considerando o quadradado do contador -> O(sqrt(n))
    for (int i = 2; i * i <= num; i++)
    {
        if (num % i == 0)
            return false;
    }
    return true; // Se passar por todo o laco, é primo
}

int numGeneration()
{   
    // Seed com fonte de entropia via hardware
    // Usar thread_local ou static para não inicializar a cada chamada, 
    // melhorando a performance e gerando entropia correta
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 1e7);

    return distrib(gen); 
}

const int M = 1e5;

void produtor(int id, int num_itens) {
    for (int i = 0; i < num_itens; ++i) {
        int item = numGeneration(); // Produz item

        sem_wait(&sem_empty); // Aguarda posição livre
        sem_wait(&sem_mutex); // Entra na região crítica

        buffer[in] = item;
        // std::cout << "Produtor " << id << " colocou " << item << " na pos " << in << "\n";

        in = (in + 1) % buffer.size(); // Avança o índice circularmente
        count++;

        sem_post(&sem_mutex); // Sai da região crítica
        sem_post(&sem_full);  // Sinaliza nova posição ocupada

        // std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

void consumidor(int id, int num_itens) {
    for (int i = 0; i < num_itens; ++i) {
        sem_wait(&sem_full);  // Aguarda posição ocupada
        sem_wait(&sem_mutex); // Entra na região crítica

        int item = buffer[out];
        // std::cout << "Consumidor " << id << " retirou " << item << " da pos " << out;

        // Validando se é primo com a função já definida
        if (isPrime(item)) {
            // std::cout << " (É primo!)\n";
        } else {
            // std::cout << " (Não é primo)\n";
        }

        out = (out + 1) % buffer.size(); // Libera a posição logicamente
        count--;

        sem_post(&sem_mutex); // Sai da região crítica
        sem_post(&sem_empty); // Sinaliza nova posição livre

        // std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 4) {
        std::cerr << "Uso: " << argv[0] << " <tamanho_do_buffer> <Np> <Nc>\n";
        return 1;
    }

    int N, Np, Nc;
    try {
        N = std::stoi(argv[1]); // Converte argumento para int
        Np = std::stoi(argv[2]);
        Nc = std::stoi(argv[3]);
        if (N <= 0 || Np <= 0 || Nc <= 0) {
            std::cerr << "Erro: O tamanho do buffer, Np e Nc devem ser maiores que 0.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro: Argumentos invalidos fornecidos.\n";
        return 1;
    }

    buffer.resize(N);       // Inicializa o vetor com tamanho N

    // Inicialização dos semáforos
    sem_init(&sem_empty, 0, N); // Inicialmente N posições livres
    sem_init(&sem_full, 0, 0);  // Inicialmente 0 posições ocupadas
    sem_init(&sem_mutex, 0, 1); // Mutex inicializado com 1

    // Inicializando as threads de produtor e consumidor
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

    for (auto& p : produtores) p.join();
    for (auto& c : consumidores) c.join();

    // Destruição dos semáforos
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);

    return 0;
}
