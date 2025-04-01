#include "RSAUtils.h"

#include "BigInt.hpp"

#include <cctype>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// 辅助函数：计算一个 BigInt 的二进制位数
int bitLength(const BigInt &n)
{
    std::stringstream ss;
    // 将 n 转换为16进制字符串
    ss << std::hex << n;
    std::string hexStr = ss.str();
    if (hexStr == "0") return 0;

    // 根据首位字符计算实际位数（最高有效位可能不足4位）
    int firstDigit = 0;
    char c = hexStr[0];
    if (c >= '0' && c <= '9')
        firstDigit = c - '0';
    else
        firstDigit = std::tolower(c) - 'a' + 10;

    // 防止负值位移，首先确保 firstDigit 为正
    int bitsInFirst = 0;
    if (firstDigit > 0)
    { // 添加此检查
        while (firstDigit)
        {
            bitsInFirst++;
            firstDigit >>= 1;
        }
    }

    return (hexStr.size() - 1) * 4 + bitsInFirst;
}

std::string hexToDec(const std::string &hexStr)
{
    BigInt decimalValue = 0;

    // 一次性计算，而不是重复乘以16
    for (char c : hexStr)
    {
        decimalValue *= 16;

        int digit = 0;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'A' && c <= 'F')
            digit = 10 + (c - 'A');
        else if (c >= 'a' && c <= 'f')
            digit = 10 + (c - 'a');
        else
            throw std::invalid_argument("Invalid hex character");

        decimalValue += digit;
    }

    return decimalValue.to_string();
}

std::string decToHex(const std::string &decStr)
{
    // 将输入的十进制字符串转换为 BigInt
    BigInt decimalValue(decStr);

    // 存储结果
    std::string hexStr;

    // 逐步除以 16，并将余数转换为十六进制字符
    while (decimalValue != 0)
    {
        BigInt remainder = decimalValue % 16; // 获取余数
        decimalValue /= 16;                   // 更新除数

        // 将余数转换为对应的十六进制字符
        if (remainder < 10)
        {
            hexStr.push_back('0' + remainder.to_int()); // 数字 0-9
        }
        else
        {
            hexStr.push_back('a' + (remainder.to_int() - 10)); // 小写字母 a-f
        }
    }

    // 如果 hexStr 是空的，意味着输入是 0，直接返回 "0"
    if (hexStr.empty())
    {
        return "0";
    }

    // 反转结果字符串（因为计算过程中，低位在前）
    std::reverse(hexStr.begin(), hexStr.end());

    return hexStr;
}

// 模幂运算：计算 (base^exponent) mod modulus
BigInt modPow(BigInt base, BigInt exponent, const BigInt &modulus)
{
    BigInt result = 1;
    base = base % modulus;
    while (exponent > 0)
    {
        if ((exponent % 2) == 1)
            result = (result * base) % modulus;
        exponent /= 2;
        base = (base * base) % modulus;
    }
    return result;
}

// RSA密钥对类（仅支持加密部分）
struct RSAKeyPair
{
    BigInt e;             // 公钥指数（加密指数）
    BigInt d;             // 私钥指数（解密指数，可为空）
    BigInt m;             // 模数
    int chunkSize;        // 分块大小，计算方式： 2 * ((bitLength(m) + 15) / 16)
    int modulusHexLength; // 模数转换为16进制时应保留的位数，计算方式： (bitLength(m) + 3) / 4

    // 构造时传入的均为十六进制字符串（若 d 为空字符串则 d 不赋值）
    RSAKeyPair(const std::string &encryptionExponentHex,
               const std::string &decryptionExponentHex,
               const std::string &modulusHex)
        : e(hexToDec(encryptionExponentHex)),
          m(hexToDec(modulusHex)),
          chunkSize(0),       // 临时初始化，将在构造函数体内更新
          modulusHexLength(0) // 临时初始化，将在构造函数体内更新
    {
        // 将16进制字符串转换为 BigInt
        if (!decryptionExponentHex.empty())
        {
            d = hexToDec(decryptionExponentHex);
        }

        int bl = bitLength(m);
        chunkSize = 2 * ((bl + 15) / 16);
        modulusHexLength = (bl + 3) / 4;
    }
};

