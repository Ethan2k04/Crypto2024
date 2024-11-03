#include <iostream>
#include <algorithm>

#define l 4
#define m 4
#define Nr 4
#define ASCII 48

using namespace std;

int SMAP[] = {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7};
int PMAP[] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};

string SPN(string x, string key_l)
{
    // Init w with x
    string w = x;
    // Nr-1 times iter
    string key;
    for (int r = 1; r < Nr; r++)
    {
        // Get thr key
        key = key_l.substr(4 * r - 4, l * m);
        // Bitwise w xor K
        string u;
        for (int i = 0; i < l * m; i++)
        {
            u.push_back(char((int(w[i]) - ASCII) ^ (int(key[i]) - ASCII) + ASCII));
        }
        string v;
        // Split str into m parts, process each
        for (int i = 0; i < m; i++)
        {
            string sub_str = u.substr(i * l, l);
            // Map binary value to hex value
            int value = 0;
            int base = 1;
            for (int j = l - 1; j >= 0; j--)
            {
                value += (int(sub_str[j]) - ASCII) * base;
                base *= 2;
            }
            // Get mapped value after S box
            int new_value = SMAP[value];
            string new_sub_str;
            // Map hex value to binary value
            for (int j = 0; j < l; j++)
            {
                new_sub_str.push_back(char(new_value % 2 + ASCII));
                new_value /= 2;
            }
            // Reverse because we push in the back, not front
            reverse(new_sub_str.begin(), new_sub_str.end());
            // Insert sub_str to the big one
            v.append(new_sub_str);
        }
        string new_w = w;
        // Apply perturbation
        for (int i = 0; i < l * m; i++)
        {
            new_w[PMAP[i]] = v[i];
        }
        w = new_w;
    }
    // The Nr time iter
    key = key_l.substr(4 * Nr - 4, l * m);
    // Bitwise w xor K
    string u;
    for (int i = 0; i < l * m; i++)
    {
        u.push_back(char((int(w[i]) - ASCII) ^ (int(key[i]) - ASCII) + ASCII));
    }
    // Apply S box
    string v;
    for (int i = 0; i < m; i++)
    {
        string sub_str = u.substr(i * l, l);
        // Map binary value to hex value
        int value = 0;
        int base = 1;
        for (int j = l - 1; j >= 0; j--)
        {
            value += (int(sub_str[j]) - ASCII) * base;
            base *= 2;
        }
        // Get mapped value after S box
        int new_value = SMAP[value];
        string new_sub_str;
        // Map hex value to binary value
        for (int j = 0; j < l; j++)
        {
            new_sub_str.push_back(char(new_value % 2 + ASCII));
            new_value /= 2;
        }
        // Reverse because we push in the back, not front
        reverse(new_sub_str.begin(), new_sub_str.end());
        // Insert sub_str to the big one
        v.insert(i * l, new_sub_str);
    }
    // The Nr+1 time iter
    key = key_l.substr(4 * Nr, l * m);
    // Bitwise v xor K
    string y;
    for (int i = 0; i < l * m; i++)
    {
        y.push_back(char((int(v[i]) - ASCII) ^ (int(key[i]) - ASCII) + ASCII));
    }
    return y;
}

int main()
{
    string _x;
    string _key;
    string x, key;
    getline(cin, _x);
    getline(cin, _key);
    for (int i = 0; i < _x.length(); i++)
    {
        if (_x[i] != ' ')
        {
            x.push_back(_x[i]);
        }
    }
    for (int i = 0; i < _key.length(); i++)
    {
        if (_key[i] != ' ')
        {
            key.push_back(_key[i]);
        }
    }
    string result = SPN(x, key);
    cout << result;
    return 0;
}