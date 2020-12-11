#include <functional>
#include <iostream>
#include <windows.h>
#include <vector>
#include "omp.h"
#include <chrono>

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
static const unsigned int defaultWait = 100; // Задержка между запросами в мс.
static const unsigned int minStudentPrep = 4000; // Минимальное время подготовки студента к ответу в мс.
static const unsigned int maxStudentPrep = 10000; // Максимальное время подготовки студента к ответу в мс.
static const unsigned int minTeacherCheck = 3000; // Минимальное время проверки преподавателем студента в мс.
static const unsigned int maxTeacherCheck = 5000; // Максимальное время проверки преподавателем студента в мс.
static const unsigned int minMark = 1; // Минимальная оценка.
static const unsigned int maxMark = 10; // Максимальная оценка.
static const unsigned int countTickets = 100; // Количество билетов.
static const unsigned int maxStudents = 273; // Максимальное количество студентов. Реальные цифры с ФКН ПИ 2 курс :)
int leftStudents;


class request {
public:
    int ticket;
    int studNumber;

    request(int ticket, int studNumber) : ticket(ticket), studNumber(studNumber) {
        //
    }
};

class response {
public:
    int mark;
    int studNumber;

    response(int mark, int studNumber) : mark(mark), studNumber(studNumber) {
        //
    }

};

request *generalRequest = nullptr;
response *generalResponse = nullptr;


// Вывод сообщения
void showMessage(string msg) {
#pragma omp critical
    cout << msg << endl;
}

// Функция потока студента(клиент)
void studentThread(request *r) {
    char buf[256];

    // События для синхронизации с сервером
    HANDLE serverReady, exchange1, exchange2, serverAnswer;

    // Открытие событий сервера
    while ((serverReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("serverReady"))) == nullptr ||
           (exchange1 = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("exchange1"))) == nullptr ||
           (exchange2 = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("exchange2"))) == nullptr ||
           (serverAnswer = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("serverAnswer"))) == nullptr)
        Sleep(defaultWait);


    wsprintfA(buf, "[Client] Студент №%d получил билет %d и начал готовиться.", r->studNumber, r->ticket);
    showMessage(buf);

    // Студент готовится:
    Sleep(minStudentPrep + rand() % (maxStudentPrep - minStudentPrep));

    // Когда готов, ожидает готовности сервера (преподавателя):
    WaitForSingleObject(serverReady, INFINITE);
    // Если сервер готов - садится отвечать:

    wsprintfA(buf, "[Client] Студент №%d c билетом %d садится отвечать.", r->studNumber, r->ticket);
    showMessage(buf);

    generalRequest = r;

    // Подает событие, что данные готовы:
    SetEvent(exchange1);

    // Ожидает ответа сервера:
    WaitForSingleObject(serverAnswer, INFINITE);

    wsprintfA(buf, "[Client] Студент №%d получил оценку %d.", generalResponse->studNumber, generalResponse->mark);
    showMessage(buf);

    //Подает сигнал серверу, что область общей памяти свободна и можно принимать следующего
    SetEvent(exchange2);

    // Овобождение ресурсов, занятых клиентом
    generalRequest = nullptr;
    delete (r);
    CloseHandle(serverAnswer);
    CloseHandle(exchange1);
    CloseHandle(exchange2);
    CloseHandle(serverReady);

    wsprintfA(buf, "[Client] Студент №%d выходит из аудитории.", r->studNumber);
    showMessage(buf);
}

// Функция потока преподавателя (сервер)
void teacherThread() {
    char buf[256];
    HANDLE serverReady = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("serverReady"));
    HANDLE exchange1 = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("exchange1"));
    HANDLE exchange2 = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("exchange2"));
    HANDLE serverAnswer = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("serverAnswer"));
    // Пока экзамен не кончился:
    while (leftStudents > 0) {
        // Установить событие что сервер готов
        SetEvent(serverReady);

        // Ожидаем пока кто-то из ожидающих клиентов заполнит общую область памяти
        WaitForSingleObject(exchange1, INFINITE);

        wsprintfA(buf, "[Server] Преподаватель начал принимать билет %d у студента №%d", generalRequest->ticket,
                  generalRequest->studNumber);
        showMessage(buf);
        // Преподаватель принимает у студента рандомное время
        Sleep(rand() % (maxTeacherCheck - minTeacherCheck) + minTeacherCheck);

        generalResponse = new response(rand() % (maxMark - minMark + 1) + minMark, generalRequest->studNumber);
        // Выставляет оценку за экзамен
        wsprintfA(buf, "[Server] Преподаватель поставил оценку %d студенту №%d", generalResponse->mark,
                  generalResponse->studNumber);
        showMessage(buf);

        // Устанавливает событие, что сервер ответил (экзамен сдан):
        SetEvent(serverAnswer);
        // Ожидает, пока клиент не подаст сигнал, что он прочитал переданные данные
        WaitForSingleObject(exchange2, INFINITE);
        --leftStudents;
        delete (generalResponse);
    }

    // Освобождение ресурсов, занятых сервером
    delete (generalRequest);
}

/**
 * Метод для генерации рандомно числа с разным сидом для потоков.
 * Так как два потока с лёгкостью могли получать одинаковые значения, то было принято решение генерарировать новый сид
 * по их айдишнику и с некой битовой магией.
 * @param lim - Максимальное число, не включая его.
 * @param threadId - айдишник треда.
 * @return
 */
int random(int lim, int threadId) {
    srand((threadId << 5) | 4096);
    return rand() % lim;
}

// Выход:
int exit() {
    cout << "Экзамен окончен. Всем спасибо." << endl;
    delete (generalRequest);
    delete (generalResponse);
    system("pause");
    exit(0);
}

int main() {
    setlocale(LC_ALL, "Russian");
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

    leftStudents = n;
    cout << "Студенты начинают входить в аудиторию и готовиться." << endl;
#pragma omp parallel
    {
        if (omp_get_thread_num() == 0) {
            teacherThread();
            exit();
        }
#pragma omp for
        for (int i = 0; i < n + 1; ++i) {
            studentThread(new request(random(countTickets, i), i));
        }
    }
}



