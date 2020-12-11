#include <iostream>
#include <windows.h>
#include <utility>
#include <vector>

/**
 * Задача: Задача про экзамен. Преподаватель проводит экзамен у группы
 *         студентов. Каждый студент заранее знает свой билет и готовит по нему
 *         ответ. Подготовив ответ, он передает его преподавателю. Преподаватель
 *         просматривает ответ и сообщает студенту оценку. Требуется создать
 *         многопоточное приложение, моделирующее действия преподавателя и
 *         студентов. При решении использовать парадигму «клиент-сервер».
 *
 *  Модель: Клиенты и серверы – способ взаимодействия неравноправных потоков. Клиентский
 *          поток запрашивает сервер и ждет ответа. Серверный поток ожидает запроса от клиента,
 *          затем действует в соответствии с поступившим запросом.
 *
 * Вариант: 21
 * @author Романюк Андрей, БПИ-194
 */
using namespace std;
bool stopExam = false;    // Флаг остановки экзамена
CRITICAL_SECTION cs;    // Критическая секция для синхронизации вывода на экран

static const unsigned int defaultWait = 100; // Задержка между запросами в мс.
static const unsigned int minStudentEnters = 100; // Минимальное время мс для входа студента в аудиторию в мс.
static const unsigned int maxStudentEnters = 250; // Максимальное время для входа студента в аудиторию в мс.
static const unsigned int minStudentPrep = 4000; // Минимальное время подготовки студента к ответу в мс.
static const unsigned int maxStudentPrep = 10000; // Максимальное время подготовки студента к ответу в мс.
static const unsigned int minTeacherCheck = 3000; // Минимальное время проверки преподавателем студента в мс.
static const unsigned int maxTeacherCheck = 5000; // Максимальное время проверки преподавателем студента в мс.
static const unsigned int minMark = 0; // Минимальная оценка.
static const unsigned int maxMark = 10; // Максимальная оценка.
static const unsigned int countTickets = 100; // Количество билетов.
static const unsigned int maxStudents = 273; // Максимальное количество студентов. Реальные цифры с ФКН ПИ 2 курс :)

// Вывод сообщения
void showMessage(char *msg) {
    EnterCriticalSection(&cs);
    cout << msg << endl;
    LeaveCriticalSection(&cs);
}

// Функция потока студента(клиент)
DWORD WINAPI studentThread(PVOID param) {
    char buf[256];
    auto p = (DWORD) param;    // Переданный параметр
    int ticket = p >> 16;    // Номер билета
    int studNumber = p & 0xffff;    // Номер студенческого билета
    srand(ticket);

    // События для синхронизации с сервером
    HANDLE serverReady, exchange1, exchange2, serverAnswer;

    // Открытие событий сервера
    while ((serverReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("serverReady"))) == nullptr ||
           (exchange1 = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("exchange1"))) == nullptr ||
           (exchange2 = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("exchange2"))) == nullptr ||
           (serverAnswer = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("serverAnswer"))) == nullptr)
        Sleep(defaultWait);

    HANDLE mapFile;
    // Открытие общей области памяти для общения клиента с сервером
    while ((mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, LPCSTR("MyShared"))) == nullptr)
        Sleep(defaultWait);

    //Проецируем файл в адресное пространство процесса
    int *pData = (int*)MapViewOfFile(mapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

    wsprintfA(buf, "[Client] Студент №%d получил билет %d и начал готовиться.", studNumber, ticket);
    showMessage(buf);

    // Студент готовится:
    Sleep(minStudentPrep + rand() % (maxStudentPrep - minStudentPrep));

    // Когда готов, ожидает готовности сервера (преподавателя):
    WaitForSingleObject(serverReady, INFINITE);
    // Если сервер готов - садится отвечать:
    wsprintfA(buf, "[Client] Студент №%d c билетом %d садится отвечать.",
            studNumber, ticket);
    showMessage(buf);

    // Говорит серверу, кто это и какой билет:
    pData[0] = studNumber;
    pData[1] = ticket;

    // Подает событие, что данные готовы:
    SetEvent(exchange1);

    // Ожидает ответа сервера:
    WaitForSingleObject(serverAnswer, INFINITE);
    wsprintfA(buf, "[Client] Студент №%d получил оценку %d.", pData[0], pData[1]);
    showMessage(buf);

    //Подает сигнал серверу, что область общей памяти свободна и можно принимать следующего
    SetEvent(exchange2);

    // Овобождение ресурсов, занятых клиентом
    UnmapViewOfFile(pData);
    CloseHandle(mapFile);
    CloseHandle(serverAnswer);
    CloseHandle(exchange1);
    CloseHandle(exchange2);
    CloseHandle(serverReady);


    wsprintfA(buf, "[Client] Студент №%d выходит из аудитории.", studNumber);
    showMessage(buf);
    return 0;
}

