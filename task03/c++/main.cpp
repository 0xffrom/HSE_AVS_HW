#define _USE_MATH_DEFINES

#include <math.h>
#include <iostream>

using namespace std;

int main() {
    double _x = 0.5;
    double eps = 0.0005; // Точность
    double s = 0;
    int n = 0;
    double a = _x;
    do {
        s += a;
        a = a * _x * _x * (2 * n - 1) * (2 * n) / 4.0 / (n * n) * (2 * (n - 1) + 1) / (2 * n + 1);
    } while (fabs(a) >= eps);

    cout << M_PI / 2 - s << endl;
    cout << acos(_x) << endl;

    system("pause");
    return 0;
}