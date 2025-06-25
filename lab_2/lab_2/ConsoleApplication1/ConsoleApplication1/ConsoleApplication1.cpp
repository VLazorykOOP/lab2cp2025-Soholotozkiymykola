#include <iostream>      
#include <thread>        // для роботи з потоками
#include <chrono>        // для вимірювання часу та затримок
#include <cmath>         // для математичних функцій (cos, sin)
#include <random>        // для генерації випадкових чисел
#include <mutex>         // для забезпечення синхронного доступу до cout

using namespace std;

double M_PI = 3.14; // Оголошення числа π вручну (можна точніше: 3.14159...)

random_device rd;           // джерело ентропії
mt19937 gen(rd());          // генератор псевдовипадкових чисел на основі rd
uniform_real_distribution<> angle_dist(0, 2 * M_PI); // рівномірний розподіл від 0 до 2π (кут у радіанах)

mutex cout_mutex; // м'ютекс для синхронізації виводу в консоль

// Функція, яка моделює рух розробника з хаотично змінюваним напрямком
void developer(int id, int N, int V) {
    double x = 0.0, y = 0.0;                         // початкова позиція
    double angle = angle_dist(gen);                 // початковий випадковий напрямок
    auto last_change = chrono::steady_clock::now(); // час останньої зміни напряму

    while (true) {
        // Якщо пройшло N секунд — змінюємо напрямок на новий випадковий
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - last_change).count() >= N) {
            angle = angle_dist(gen);
            last_change = now;
        }

        // Зміна позиції згідно з напрямком та швидкістю
        x += V * cos(angle);
        y += V * sin(angle);

        // Потокобезпечний вивід інформації
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Developer #" << id << " position: (" << x << ", " << y << ")\n";
        }

        this_thread::sleep_for(chrono::milliseconds(1000)); // затримка 1 секунда
    }
}

// Функція, яка моделює рух менеджера по колу радіуса R зі швидкістю V
void manager(int id, double R, double V) {
    double angle = 0.0;              // початковий кут
    double angular_speed = V / R;    // кутова швидкість ω = v / r

    while (true) {
        // Обчислення координат за формулами руху по колу
        double x = R * cos(angle);
        double y = R * sin(angle);

        // Потокобезпечний вивід позиції
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Manager #" << id << " position: (" << x << ", " << y << ")\n";
        }

        // Зміна кута (рух по колу)
        angle += angular_speed;
        if (angle >= 2 * M_PI) angle -= 2 * M_PI; // оберт назад до [0; 2π]

        this_thread::sleep_for(chrono::milliseconds(1000)); // затримка 1 секунда
    }
}

int main() {
    const int num_devs = 3;   // кількість розробників
    const int num_mgrs = 2;   // кількість менеджерів

    const int N = 3;          // розробник змінює напрямок кожні N секунд
    const int V_dev = 10;     // швидкість розробника (в пікселях/одиницях)
    const int V_mgr = 5;      // швидкість менеджера
    const double R = 50;      // радіус кола для менеджера

    thread developers[num_devs]; // масив потоків для розробників
    thread managers[num_mgrs];   // масив потоків для менеджерів

    // Запуск потоків розробників
    for (int i = 0; i < num_devs; i++) {
        developers[i] = thread(developer, i + 1, N, V_dev);
    }

    // Запуск потоків менеджерів
    for (int i = 0; i < num_mgrs; i++) {
        managers[i] = thread(manager, i + 1, R, V_mgr);
    }

    // Очікування завершення всіх потоків (програма буде працювати нескінченно)
    for (int i = 0; i < num_devs; i++) {
        developers[i].join();
    }

    for (int i = 0; i < num_mgrs; i++) {
        managers[i].join();
    }

    return 0;
}
