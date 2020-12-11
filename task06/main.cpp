#include <iostream>
#include <thread>
#include <utility>
#include <vector>
#include <algorithm>
#include <mutex>
#include "set"
#include "queue"

#pragma ide diagnostic ignored "EndlessLoop"
/*
* Задача о магазине - 2 (Вариант 21).
* ФИО: Романюк Андрей Сергеевич. БПИ-194
* ================ Условие ================
* В магазине работают два отдела, каждый отдел обладает уникальным ассортиментом. В
* каждом отделе работает один продавец. В магазин ходят исключительно забывчивые покупатели,
* поэтому каждый покупатель носит с собой список товаров, которые желает купить.
* Покупатель приобретает товары точно в том порядке, в каком они записаны в его списке.
* Продавец может обслужить только одного покупателя за раз. Покупатель, вставший в очередь, засыпает
* пока не дойдет до продавца. Продавец засыпает, если в его отделе нет покупателей, и просыпается,
* если появится хотя бы один. Создать  многопоточное приложение, моделирующее работу магазина.
* ================ Условие ================
*
*
*/

using namespace std;

// Количество купленных товаров покупателем:
static const int customerMin = 5;
static const int customerMax = 10;

// Время прибытия покупателя к продавцу:
static const int arrivalTimeMin = 1000;
static const int arrivalTimeMax = 10000;

// Максимальное количество товаров:
static const int maxProduct = 500;
static const int minProduct = 10;
// Введённое количество товаров:
static int countProducts;

class Customer {
private:
    int n;        // Размер списка покупок
    friend void walker(Customer *, int id);    // Поток покупателя
    mutex m;              // Мьютекс для условной переменной
public:
    int id;               // Номер покупателя.
    condition_variable cv;// Условная переменная.
    bool isSleep = true;  // Флаг, что покупатель спит.
    thread *th;           // Поток.
    Customer(int _id);       // Конструктор покупателя.
    vector<int> list;     // Список покупок.
};

class Department {
private:
    mutex ma, mb;    // Мьютексы доступа к ассортименту и очереди покупателей
    friend void seller(Department *, string name);    // Поток продавца

    set<int> assort;        // Ассортимент товаров
    queue<Customer *> q;       // Очередь покупателей
    thread *th;             // Поток продавца
    mutex m;                // Мьютекс для условной переменной

public:
    string name;            // Название отдела
    condition_variable cv;  // Условная переменная
    bool isSleep = true;    // Флаг, что продавец спит

    Department(vector<int> &data, string _name);    // Конструктор.
    bool check_assort(int val);                  // Проверить, есть ли товар в ассортименте.
    void add(Customer *customer);                          // Добавить покупателя в очередь.
};

mutex show;        // Мьютекс синхронизации вывода на экран
Department *a, *b; //

// Поток покупателя
void walker(Customer *customer, int id) {
    std::unique_lock<std::mutex> lock(customer->m);
    int i = 0;
    // Идем по списку покупок:
    while (i < customer->list.size()) {
        // Пока очередной товар по списку есть в отделе А:
        while (i < customer->list.size() && a->check_assort(customer->list[i])) {
            a->add(customer);    // Становися в очередь в этот отдел.
            show.lock();
            cout << "[Customer]: Покупатель " << customer->id << " стал в очередь в отдел " << a->name << " за товаром "
                 << customer->list[i]
                 << endl;
            show.unlock();
            // Пинаем продавца отдела, куда стали в очередь:
            a->isSleep = false;
            a->cv.notify_one();
            i++;
        }
        // Пока очередной товар по списку есть в отделе B:
        while (i < customer->list.size() && b->check_assort(customer->list[i])) {
            b->add(customer);    // Становися в очередь в этот отдел.
            show.lock();
            cout << "[Customer]: Покупатель " << customer->id << " стал в очередь в отдел " << b->name << " за товаром "
                 << customer->list[i]
                 << endl;
            show.unlock();
            // Пинаем продавца отдела, куда стали в очередь:
            b->isSleep = false;
            b->cv.notify_one();
            i++;
        }
    }
    // Стоим в очередях и забираем товары:
    for (int i = 0; i < customer->list.size(); i++) {
        // Цикл чтобы избежать случайного пробуждения:
        while (customer->isSleep)
            customer->cv.wait(lock);

        // Засыпаем:
        customer->isSleep = true;

        // Получили очередной товар:
        show.lock();
        cout << "[Customer]: Покупатель " << customer->id << " получил товар #" << customer->list[i] << endl;
        show.unlock();
    }
    show.lock();
    cout << "[Customer]: Покупатель " << customer->id << " полностью скупился и уходит." << endl;
    show.unlock();
}

// Конструктор покупателя:
Customer::Customer(int _id) {
    id = _id;
    n = rand() % (customerMax - customerMin) + customerMin;            // Размер списка покупок.

    for (int i = 0; i < n; i++)                                        // Генерируем список покупок.
        list.push_back(rand() % countProducts);

    // Выводим список покупок:
    show.lock();
    cout << "[Customer]: Покупатель #" << id << " со списком покупок: ";
    for (int i = 0; i < n; i++)
        cout << list[i] << " ";
    cout << endl;
    show.unlock();

    // Начинаем ходить по отделам, совершать покупки:
    th = new thread(walker, this, id);
}


