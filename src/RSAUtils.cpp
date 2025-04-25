#include "RSAUtils.h"

#include "mini-gmp.h"

#include <QByteArray>
#include <QString>
#include <algorithm>

namespace
{

// 使用Mini-GMP进行RSA加密 - 极致优化版本
QString encryptedString_gmp(const QString &eHex, const QString &modulusHex, const QString &plainText)
{
    // 初始化GMP变量 - 一次性完成
    mpz_t e, m, block, crypt, temp, shift;
    mpz_init(e);
    mpz_init(m);
    mpz_init(block);
    mpz_init(crypt);
    mpz_init(temp);
    mpz_init(shift);

    // 直接使用QByteArray避免重复转换
    QByteArray eHexArr = eHex.toLatin1();
    QByteArray mHexArr = modulusHex.toLatin1();

    // 直接操作字节数组
    mpz_set_str(e, eHexArr.constData(), 16);
    mpz_set_str(m, mHexArr.constData(), 16);

    // 计算分块大小
    unsigned long bits = mpz_sizeinbase(m, 2);
    int chunkSize = 2 * ((bits + 15) / 16);

    // 准备输入字节数组
    QByteArray bytes = plainText.toLatin1();

    // 计算并添加填充
    int paddingNeeded = (chunkSize - (bytes.size() % chunkSize)) % chunkSize;
    if (paddingNeeded > 0)
    {
        bytes.resize(bytes.size() + paddingNeeded, '\0');
    }

    // 预分配足够大的结果缓冲区
    int maxHexLen = mpz_sizeinbase(m, 16) + 1;
    int estimatedResultSize = (bytes.size() / chunkSize) * (maxHexLen + 1);
    QByteArray resultBytes;
    resultBytes.reserve(estimatedResultSize);

    // 直接使用字节指针加速访问
    const char *data = bytes.constData();

    // 分块加密
    for (int i = 0; i < bytes.size(); i += chunkSize)
    {
        // 重置block为0
        mpz_set_ui(block, 0);

        // 构建当前块 - 直接处理字节
        int j = 0;
        for (int k = 0; k < chunkSize; k += 2)
        {
            int idx = i + k;
            if (idx >= bytes.size()) break;

            // 合并两个字节为一个16位整数
            int val = static_cast<unsigned char>(data[idx]);
            if (idx + 1 < bytes.size())
            {
                val |= static_cast<unsigned char>(data[idx + 1]) << 8;
            }

            // 构建大整数块
            mpz_set_ui(temp, val);
            mpz_set_ui(shift, 1);
            mpz_mul_2exp(shift, shift, 16 * j);
            mpz_mul(temp, temp, shift);
            mpz_add(block, block, temp);
            j++;
        }

        // 执行模幂运算
        mpz_powm(crypt, block, e, m);

        // 转换为十六进制并添加到结果
        size_t bufSize = mpz_sizeinbase(crypt, 16) + 2;
        QByteArray hexBuffer(bufSize, '\0');
        mpz_get_str(hexBuffer.data(), 16, crypt);

        // 创建不包含多余空字符的新QByteArray
        hexBuffer = QByteArray(hexBuffer.constData()); // 自动到第一个\0截断
        hexBuffer = hexBuffer.trimmed();

        if (hexBuffer.size() % 2 == 1)
        {
            hexBuffer.prepend('0');
        }
        resultBytes.append(hexBuffer);

        // 添加空格分隔符（除了最后一个块）
        if (i + chunkSize < bytes.size())
        {
            resultBytes.append(' ');
        }
    }

    // 清理GMP变量
    mpz_clear(e);
    mpz_clear(m);
    mpz_clear(block);
    mpz_clear(crypt);
    mpz_clear(temp);
    mpz_clear(shift);

    // 一次性转换为QString
    return QString::fromLatin1(resultBytes);
}

} // namespace

RSAUtils::RSAUtils(QObject *parent)
    : QThread {parent}
{
}

QByteArray RSAUtils::encryptedPassword(QStringView password, QStringView mac)
{
    static const QString eHex = "10001";
    static const QString modulusHex =
        "94dd2a8675fb779e6b9f7103698634cd400f27a154afa67af6166a43fc2641"
        "7222a79506d34cacc7641946abda1785b7acf9910ad6a0978c91ec84d40b71"
        "d2891379af19ffb333e7517e390bd26ac312fe940c340466b4a5d4af1d65c3"
        "b5944078f96a1a51a5a53e4bc302818b7c9f63c4a1b07bd7d874cef1c3d4b2"
        "f5eb7871";

    // 预分配并高效构建输入字符串
    QString plainText;
    plainText.reserve(password.length() + mac.length() + 1);
    plainText.append(password);
    plainText.append('>');
    plainText.append(mac);

    // 使用std::reverse优化字符串反转
    QByteArray bytes = plainText.toLatin1();
    std::reverse(bytes.begin(), bytes.end());
    QString reversed = QString::fromLatin1(bytes);

    // 使用优化的加密函数
    return encryptedString_gmp(eHex, modulusHex, reversed).toLatin1();
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
    emit encryptedPasswordFinished(m_encryptedPassword);
}
