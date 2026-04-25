#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

int count = 0; // Itens no buffer
int in = 0;    // Índice de produção
int out = 0;   // Índice de consumo

std::vector<int> buffer; // Memória compartilhada (buffer)

std::mutex mtx;
std::condition_variable cv_prod, cv_cons;

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

void produtor(int id) {
    while (true) {
        int item = numGeneration(); // Produz item
        
        std::unique_lock<std::mutex> lock(mtx);

        // Espera enquanto o vetor estiver cheio
        cv_prod.wait(lock, [] { return count < static_cast<int>(buffer.size()); });

        buffer[in] = item;
        std::cout << "Produtor " << id << " colocou " << item << " na pos " << in << "\n";
        
        in = (in + 1) % buffer.size(); // Avança o índice circularmente
        count++;

        lock.unlock();
        cv_cons.notify_one();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

void consumidor(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        // Espera enquanto o vetor estiver vazio
        cv_cons.wait(lock, [] { return count > 0; });

        int item = buffer[out];
        std::cout << "Consumidor " << id << " retirou " << item << " da pos " << out;
        
        // Validando se é primo com a função já definida
        if (isPrime(item)) {
            std::cout << " (É primo!)\n";
        } else {
            std::cout << " (Não é primo)\n";
        }
        
        out = (out + 1) % buffer.size(); // Libera a posição logicamente
        count--;

        lock.unlock();
        cv_prod.notify_one();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <tamanho_do_buffer>\n";
        return 1;
    }

    int N;
    try {
        N = std::stoi(argv[1]); // Converte argumento para int
        if (N <= 0) {
            std::cerr << "Erro: O tamanho do buffer deve ser maior que 0.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro: Tamanho de buffer inválido fornecido.\n";
        return 1;
    }

    buffer.resize(N);       // Inicializa o vetor com tamanho N

    // Inicializando as threads de produtor e consumidor
    std::thread p1(produtor, 1);
    std::thread c1(consumidor, 1);

    p1.join();
    c1.join();

    return 0;
}
