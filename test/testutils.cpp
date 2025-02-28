#include <iostream>
#include "utils.hpp"
#include <climits>
#include "Timer.hpp"
using namespace std;

int LenOfInt(int num) {
    int len = 1;
    while (num >= 10) {
        ++len;
        num /= 10;
    }
    return len;
}

int LenOfIntFast(int num) {
    if (num == 0) return 1;
    num = abs(num);

    // 计算二进制前导零数量（仅对正整数有效）
    int clz = __builtin_clz(num | 1); // |1 避免 num=0 时的未定义行为
    int log2_approx = (sizeof(int) * CHAR_BIT - 1) - clz;
    int digits_approx = (log2_approx * 1233) >> 12; // 近似 log10(2) ≈ 0.3010 * 4096 = 1233

    // 修正近似误差
    static const int thresholds[] = {
        9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999, INT_MAX
    };
    return (num > thresholds[digits_approx - 1]) ? digits_approx + 1 : digits_approx;
}

int main()
{
    Timer::getInstance().start();
    for (int i = 0; i < 10000000; i++) {
        LenOfInt(i);
    }
    Timer::getInstance().stop();
    cout << "LenOfInt: " << Timer::getInstance().getDuration() << "ms" << endl;

    Timer::getInstance().start();
    for (int i = 0; i < 10000000; i++) {
        LenOfIntFast(i);
    }
    Timer::getInstance().stop();
    cout << "LenOfIntFast: " << Timer::getInstance().getDuration() << "ms" << endl;

    return 0;
}