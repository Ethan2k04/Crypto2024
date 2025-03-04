#include <iostream>
#include <cmath>

using namespace std;

// 椭圆曲线参数
const int p = 11;
const int a = 1;
const int b = 6;
const int q = 13;

// 求逆元 (扩展欧几里得算法)
int mod_inverse(int a, int m)
{
    int t = 0, new_t = 1;
    int r = m, new_r = a;
    while (new_r != 0)
    {
        int quotient = r / new_r;
        t -= quotient * new_t;
        r -= quotient * new_r;
        swap(t, new_t);
        swap(r, new_r);
    }
    if (r > 1)
        return -1;
    if (t < 0)
        t += m;
    return t;
}

// 椭圆曲线加法
pair<int, int> elliptic_add(int x1, int y1, int x2, int y2)
{
    if (x1 == -1 || y1 == -1)
        return {x2, y2};
    if (x2 == -1 || y2 == -1)
        return {x1, y1};

    int m;
    if (x1 == x2 && y1 == y2)
    {
        // 点相同，计算斜率 m = (3 * x1^2 + a) / (2 * y1) mod p
        int numerator = (3 * x1 * x1 + a) % p;
        int denominator = mod_inverse(2 * y1, p);
        m = (numerator * denominator) % p;
    }
    else
    {
        // 点不同，计算斜率 m = (y2 - y1) / (x2 - x1) mod p
        int numerator = (y2 - y1 + p) % p;
        int denominator = mod_inverse((x2 - x1 + p) % p, p);
        m = (numerator * denominator) % p;
    }

    // 计算新的点 (x3, y3)
    int x3 = (m * m - x1 - x2) % p;
    int y3 = (m * (x1 - x3) - y1) % p;

    return {(x3 + p) % p, (y3 + p) % p};
}

// 椭圆曲线标量乘法（乘法变连加）
pair<int, int> scalar_mult(int k, int x, int y)
{
    pair<int, int> result = {-1, -1};
    pair<int, int> addend = {x, y};

    while (k > 0)
    {
        if (k % 2 == 1)
        {
            result = elliptic_add(result.first, result.second, addend.first, addend.second);
        }
        addend = elliptic_add(addend.first, addend.second, addend.first, addend.second);
        k /= 2;
    }

    return result;
}

// 哈希函数 hash(x) = 2^x mod q（每次乘法操作都要取模）
int hash_message(int x)
{
    int result = 1;
    int base = 2;
    while (x > 0)
    {
        // 如果 x 是奇数，将 base 乘入 result
        if (x % 2 == 1)
        {
            result = (result * base) % q;
        }
        base = (base * base) % q;
        x /= 2;
    }
    return result;
}

// 生成签名
pair<int, int> generate_signature(int x0, int y0, int m, int x, int k)
{
    // 计算 r = k * A mod q
    auto R = scalar_mult(k, x0, y0);
    int r = R.first % q;
    // 如果 r == 0 则重新计算
    if (r == 0)
        return generate_signature(x0, y0, m, x, k + 1);

    // 计算 s = k^(-1) * (H(m) + r * x) mod q
    int z = hash_message(x);
    int s = mod_inverse(k, q) * (z + r * m) % q;
    // 如果 s == 0 则重新计算
    if (s == 0)
        return generate_signature(x0, y0, m, x, k + 1);

    return {r, s};
}

int main()
{
    int x0, y0;
    cin >> x0 >> y0;
    int m, x, k;
    cin >> m >> x >> k;
    auto signature = generate_signature(x0, y0, m, x, k);
    cout << signature.first << " " << signature.second << endl;

    return 0;
}
