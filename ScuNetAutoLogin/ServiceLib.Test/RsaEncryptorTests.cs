using AwesomeAssertions;
using ServiceLib.Helper;

namespace ServiceLib.Test;

public class RsaEncryptorTests
{
    [Fact]
    public void TestEncrypt()
    {
        var expectedResult = "0f8c069affb97231" +
                             "446dce90164fd7d5" +
                             "32b6e1a9fbe1d57e" +
                             "3d9f0794cc66b38e" +
                             "7161ba9985412021" +
                             "89626c132989e3ba" +
                             "ab7f27f361ad5b4a" +
                             "ca9589d42af9046a" +
                             "31a1ced0f46687f3" +
                             "7175917d6563962a" +
                             "95937b283b725ab2" +
                             "18d90b43777db40b" +
                             "0325baa07447a21b" +
                             "aa892d9606324198" +
                             "5df2af279928d742" +
                             "ea29dd6f0174e12f";

        var password = "password9977";
        var mac = "eef900330a8987f0957c14c756513384";
        var encrypted = LegacyCampusRsaEncryptor.EncryptPassword(password, mac);

        encrypted.Should().Be(expectedResult);
    }
}
