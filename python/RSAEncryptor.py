"""
RSAUtils Python 实现
基于 JavaScript 版本的 RSA 加密库转换而来

原 JavaScript 版权:
Copyright 1998-2005 David Shapiro
Modified by Fuchun, 2010-05-06

注意，此实现为 RSA 零填充，存在安全隐患
"""


class BigInt:
    """大整数类，对应JavaScript中的BigInt"""

    def __init__(self, init_value=None):
        if init_value is None:
            self.digits = []
            self.isNeg = False
        elif isinstance(init_value, int):
            self.fromInt(init_value)
        elif isinstance(init_value, str):
            if init_value.startswith("0x"):
                self.fromHex(init_value[2:])
            else:
                self.fromString(init_value, 10)

    def fromInt(self, value):
        """从整数创建BigInt"""
        self.isNeg = value < 0
        value = abs(value)
        self.digits = []

        while value > 0:
            self.digits.append(value & 0xFFFF)
            value >>= 16

        if not self.digits:
            self.digits = [0]

    def fromHex(self, hex_str):
        """从十六进制字符串创建BigInt"""
        self.digits = []
        self.isNeg = False

        i = len(hex_str)
        j = 0

        while i > 0:
            start = max(i - 4, 0)
            chunk = hex_str[start:i]
            self.digits.append(int(chunk, 16))
            i -= 4

    def toHex(self):
        """转换为十六进制字符串"""
        result = ""
        for digit in reversed(self.digits):
            result += f"{digit:04x}"

        # 去掉前导零
        result = result.lstrip("0")
        if not result:
            result = "0"

        return result

    def __str__(self):
        return ("- " if self.isNeg else "") + " ".join(str(d) for d in self.digits)


class RSAUtils:
    """RSA工具类，包含RSA加密所需的各种函数"""

    @staticmethod
    def biToHex(x):
        """BigInt转十六进制字符串"""
        if isinstance(x, BigInt):
            return x.toHex()
        return hex(x)[2:]

    @staticmethod
    def biFromHex(s):
        """十六进制字符串转BigInt"""
        if isinstance(s, str):
            result = BigInt()
            result.fromHex(s)
            return result
        return s

    @staticmethod
    def modPow(base, exponent, modulus):
        """模幂运算，等同于(base^exponent) % modulus"""
        return pow(base, exponent, modulus)

    @staticmethod
    def setMaxDigits(n):
        """设置最大位数，Python原生支持大整数，此函数仅保留兼容性"""
        pass


class BarrettMu:
    """Barrett约简算法实现"""

    def __init__(self, modulus):
        self.modulus = modulus
        self.k = modulus.bit_length() // 16 + 1
        r = 1 << (self.k * 16 * 2)
        self.mu = r // modulus
        self.r2 = 1 << (16 * (self.k + 1))

    def reduce(self, x):
        """Barrett约简"""
        q1 = x >> (16 * (self.k - 1))
        q2 = q1 * self.mu
        q3 = q2 >> (16 * (self.k + 1))
        r1 = x & (self.r2 - 1)
        r2 = (q3 * self.modulus) & (self.r2 - 1)
        r = r1 - r2

        if r < 0:
            r += self.r2

        while r >= self.modulus:
            r -= self.modulus

        return r

    def powMod(self, x, y):
        """模幂运算"""
        return pow(x, y, self.modulus)


class RSAKeyPair:
    """RSA密钥对类"""

    def __init__(self, encryptionExponent, decryptionExponent, modulus):
        self.e = int(encryptionExponent, 16)
        self.d = int(decryptionExponent, 16) if decryptionExponent else None
        self.m = int(modulus, 16)
        self.chunkSize = 2 * ((self.m.bit_length() + 15) // 16)
        self.radix = 16
        self.barrett = BarrettMu(self.m)


def getKeyPair(encryptionExponent, decryptionExponent, modulus):
    """创建RSA密钥对"""
    return RSAKeyPair(encryptionExponent, decryptionExponent, modulus)


def encryptedString(key, s):
    """
    使用RSA公钥加密字符串
    :param key: RSA密钥对
    :param s: 待加密字符串
    :return: 加密后的十六进制字符串
    """
    # 将字符串转换为字节数组
    a = [ord(c) for c in s]

    # 填充0直到长度为chunkSize的整数倍
    while len(a) % key.chunkSize != 0:
        a.append(0)

    result = []

    # 按chunkSize大小分块加密
    for i in range(0, len(a), key.chunkSize):
        block = 0
        j = 0

        for k in range(i, min(i + key.chunkSize, len(a)), 2):
            val = a[k]
            if k + 1 < len(a):
                val |= a[k + 1] << 8
            block |= val << (16 * j)
            j += 1

        # 使用Barrett约简进行模幂运算
        crypt = key.barrett.powMod(block, key.e)

        # 转换为十六进制，确保保留前导零
        # 计算模数的十六进制长度，确保每个加密块有相同长度
        modulus_hex_length = (key.m.bit_length() + 3) // 4
        text = hex(crypt)[2:].zfill(modulus_hex_length)
        result.append(text)

    return " ".join(result)


def decryptedString(key, s):
    """
    使用RSA私钥解密字符串
    :param key: RSA密钥对
    :param s: 加密的十六进制字符串
    :return: 解密后的字符串
    """
    if not key.d:
        raise ValueError("解密需要私钥(d)")

    blocks = s.split(" ")
    result = ""

    for block in blocks:
        bi = int(block, 16)
        decrypted = key.barrett.powMod(bi, key.d)

        # 每个块包含多个2字节字符
        while decrypted > 0:
            char1 = decrypted & 0xFF
            decrypted >>= 8
            char2 = decrypted & 0xFF
            decrypted >>= 8

            result += chr(char1)
            if char2 != 0:  # 忽略尾部的空字节
                result += chr(char2)

    # 移除尾部的空字符
    return result.rstrip("\0")


# 设置最大位数为130，以支持1024位RSA密钥
RSAUtils.setMaxDigits(130)


def encryptedPassword(password, mac):
    """
    加密密码
    :param password: 待加密的密码
    :return: 加密后的十六进制字符串
    """
    # 反转密码
    password_reversed = (password + ">" + mac)[::-1]

    publicKeyExponent = "10001"  # 十六进制，等于65537
    modulus = "94dd2a8675fb779e6b9f7103698634cd400f27a154afa67af6166a43fc26417222a79506d34cacc7641946abda1785b7acf9910ad6a0978c91ec84d40b71d2891379af19ffb333e7517e390bd26ac312fe940c340466b4a5d4af1d65c3b5944078f96a1a51a5a53e4bc302818b7c9f63c4a1b07bd7d874cef1c3d4b2f5eb7871"
    key = getKeyPair(publicKeyExponent, "", modulus)

    # 加密
    encrypted = encryptedString(key, password_reversed)
    return encrypted