void seller(Department *s, string name) {
    std::unique_lock<std::mutex> lock(s->m);

    // Уходим в бесконечный цикл обработки покупателей:
    while (true) {

        // Даём время продавку найти товар:
        this_thread::sleep_for(chrono::milliseconds(1000));
        int sz;

        // Берем размер очереди через мьютекс:
        s->mb.lock();
        sz = s->q.size();
        s->mb.unlock();
        if (sz) {
            // Ожидаем пробуждения:
            while (s->isSleep) {  // Цикл, чтобы избежать случайного пробуждения
                s->cv.wait(lock);
            }
            s->isSleep = true;        // Установить флаг продавец засыпает

        }
        // Когда проснулся, отпускает товар покупателю:
        s->mb.lock();
        if (!s->q.empty()) {
            show.lock();
            cout << "[Shop]: Продавец отдела " << name << " отдаёт товар покупателю " << s->q.front()->id << endl;
            show.unlock();
            // Пинает покупателя, чтобы тот збирал товар
            s->q.front()->isSleep = false;
            s->q.front()->cv.notify_one();
            s->q.pop();    // Удаляет покупателя из очереди

            // И сам себя пинает, чтобы проверить чтобы проверить пустая ли очередь
            s->isSleep = false;
            s->cv.notify_one();
        }
        s->mb.unlock();
    }
}

// Конструктор отдела
Department::Department(vector<int> &data, string _name) {
    name = std::move(_name);
    // Вывести ассотримент:
    show.lock();
    cout << "[Shop]: Отдел " << name << " имеет следующий ассортимент товара: ";
    for (int i : data) {
        cout << " " << i;
    }
    cout << endl;
    show.unlock();
    // Заполнить ассортимент:
    for (int &i : data) {
        ma.lock();
        assort.insert(i);
        ma.unlock();
    }

    // Создать поток продавца:
    th = new thread(seller, this, name);
}

// Проверить есть ли товар в ассортименте:
bool Department::check_assort(int val) {
    ma.lock();
    bool r = assort.find(val) != assort.end();
    ma.unlock();
    return r;
}

// Добавить покупателя в очередь:
void Department::add(Customer *customer) {
    mb.lock();
    q.push(customer);
    mb.unlock();
}

vector<int> getNumbers(int count) {
    vector<int> numbers;
    for (int i = 1; i <= count; ++i) {
        numbers.push_back(i);
    }
    return numbers;
}

int main() {
    setlocale(LC_ALL, "Russian");//русская локаль

    cout << "[MainThread]: Введите количество видов товара в двух магазинах: ";
    cin >> countProducts;
    while (countProducts < minProduct || countProducts > maxProduct) {
        if (countProducts < minProduct)
            cout << "[MainThread]: Что-то мало у вас товаров. Попробуйте ввести больше." << endl;
        else if (countProducts > maxProduct)
            cout << "[MainThread]: Не-не, тут Вы переборщили. Максимум можно ввести: " << maxProduct << endl;

        cout << "[MainThread]: Введите повторно, пожалуйста: ";
        cin >> countProducts;
    };

    vector<int> v;
    vector<int> numbers = getNumbers(countProducts);


    // Создание ассортимента для первого отдела:

    for (int i = 0; i < countProducts / 2; i++) {
        int rndPos = rand() % numbers.size();
        int z = numbers.at(rndPos);
        numbers.erase(numbers.begin() + rndPos);

        v.push_back(z);    // Добавляем их.
    }
    a = new Department(v, "A");    // Создать первый отдел
    v.clear();

    //Создание ассортимента для второго отдела

    //Берем только уникальные наименования товаров и чтобы их не было в первом отделе, т.е все оставшиеся.
    for (int number : numbers) {
        //добавляем их
        v.push_back(number);
    }

    b = new Department(v, "B");    //создать второй отдел
    int n;
    cout << "[MainThread]: Количество покупателей: ";
    cin >> n;
    vector<Customer *> customers;
    // Создать потоки покупателей:
    for (int i = 0; i < n; i++) {
        // Пауза перед новым покупателем:
        this_thread::sleep_for(chrono::milliseconds(arrivalTimeMin + rand() % (arrivalTimeMax - arrivalTimeMin)));
        cout << "[MainThread]: Покупатель #" << i + 1 << " зашёл в магазин." << endl;
        customers.push_back(new Customer(i + 1));
    }
    // Оожидаем их завершения:
    for (int i = 0; i < n; i++) {
        customers[i]->th->join();
    }

    // Подчищаем память:
    for (auto & customer : customers) {
        delete(customer);
    }
    customers.clear();
    delete(a);
    delete(b);
    
    system("pause");
    return 0;
}
