#include <iostream>
#include <cstdlib>

// 模 n 快速乘，用于快速幂，避免溢出
long long mod_mul(long long a, long long b, long long n)
{
    long long result = 0;
    a = a % n;
    while (b > 0)
    {
        if (b % 2 == 1)
        {
            result = (result + a) % n;
        }
        a = (a * 2) % n;
        b = b / 2;
    }
    return result;
}

// 模 n 快速幂，用于 Miller-Rabin k 次循环内的乘方运算
long long mod_ebp(long long base, long long ebp, long long n)
{
    long long result = 1;
    base = base % n;
    while (ebp > 0)
    {
        if (ebp % 2 == 1)
        {
            result = mod_mul(result, base, n);
        }
        ebp = ebp / 2;
        base = mod_mul(base, base, n);
    }
    return result;
}

bool miller_rabin(long long n, int iter)
{
    if (n <= 1)
        return false;
    if (n == 2 || n == 3)
        return true;
    if (n % 2 == 0)
        return false;

    // 把 n - 1 写成 n - 1 = 2^k * m 的形式
    long long d = n - 1;
    int k = 0;
    while (d % 2 == 0)
    {
        d /= 2;
        k++;
    }

    // 进行 iter 次测试（提高准确率）
    for (int i = 0; i < iter; i++)
    {
        // 随机选择 a，1 <= a <= n - 1
        long long a = 1 + rand() % (n - 1);
        long long b = mod_ebp(a, d, n);
        if (b == 1 || b == n - 1)
            continue;

        bool is_composite = true;
        for (int j = 0; j < k - 1; j++)
        {
            b = mod_ebp(b, 2, n);
            if (b == n - 1)
            {
                is_composite = false;
                break;
            }
        }

        if (is_composite)
            return false;
    }

    return true;
}

int main()
{
    long long n;
    std::cin >> n;

    if (miller_rabin(n, 16))
    {
        std::cout << "Yes" << std::endl;
    }
    else
    {
        std::cout << "No" << std::endl;
    }

    return 0;
}
