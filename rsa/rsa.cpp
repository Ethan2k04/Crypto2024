#include <iostream>

using namespace std;

// 快速乘算法
long long fastMul(long long a, long long b, long long mod)
{
    long long result = 0;
    a = a % mod;
    while (b > 0)
    {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result;
}

// 快速幂算法
long long fastExp(long long base, long long exp, long long mod)
{
    long long result = 1;
    base = base % mod;
    while (exp > 0)
    {
        if (exp % 2 == 1)
            result = fastMul(result, base, mod);
        base = fastMul(base, base, mod);
        exp /= 2;
    }
    return result;
}

// 扩展欧几里得算法
long long modInverse(long long e, long long phi)
{
    long long t = 0, new_t = 1;
    long long r = phi, new_r = e;

    while (new_r != 0)
    {
        long long quotient = r / new_r;
        long long temp_t = new_t;
        new_t = t - quotient * new_t;
        t = temp_t;
        long long temp_r = new_r;
        new_r = r - quotient * new_r;
        r = temp_r;
    }

    if (r > 1)
        return -1;
    if (t < 0)
        t += phi;
    return t;
}

int main()
{
    long long p, q;
    cin >> p >> q;
    long long e, c;
    cin >> e >> c;
    long long n = p * q;
    long long phi = (p - 1) * (q - 1);
    long long d = modInverse(e, phi);
    long long m = fastExp(c, d, n);
    cout << m << endl;
    return 0;
}
