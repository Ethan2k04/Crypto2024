import random
from collections import defaultdict

# S盒及其逆
S_BOX = [14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7]
INV_S_BOX = [S_BOX.index(x) for x in range(16)]

# 置换P盒
P_BOX = [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15]

# 密钥
INITIAL_KEY = 0x3A94D63F  # 32位初始密钥
KEYS = []  # 存储生成的轮密钥


# 生成轮密钥
def generate_keys(initial_key):
    global KEYS
    for r in range(0, 5):  # 生成5轮密钥
        key = (initial_key >> (16 - 4 * r)) & 0xFFFF  # 从第4r位开始取连续16位
        KEYS.append(key)


generate_keys(INITIAL_KEY)


# 轮函数
def round_function(input_block, key, i):
    state = input_block ^ key
    state = substitute(state)
    state = permute(state)
    return state


# 代换函数（S盒）
def substitute(block):
    output = 0
    for i in range(4):
        nibble = (block >> (i * 4)) & 0xF
        output |= S_BOX[nibble] << (i * 4)
    return output


# 置换函数（P层）
def permute(block):
    output = 0
    for i in range(16):
        bit = (block >> (15 - i)) & 1  # 获取从高位到低位的位
        output |= bit << (15 - P_BOX[i])  # 根据 P_BOX 进行置换
    return output


# SPN加密
def spn_encrypt(plaintext, keys):
    state = plaintext
    for i in range(3):  # 前3轮
        state = round_function(state, keys[i], i)
    state ^= keys[-2]
    state = substitute(state)  # 第4轮不进行P置换
    state ^= keys[-1]  # 最后一轮（第5轮）只进行密钥加操作
    return state


# 生成随机明文和密文对，并写入文件
def generate_plaintext_ciphertext_pairs(num_pairs, filename):
    pairs = []
    with open(filename + ".csv", "w") as f:
        for _ in range(num_pairs):
            plaintext = random.randint(0, 0xFFFF)
            ciphertext = spn_encrypt(plaintext, KEYS)
            pairs.append((plaintext, ciphertext))
            f.write(f"{hex(plaintext)},{hex(ciphertext)}\n")  # 写入明文和密文对
    return pairs


# 从文件读取明文-密文对
def read_plaintext_ciphertext_pairs(filename):
    pairs = []
    with open(filename, "r") as f:
        for line in f:
            plaintext, ciphertext = line.strip().split(",")
            plaintext = int(plaintext, 16)
            ciphertext = int(ciphertext, 16)
            pairs.append((plaintext, ciphertext))
    return pairs


# 线性攻击分析
def linear_attack(pairs, num_pairs):
    # 先分析第24位
    key_24_count = defaultdict(int)  # 用于存储每个密钥猜测的正确统计次数

    # 对每个明文密文对进行分析
    for plaintext, ciphertext in pairs[:num_pairs]:
        # 先处理输入，获取到第578位
        x5 = (plaintext >> 11) & 0x1
        x7 = (plaintext >> 9) & 0x1
        x8 = (plaintext >> 8) & 0x1
        # 遍历所有可能的最后一轮密钥24位猜测
        for L1 in range(16):
            for L2 in range(16):
                y2 = (ciphertext >> 8) & 0xF
                y4 = ciphertext & 0xF
                u2 = INV_S_BOX[L1 ^ y2]
                u4 = INV_S_BOX[L2 ^ y4]
                z = (x5 ^ x7 ^ x8 ^ (u2 >> 2) ^ u2 ^ (u4 >> 2) ^ u4) & 0x1
                if z == 0x0:
                    key_24_count[(L1, L2)] += 1

    # 偏差计算: 偏差 = |正确计数 - num_pairs / 2|
    key_bias = {
        key: abs(count - (num_pairs / 2)) / num_pairs
        for key, count in key_24_count.items()
    }

    # 按偏差从大到小排序，显示前十个密钥猜测和偏差值
    top_32_keys = sorted(key_bias.items(), key=lambda x: x[1], reverse=True)[:32]

    return top_32_keys


# 测试线性攻击
def test_linear_attack():
    opcode = input(
        "Please enter opcode:\n1 - Generate SPN pairs 2 - Do linear attack: "
    )
    if opcode == "1":
        num_pairs = int(
            input("Please enter the number of plaintext-ciphertext pairs: ")
        )
        filename = input("Please enter the filename to save the pairs: ")
        generate_plaintext_ciphertext_pairs(num_pairs, filename)
    elif opcode == "2":
        num_pairs = int(input("Please enter the number of pairs to process: "))
        filename = input("Please enter the filename of pairs: ")
        pairs = read_plaintext_ciphertext_pairs(filename)
        top_32_keys = linear_attack(pairs, num_pairs)
        print("Top 32 key guesses with highest bias:")
        for key, bias in top_32_keys:
            print(f"Key guess: {hex(key[0]), hex(key[1])}, Bias: {bias}")


test_linear_attack()