// Функция потока преподавателя (сервер)
DWORD WINAPI teacherThread(PVOID param) {
    char buf[256];
    // Создание событий для синхронизации
    HANDLE serverReady = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("serverReady"));
    HANDLE exchange1 = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("exchange1"));
    HANDLE exchange2 = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("exchange2"));
    HANDLE serverAnswer = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("serverAnswer"));
    // Создание общей области памяти для общения клиентов с сервером
    HANDLE mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(int) * 2,
                                       LPCSTR("MyShared"));    //Создать проекцию на файл в памяти (используется файл подкачки)
    int *pData = (int*) MapViewOfFile(mapFile, FILE_MAP_READ | FILE_MAP_WRITE,
                                                 0, 0, 0);    //Проецируем файл в адресное пространство процесса
    // Пока экзамен не кончился:
    while (!stopExam) {
        // Установить событие что сервер готов
        SetEvent(serverReady);

        // Ожидаем пока кто-то из ожидающих клиентов заполнит общую область памяти
        WaitForSingleObject(exchange1, INFINITE);
        wsprintfA(buf, "[Server] Преподаватель начал принимать билет %d у студента №%d", pData[1], pData[0]);
        showMessage(buf);
        // Преподаватель принимает у студента рандомное время
        Sleep(rand() % (maxTeacherCheck - minTeacherCheck) + minTeacherCheck);

        pData[1] = rand() % (maxMark - minMark + 1) + minMark;    // Выставляет оценку за экзамен
        wsprintfA(buf, "[Server] Преподаватель поставил оценку %d студенту №%d", pData[1], pData[0]);
        showMessage(buf);

        // Устанавливает событие, что сервер ответил (экзамен сдан):
        SetEvent(serverAnswer);
        // Ожидает, пока клиент не подаст сигнал, что он прочитал переданные данные
        WaitForSingleObject(exchange2, INFINITE);
    }

    // Освобождение ресурсов, занятых сервером
    UnmapViewOfFile(pData);
    CloseHandle(mapFile);
    CloseHandle(serverAnswer);
    CloseHandle(exchange1);
    CloseHandle(exchange2);
    CloseHandle(serverReady);
    return 0;
}


int main() {
    DWORD idThread, tmp;

    setlocale(LC_ALL, "Russian");
    InitializeCriticalSection(&cs);

    // Создать поток-сервер
    HANDLE prep = CreateThread(nullptr, 0, teacherThread, nullptr, 0, &idThread);
    int n;
    cout << "Введите количество студентов: ";
    cin >> n;
    while (n <= 0 || n >= maxStudents) {
        if (n <= 0)
            cout << "Правильно. Какие очные экзамены во время пандемии?" << endl;
        else if (n >= maxStudents)
            cout << "У нас на програмной инженерии точно не больше " << maxStudents << " человек. Видимо, Вы ошиблись."
                 << endl;

        cout << "Введите повторно, пожалуйста: ";
        cin >> n;
    };

    auto *st = new HANDLE[n];
    //Создать потоки студентов
    cout << "Студенты начинают входить в аудиторию и готовиться." << endl;
    for (int i = 0; i < n; i++) {
        tmp = (rand() % (countTickets + 1) + 1) << 16;    //случайные номера билетов
        tmp |= (i + 1);        //и номера студентов упаковать в передаваемый параметр
        st[i] = CreateThread(NULL, 0, studentThread, (LPVOID) tmp, NULL, &idThread);

        Sleep(rand() % (maxStudentEnters - minStudentEnters) + minStudentEnters);
    }

    // Дожидаемся завершения всех потоков-клиентов
    if (n <= MAXIMUM_WAIT_OBJECTS)    // Либо всех сразу, если возможно
        WaitForMultipleObjects(n, st, TRUE, INFINITE);
    else {   // либо по частям
        int j = n;
        int k = 0;
        while (j) {
            if (j <= MAXIMUM_WAIT_OBJECTS) {
                WaitForMultipleObjects(j, &st[k], TRUE, INFINITE);
                j = 0;
            } else {
                WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, &st[k], TRUE, INFINITE);
                k += MAXIMUM_WAIT_OBJECTS;
                j -= MAXIMUM_WAIT_OBJECTS;
            }
        }
    }
    // Выставляем флаг окончания экзамена
    stopExam = true;
    cout << "Экзамен окончен. Всем спасибо." << endl;
    // Подчищаем за собой
    DeleteCriticalSection(&cs);
    delete[] st;
    system("pause");
    return 0;
}