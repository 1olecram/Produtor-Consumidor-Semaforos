#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <thread>
#include <chrono>
#include <semaphore.h>
#include <fstream>

int count = 0; // Itens no buffer
int in = 0;    // Índice de produção
int out = 0;   // Índice de consumo

std::vector<int> buffer; // Memória compartilhada (buffer)
std::vector<int> occupancy_log; // Log da ocupação do buffer após cada operação

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
        
        occupancy_log.push_back(count); // Salva log

        sem_post(&sem_mutex); // Sai da região crítica
        sem_post(&sem_full);  // Sinaliza nova posição ocupada
    }
}

void consumidor(int id, int num_itens) {
    for (int i = 0; i < num_itens; ++i) {
        sem_wait(&sem_full);  // Aguarda posição ocupada
        sem_wait(&sem_mutex); // Entra na região crítica

        int item = buffer[out];
        
        // Validando se é primo com a função já definida
         if (isPrime(item)) {
            // É primo
        } else {
            // Não é primo
        }
        out = (out + 1) % buffer.size(); // Libera a posição logicamente
        count--;
        
        occupancy_log.push_back(count); // Salva log

        sem_post(&sem_mutex); // Sai da região crítica
        sem_post(&sem_empty); // Sinaliza nova posição livre
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
    occupancy_log.reserve(2 * M); // Previne realocação durante execução

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

    auto start_time = std::chrono::high_resolution_clock::now();

    for (auto& p : produtores) p.join();
    for (auto& c : consumidores) c.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "Tempo de execução (" << Np << " produtores, " << Nc << " consumidores): " << duration.count() << " s\n";

    // Salvar log em arquivo
    std::string filename = "ocupacao_buffer_" + std::to_string(Np) + "p_" + std::to_string(Nc) + "c.csv";
    std::ofstream out_file(filename);
    if (out_file.is_open()) {
        out_file << "Operacao,Ocupacao\n";
        for (size_t i = 0; i < occupancy_log.size(); ++i) {
            out_file << i << "," << occupancy_log[i] << "\n";
        }
        out_file.close();
        std::cout << "Log salvo em " << filename << "\n";
    }

    // Destruição dos semáforos
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);

    return 0;
}
