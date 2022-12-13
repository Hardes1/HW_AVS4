#include <chrono>
#include <iostream> // необходимые библиотеки
#include <pthread.h>
#include <random>
#include <thread>
#include <vector>

pthread_t bear; // поток для медведя

const int max_capacity = 30; //  максимальное количество мёда в улее

const int minimal_bee_count = 1; // минимальное количество пчёл в улее

pthread_t bee[max_capacity]; // массив потоков для пчёл

int bee_index[max_capacity]; // массив индексов пчелы для логирования

int honey_count = 0; // изначальное количество мёда

int bee_count = 0; // изначальное количество пчёл в улее

pthread_mutex_t honey_mutex; // мьютекс для количества мёда

pthread_mutex_t bee_mutex; // мьютекс для количества пчёл

pthread_mutex_t condition_mutex; // мьютекс для условной переменной

pthread_cond_t condition_thread; // условный поток

std::mt19937 rnd(std::chrono::steady_clock::now()
                     .time_since_epoch()
                     .count()); // рандом, зависящий от времени

unsigned int getBigRandomInt() {
  return rnd() % 5000 + 2000;
} // генерация большого времени ожидания

unsigned int getSmallRandomInt() {
  return rnd() % 2000;
} // генерация малого времени ожидания

void *fill_honey(void *num) {
  const int bee_index = *(int *)num; // получаем индекс текущей пчелы
  while (true) { // бесконечный цикл для пчелы
    pthread_mutex_lock(&bee_mutex); // блокируем мьютекс для счётчика пчёл
    bee_count++;
    pthread_cond_broadcast(
        &condition_thread); // разблокируем условный мьютекс - в улее теперь
                            // хотя бы 2 пчелы
    std::cout << "Прилетела пчела " << bee_index
              << ". Общее число пчёл стало: " << bee_count << ".\n";
    pthread_mutex_unlock(
        &bee_mutex); // разблокировали мьютекс для счётчика пчёл
    std::this_thread::sleep_for(
        std::chrono::milliseconds(getSmallRandomInt())); // поток засыпает
    pthread_mutex_lock(&honey_mutex); // заблокируем мьютекс для счётчкиа мёда
    if (honey_count < max_capacity) {
      honey_count++;
      std::cout << "Пчела " << bee_index << " добавила мёд."
                << " Стало: " << honey_count << " единиц.\n";
    } else {
      std::cout << "Количество мёда превысило макисмальное значение!\n";
    }
    pthread_mutex_unlock(
        &honey_mutex); // разблокируем мьютекс для счётчика мёда
    std::this_thread::sleep_for(
        std::chrono::milliseconds(getSmallRandomInt())); // поток засыпает
    pthread_mutex_lock(&condition_mutex); // блокировка условного мьютекса
    while (bee_count == minimal_bee_count) {
      pthread_cond_wait(
          &condition_thread,
          &condition_mutex); // пчела не может улететь, если она одна
    }
    pthread_mutex_unlock(&condition_mutex); // разблокировка условного мьютекса
    pthread_mutex_lock(&bee_mutex);
    bee_count--;
    std::cout << "Улетела пчела " << bee_index
              << ". Общее число пчёл стало: " << bee_count
              << ".\n"; // уменьшаем количество пчёл
    pthread_mutex_unlock(&bee_mutex);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(getBigRandomInt())); // пчела "долго" ищет мёд
  }
}

void *take_honey(void *) {
  while (true) {
    pthread_mutex_lock(&honey_mutex); // блокировка мьютекса для счётчика мёда
    pthread_mutex_lock(&bee_mutex); // блокировка мьютекса для счётчика пчёл
    if (honey_count < max_capacity / 2) {
      std::cout << "Мёд ещё не скопился, Винни-пух спит.\n";
    } else if (bee_count >= 3) {
      std::cout << "Винни-пух пытается взять мёд." << '\n';
      std::cout
          << "Пчёлы покусали Винни-пуха, он идёт лечится.\n"; // проверка
                                                              // условий, что
                                                              // Винни-Пух
                                                              // может/не может
                                                              // забрать мёд
    } else {
      std::cout << "Винни-пух пытается взять мёд." << '\n';
      std::cout << "Винни-пух забирает весь мёд и уходит.\n";
      honey_count = 0;
    }
    pthread_mutex_unlock(&bee_mutex); // разблокировка мьютекс для счётчика пчёл
    pthread_mutex_unlock(
        &honey_mutex); // разблокировка мьютекса для счётчика мёда
    std::this_thread::sleep_for(std::chrono::milliseconds(
        getBigRandomInt())); // медведь продолжает спать
  }
}

int handle_arguments(int argc,
                     char **argv) { // функция для настройки ввода вывода
  int n = -1;
  try {
    if (argc == 1) {
      std::cin >> n;
    } else if (argc == 3 && std::string(argv[1]) == "-c") {
      n = std::stoi(argv[2]);
    }
  } catch (std::exception &e) {
    return n;
  }
  return n;
}

int main(int argc, char *argv[]) {
  int n = handle_arguments(argc, argv);
  if (n < 3 || n > max_capacity) {
    std::cout << "Некорректное число пчёл или некорректные данные. Завершаем "
                 "программу.\n"; // проверка на
                               // корректность
    return EXIT_SUCCESS;
  }
  pthread_mutex_init(&honey_mutex, nullptr);
  pthread_mutex_init(&bee_mutex, nullptr); // иницализация мьютексов
  pthread_mutex_init(&condition_mutex, nullptr);
  pthread_cond_init(&condition_thread, nullptr);
  for (int i = 0; i < n; ++i) {
    bee_index[i] = i + 1; // инициализурем массив индекса пчёл
  }
  for (int i = 0; i < n; ++i) {
    if (pthread_create(&bee[i], nullptr, fill_honey,
                       (void *)(bee_index + i))) { // создаём потоки пчёл
      std::cout << "Ошибка создания потока\n" << bee_index[i];
      return EXIT_FAILURE;
    }
  }
  if (pthread_create(&bear, nullptr, take_honey,
                     nullptr)) { // создаём поток медведя
    std::cout << "Ошибка создания потока Винни-Пух\n";
    return EXIT_FAILURE;
  }
  for (int i = 0; i < n; ++i) {
    if (pthread_join(bee[i], nullptr)) { // дожидаемся завершения потоков пчёл
      std::cout << "Ошибка ожидания потока\n" << i;
      return EXIT_FAILURE;
    }
  }
  if (pthread_join(bear, nullptr)) { //  дожидаемся завершения потока медведя
    std::cout << "Ошибка ожидания потока Винни-пух\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
