using ServiceLib.Resx;

namespace ServiceLib.Helper;

using System.Globalization;
using System.Numerics;

public class LegacyCampusRsaEncryptor
{
    private const string DefaultPublicExponentHex = "10001";
    private const string DefaultModulusHex = "94dd2a8675fb779e6b9f7103698634cd400f27a154afa67af6166a43fc26417222a79506d34cacc7641946abda1785b7acf9910ad6a0978c91ec84d40b71d2891379af19ffb333e7517e390bd26ac312fe940c340466b4a5d4af1d65c3b5944078f96a1a51a5a53e4bc302818b7c9f63c4a1b07bd7d874cef1c3d4b2f5eb7871";

    public static string EncryptPassword(string password, string mac)
    {
        ArgumentNullException.ThrowIfNull(password);
        ArgumentNullException.ThrowIfNull(mac);

        var plainText = string.Concat(password, ">", mac);
        var reversed = new string(plainText.Reverse().ToArray());

        return EncryptString(reversed, DefaultPublicExponentHex, DefaultModulusHex);
    }

    public static string EncryptPassword(string password, string mac, string encryptionExponentHex, string modulusHex)
    {
        ArgumentNullException.ThrowIfNull(password);
        ArgumentNullException.ThrowIfNull(mac);

        var plainText = string.Concat(password, ">", mac);
        var reversed = new string(plainText.Reverse().ToArray());

        return EncryptString(reversed, encryptionExponentHex, modulusHex);
    }

    public static string EncryptString(string plainText)
    {
        ArgumentNullException.ThrowIfNull(plainText);

        return EncryptString(plainText, DefaultPublicExponentHex, DefaultModulusHex);
    }

    public static string EncryptString(string plainText, string encryptionExponentHex, string modulusHex)
    {
        ArgumentNullException.ThrowIfNull(plainText);
        ArgumentException.ThrowIfNullOrWhiteSpace(encryptionExponentHex);
        ArgumentException.ThrowIfNullOrWhiteSpace(modulusHex);

        var exponent = ParseHex(encryptionExponentHex);
        var modulus = ParseHex(modulusHex);

        if (modulus <= 0)
        {
            throw new ArgumentOutOfRangeException(nameof(modulusHex), ResStr.ErrModulusNotPositiveHex);
        }

        var bitLength = GetBitLength(modulus);
        var chunkSize = 2 * ((bitLength + 15) / 16);
        var modulusHexLength = (bitLength + 3) / 4;

        var input = plainText.Select(character => (int)character).ToList();

        if (input.Count == 0)
        {
            return string.Empty;
        }

        while (input.Count % chunkSize != 0)
        {
            input.Add(0);
        }

        var result = new List<string>(input.Count / chunkSize);

        for (var index = 0; index < input.Count; index += chunkSize)
        {
            var block = BigInteger.Zero;
            var wordIndex = 0;

            for (var offset = index; offset < index + chunkSize && offset < input.Count; offset += 2)
            {
                var value = input[offset];

                if (offset + 1 < input.Count)
                {
                    value |= input[offset + 1] << 8;
                }

                block |= (BigInteger)value << (16 * wordIndex);
                wordIndex++;
            }

            var encrypted = BigInteger.ModPow(block, exponent, modulus);
            var hex = encrypted.ToString("x", CultureInfo.InvariantCulture).PadLeft(modulusHexLength, '0');
            result.Add(hex);
        }

        return string.Join(" ", result);
    }

    private static BigInteger ParseHex(string value)
    {
        var hex = value.Trim();

        if (hex.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
        {
            hex = hex[2..];
        }

        if (hex.Length == 0)
        {
            return BigInteger.Zero;
        }

        if (hex.Length % 2 != 0)
        {
            hex = "0" + hex;
        }

        var bytes = new byte[(hex.Length / 2) + 1];

        for (var sourceIndex = hex.Length; sourceIndex > 0; sourceIndex -= 2)
        {
            var targetIndex = (hex.Length - sourceIndex) / 2;
            bytes[targetIndex] = byte.Parse(hex.AsSpan(sourceIndex - 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
        }

        return new BigInteger(bytes);
    }

    private static int GetBitLength(BigInteger value)
    {
        if (value <= 0)
        {
            return 0;
        }

        var bytes = value.ToByteArray(isUnsigned: true, isBigEndian: true);
        var bits = (bytes.Length - 1) * 8;
        var mostSignificantByte = bytes[0];

        while (mostSignificantByte > 0)
        {
            bits++;
            mostSignificantByte >>= 1;
        }

        return bits;
    }
}