std::string encryptedString(const RSAKeyPair &key, const std::string &s)
{
    // 将字符串转换为字节数组（ASCII码）
    std::vector<int> bytes;
    bytes.reserve(s.size()); // 预分配空间，避免多次重新分配

    // 使用std::transform替代原始循环
    std::transform(s.begin(), s.end(), std::back_inserter(bytes), [](char c)
                   { return static_cast<unsigned char>(c); });

    // 填充0，直到字节数为 chunkSize 的整数倍
    while (bytes.size() % key.chunkSize != 0)
    {
        bytes.push_back(0);
    }

    std::vector<std::string> encryptedBlocks;
    // 每次处理 chunkSize 个字节（实际上每块每 2 字节构成一个 16 位整数）
    for (size_t i = 0; i < bytes.size(); i += key.chunkSize)
    {
        BigInt block = 0;
        int j = 0;
        // 每 2 个字节合并为一个 16 位整数
        for (size_t k = i; k < i + key.chunkSize && k < bytes.size(); k += 2)
        {
            int val = bytes[k];
            if (k + 1 < bytes.size())
            {
                val |= bytes[k + 1] << 8;
            }
            // 将该值左移 16*j 位后累加到 block 中
            block += BigInt(val) * pow(BigInt(2), 16 * j);
            j++;
        }
        // 进行 RSA 加密：加密块 = block^e mod m
        BigInt crypt = modPow(block, key.e, key.m);

        // 将加密结果转换为真正的十六进制格式
        std::string text = decToHex(crypt.to_string());

        // 确保每个加密块都有相同的长度（通过填充前导零）
        // const size_t expectedLength = 256; // 从您的预期输出计算得到
        // if (text.size() < expectedLength)
        // {
        //     text = "0" + text; // 只添加一个前导零
        // }
        if (text.size() % 2 == 1)
        {
            text = "0" + text; // 确保字节对齐
        }

        encryptedBlocks.push_back(text);
    }

    // 将各加密块用空格拼接成最终密文
    std::string result;
    // 预估结果字符串的长度，避免多次重新分配
    size_t resultSize = std::accumulate(encryptedBlocks.begin(), encryptedBlocks.end(), size_t(0), [](size_t sum, const std::string &block)
                                        {
                                            return sum + block.size() + 1; // +1 for space
                                        });
    // 如果有块存在，则最后一个块后不需要空格，可以减去1
    if (!encryptedBlocks.empty() && resultSize > 0)
    {
        resultSize--; // 最后一个块后不需要空格
    }
    result.reserve(resultSize);

    for (size_t i = 0; i < encryptedBlocks.size(); ++i)
    {
        result += encryptedBlocks[i];
        if (i != encryptedBlocks.size() - 1)
            result += " ";
    }
    return result;
}

QByteArray RSAUtils::encryptedPassword(QStringView password, QStringView mac)
{
    const std::string eHex = "10001"; // 公钥常用值 65537 的十六进制表示
    const std::string dHex = "";      // 私钥指数，这里不需要用到
    const std::string modulusHex =
        "94dd2a8675fb779e6b9f7103698634cd400f27a154afa67af6166a43fc2641"
        "7222a79506d34cacc7641946abda1785b7acf9910ad6a0978c91ec84d40b71"
        "d2891379af19ffb333e7517e390bd26ac312fe940c340466b4a5d4af1d65c3"
        "b5944078f96a1a51a5a53e4bc302818b7c9f63c4a1b07bd7d874cef1c3d4b2"
        "f5eb7871"; // 示例模数（实际应为大整数）

    RSAKeyPair key(eHex, dHex, modulusHex);

    std::string plainText = QString(password).append(QStringLiteral(">")).append(mac).toStdString();
    std::reverse(plainText.begin(), plainText.end());
    std::string cipherText = encryptedString(key, plainText);

    return QByteArray::fromStdString(cipherText);
}
