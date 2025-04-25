#include "RSAUtils.h"

#include "mini-gmp.h"

#include <string>

namespace
{
// 转换16进制字符串到mpz_t
void mpz_set_hex(mpz_t result, const std::string &hexStr)
{
    mpz_init_set_str(result, hexStr.c_str(), 16);
}

// 转换mpz_t到16进制字符串
std::string mpz_get_hex(const mpz_t num)
{
    char *str = mpz_get_str(nullptr, 16, num);
    std::string result(str);
    free(str);
    return result;
}

// 使用Mini-GMP进行RSA加密
std::string encryptedString_gmp(const std::string &eHex, const std::string &modulusHex, const std::string &plainText)
{
    // 初始化GMP变量
    mpz_t e, m, block, crypt;
    mpz_init_set_str(e, eHex.c_str(), 16);
    mpz_init_set_str(m, modulusHex.c_str(), 16);
    mpz_init(block);
    mpz_init(crypt);

    // 计算分块大小 - 模仿原始代码的处理方式
    unsigned long bits = mpz_sizeinbase(m, 2);
    int chunkSize = 2 * ((bits + 15) / 16);

    // 准备字节数组
    std::vector<int> bytes;
    bytes.reserve(plainText.size());
    std::transform(plainText.begin(), plainText.end(), std::back_inserter(bytes), [](char c)
    {
        return static_cast<unsigned char>(c);
    });

    // 填充0到chunkSize整数倍
    while (bytes.size() % chunkSize != 0)
    {
        bytes.push_back(0);
    }

    // 分块加密
    std::vector<std::string> encryptedBlocks;
    for (size_t i = 0; i < bytes.size(); i += chunkSize)
    {
        // 重置block为0
        mpz_set_ui(block, 0);

        // 构建当前块
        int j = 0;
        for (size_t k = i; k < i + chunkSize && k < bytes.size(); k += 2)
        {
            int val = bytes[k];
            if (k + 1 < bytes.size())
            {
                val |= bytes[k + 1] << 8;
            }

            // 将val左移16*j位后添加到block
            mpz_t temp, shift;
            mpz_init_set_ui(temp, val);
            mpz_init_set_ui(shift, 1);
            mpz_mul_2exp(shift, shift, 16 * j); // shift = 2^(16*j)
            mpz_mul(temp, temp, shift);         // temp = val * 2^(16*j)
            mpz_add(block, block, temp);        // block += temp
            mpz_clear(temp);
            mpz_clear(shift);
            j++;
        }

        // 执行模幂运算: crypt = block^e mod m
        mpz_powm(crypt, block, e, m);

        // 转换为十六进制
        std::string text = mpz_get_hex(crypt);

        // 确保字节对齐
        if (text.size() % 2 == 1)
        {
            text = "0" + text;
        }

        encryptedBlocks.push_back(text);
    }

    // 拼接结果
    std::string result;
    size_t resultSize = std::accumulate(encryptedBlocks.begin(), encryptedBlocks.end(), size_t(0), [](size_t sum, const std::string &block)
    {
        return sum + block.size() + 1;
    });

    if (!encryptedBlocks.empty() && resultSize > 0)
    {
        resultSize--;
    }
    result.reserve(resultSize);

    for (size_t i = 0; i < encryptedBlocks.size(); ++i)
    {
        result += encryptedBlocks[i];
        if (i != encryptedBlocks.size() - 1)
            result += " ";
    }

    // 清理GMP变量
    mpz_clear(e);
    mpz_clear(m);
    mpz_clear(block);
    mpz_clear(crypt);

    return result;
}
} // namespace

RSAUtils::RSAUtils(QObject *parent)
    : QThread {parent}
{
}

QByteArray RSAUtils::encryptedPassword(QStringView password, QStringView mac)
{
    static const std::string eHex = "10001"; // 公钥常用值 65537 的十六进制表示
    static const std::string modulusHex =
        "94dd2a8675fb779e6b9f7103698634cd400f27a154afa67af6166a43fc2641"
        "7222a79506d34cacc7641946abda1785b7acf9910ad6a0978c91ec84d40b71"
        "d2891379af19ffb333e7517e390bd26ac312fe940c340466b4a5d4af1d65c3"
        "b5944078f96a1a51a5a53e4bc302818b7c9f63c4a1b07bd7d874cef1c3d4b2"
        "f5eb7871";

    std::string plainText = QString(password).append(QStringLiteral(">")).append(mac).toStdString();
    std::reverse(plainText.begin(), plainText.end());

    // 使用Mini-GMP版本的加密函数
    std::string cipherText = encryptedString_gmp(eHex, modulusHex, plainText);

    return QByteArray::fromStdString(cipherText);
}

void RSAUtils::syncEncryptedPassword(const QString &password, const QString &mac)
{
    m_password = password;
    m_mac = mac;
    start();
}

void RSAUtils::run()
{
    m_encryptedPassword = encryptedPassword(m_password, m_mac);
    emit encryptedPasswordReady(m_encryptedPassword);
}
