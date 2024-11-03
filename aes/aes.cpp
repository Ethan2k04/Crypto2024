#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

const int Nb = 4;  // 一个状态中包含列（32bit）的个数
const int Nk = 4;  // 一个轮密钥中包含字（32bit）的个数
const int Nr = 10; // AES加密的轮数

// 替换矩阵sbox，使用二位矩阵存储
const unsigned char sbox[16][16] = {
    {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
    {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
    {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
    {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
    {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
    {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
    {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
    {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
    {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
    {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
    {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
    {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
    {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
    {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
    {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
    {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}};

// 轮常数rcon，用于密钥扩展，每个元素代表该轮中使用的常量
const unsigned char rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

// FieldMult函数 - 域F28中的乘法
// @param x: 第一个乘数
// @param y: 第二个乘数
// @return: x 和 y 的乘积
unsigned char FieldMult(unsigned char x, unsigned char y)
{
    unsigned char result = 0;
    while (y)
    {
        if (y & 1)
        {
            result ^= x;
        }
        x = (x << 1) ^ ((x & 0x80) ? 0x1b : 0);
        y >>= 1;
    }
    return result;
}

// RotWord函数 - 旋转字的字节
// @param word: 需要旋转的字，包含4个字节
void RotWord(unsigned char word[])
{
    unsigned char temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

// AddWord函数 - 向临时字节数组添加轮常数
// @param temp: 需要添加轮常数的字节数组
// @param i: 当前轮次
void AddWord(unsigned char temp[], int i)
{
    temp[0] ^= rcon[i / Nk];
}

// SubWord函数 - 字节替代操作
// @param word: 需要进行替代的字，包含4个字节
void SubWord(unsigned char word[])
{
    for (int i = 0; i < 4; i++)
    {
        word[i] = sbox[(word[i] & 0xF0) >> 4][word[i] & 0x0F];
    }
}

// AddRoundKey函数 - 将轮密钥与状态进行异或操作
// @param state: 当前状态矩阵，注意state是列主序存储的
// @param roundKey: 当前轮密钥矩阵，注意roundKey是行主序存储的
void AddRoundKey(unsigned char state[4][4], const unsigned char roundKey[4][4])
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            state[j][i] ^= roundKey[i][j];
        }
    }
}

// SubBytes函数 - 进行字节替代
// @param state: 当前状态矩阵
void SubBytes(unsigned char state[4][4])
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            state[j][i] = sbox[(state[j][i] & 0xF0) >> 4][(state[j][i] & 0x0F)];
        }
    }
}

// ShiftRows函数 - 将state[4][4]的第i行每个元素左移i位
// @param state: 当前状态矩阵
void ShiftRows(unsigned char state[4][4])
{
    unsigned char temp;

    // 第一行
    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    // 第二行
    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    // 第三行
    temp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = temp;
}

// MixColumns函数 - 对状态矩阵的列进行混合
// @param state: 当前状态矩阵
void MixColumns(unsigned char state[4][4])
{
    // temp - 4x4存储的矩阵，存储MixColumns后的每一列
    unsigned char temp[4][4];

    for (int i = 0; i < 4; ++i)
    {
        temp[0][i] = FieldMult(state[0][i], 0x02) ^
                     FieldMult(state[1][i], 0x03) ^
                     state[2][i] ^
                     state[3][i];
        temp[1][i] = state[0][i] ^
                     FieldMult(state[1][i], 0x02) ^
                     FieldMult(state[2][i], 0x03) ^
                     state[3][i];
        temp[2][i] = state[0][i] ^
                     state[1][i] ^
                     FieldMult(state[2][i], 0x02) ^
                     FieldMult(state[3][i], 0x03);
        temp[3][i] = FieldMult(state[0][i], 0x03) ^
                     state[1][i] ^
                     state[2][i] ^
                     FieldMult(state[3][i], 0x02);
    }

    memcpy(state, temp, 16);
}

// KeyExpansion函数 - 扩展密钥
// @param key: 原始128位密钥
// @param w: 扩展后的轮密钥
void KeyExpansion(const unsigned char *key, unsigned char *w)
{
    for (int i = 0; i < Nk; ++i)
    {
        w[i * 4] = key[i * 4];
        w[i * 4 + 1] = key[i * 4 + 1];
        w[i * 4 + 2] = key[i * 4 + 2];
        w[i * 4 + 3] = key[i * 4 + 3];
    }

    for (int i = Nk; i < Nb * (Nr + 1); ++i)
    {
        unsigned char temp[4];
        for (int j = 0; j < 4; ++j)
        {
            temp[j] = w[(i - 1) * 4 + j];
        }

        if (i % Nk == 0)
        {
            RotWord(temp);
            SubWord(temp);
            AddWord(temp, i);
        }
        w[i * 4] = w[(i - Nk) * 4] ^ temp[0];
        w[i * 4 + 1] = w[(i - Nk) * 4 + 1] ^ temp[1];
        w[i * 4 + 2] = w[(i - Nk) * 4 + 2] ^ temp[2];
        w[i * 4 + 3] = w[(i - Nk) * 4 + 3] ^ temp[3];
    }
}

// aes_encrypt函数 - AES加密算法
// @param input: 明文输入
// @param key: 加密密钥
// @param output: 加密后的密文输出
void aes_encrypt(const unsigned char *input, const unsigned char *key, unsigned char *output)
{
    unsigned char state[4][4];
    unsigned char w[176];

    KeyExpansion(key, w);

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            state[j][i] = input[i * 4 + j];
        }
    }

    AddRoundKey(state, (unsigned char(*)[4]) & w[0]);

    for (int round = 1; round < Nr; ++round)
    {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, (unsigned char(*)[4]) & w[round * 16]);
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, (unsigned char(*)[4]) & w[Nr * 16]);

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            output[i * 4 + j] = state[j][i];
        }
    }
}

// printHex函数 - 打印十六进制数据
// @param data: 数据缓冲区
// @param length: 数据长度
void printHex(const unsigned char *data, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<int>(data[i]);
    }
    std::cout << std::endl;
}

int main()
{
    const int maxLength = 64;
    char *keyHex = new char[maxLength];
    char *plainTextHex = new char[maxLength];

    std::cin.getline(keyHex, maxLength);
    std::cin.getline(plainTextHex, maxLength);

    unsigned char key[16];
    unsigned char plainText[16];
    unsigned char cipherText[16];

    for (int i = 0; i < 16; ++i)
    {
        // sscanf用于从字符串中读取格式化的数据，将16进制转换为10进制
        sscanf(keyHex + 2 * i, "%2hhx", &key[i]);
        sscanf(plainTextHex + 2 * i, "%2hhx", &plainText[i]);
    }

    aes_encrypt(plainText, key, cipherText);

    printHex(cipherText, 16);

    delete keyHex;
    delete plainTextHex;

    return 0;
}