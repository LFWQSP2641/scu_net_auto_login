#include "RSAUtils.h"

#include "mini-gmp.h"

#include <QString>
#include <numeric>

namespace
{
// 转换16进制字符串到mpz_t
void mpz_set_hex(mpz_t result, const QString &hexStr)
{
    mpz_init_set_str(result, hexStr.toLatin1().constData(), 16);
}

// 转换mpz_t到16进制字符串
QString mpz_get_hex(const mpz_t num)
{
    char *str = mpz_get_str(nullptr, 16, num);
    QString result = QString::fromLatin1(str);
    free(str);
    return result;
}

// 使用Mini-GMP进行RSA加密
QString encryptedString_gmp(const QString &eHex, const QString &modulusHex, const QString &plainText)
{
    // 初始化GMP变量
    mpz_t e, m, block, crypt;
    mpz_init_set_str(e, eHex.toLatin1().constData(), 16);
    mpz_init_set_str(m, modulusHex.toLatin1().constData(), 16);
    mpz_init(block);
    mpz_init(crypt);

    // 计算分块大小 - 模仿原始代码的处理方式
    unsigned long bits = mpz_sizeinbase(m, 2);
    int chunkSize = 2 * ((bits + 15) / 16);

    // 准备字节数组
    QVector<int> bytes;
    bytes.reserve(plainText.size());
    for (int i = 0; i < plainText.size(); ++i)
    {
        bytes.append(static_cast<unsigned char>(plainText.at(i).toLatin1()));
    }

    // 填充0到chunkSize整数倍
    while (bytes.size() % chunkSize != 0)
    {
        bytes.append(0);
    }

    // 分块加密
    QStringList encryptedBlocks;
    for (int i = 0; i < bytes.size(); i += chunkSize)
    {
        // 重置block为0
        mpz_set_ui(block, 0);

        // 构建当前块
        int j = 0;
        for (int k = i; k < i + chunkSize && k < bytes.size(); k += 2)
        {
            int val = bytes[k];
            if (k + 1 < bytes.size())
            {
                val |= bytes[k + 1] << 8;
            }

            // 将val左移16*j位后添加到block
            mpz_t temp, shift; // 修改了变量名，避免与外部block变量冲突
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
        QString text = mpz_get_hex(crypt);

        // 确保字节对齐
        if (text.size() % 2 == 1)
        {
            text = "0" + text;
        }

        encryptedBlocks.append(text);
    }

    // 使用std::accumulate拼接结果
    QString result = std::accumulate(encryptedBlocks.begin(), encryptedBlocks.end(), QString(), [](const QString &accum, const QString &currBlock)
    {
        return accum.isEmpty() ? currBlock : accum + " " + currBlock;
    });

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
    static const QString eHex = "10001"; // 公钥常用值 65537 的十六进制表示
    static const QString modulusHex =
        "94dd2a8675fb779e6b9f7103698634cd400f27a154afa67af6166a43fc2641"
        "7222a79506d34cacc7641946abda1785b7acf9910ad6a0978c91ec84d40b71"
        "d2891379af19ffb333e7517e390bd26ac312fe940c340466b4a5d4af1d65c3"
        "b5944078f96a1a51a5a53e4bc302818b7c9f63c4a1b07bd7d874cef1c3d4b2"
        "f5eb7871";

    QString plainText = QString(password).append(QStringLiteral(">")).append(mac);

    // 反转字符串
    QString reversed;
    reversed.resize(plainText.size());
    for (int i = 0; i < plainText.size(); ++i)
    {
        reversed[plainText.size() - 1 - i] = plainText[i];
    }

    // 使用Mini-GMP版本的加密函数
    QString cipherText = encryptedString_gmp(eHex, modulusHex, reversed);

    return cipherText.toLatin1();
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
