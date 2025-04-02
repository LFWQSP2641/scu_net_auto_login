# scu_net_auto_login

四川大学校园网自动登录（江安）

有 Python 和 C++ (Qt) 两种实现。

C++ (Qt) 功能更多。

Python 暂不接受 feature request, PR welcome。

## 注意

- 四川大学江安校区的校园网使用锐捷ePortal Web 认证，使用不加密的 HTTP Post 方式进行登录。使用 RSA 零填充的方式加密密码。

- 不建议将 `passwordEncrypt` 设置为 `false`，因为这样会将密码以明文的方式传输到校园网服务器上，可能会被网络攻击者窃取。

- RSA 零填充的方式加密存在安全隐患。

- 校园网重写 response 的策略非常规。通常情况下，访问 `http://192.168.2.135` 后，会 302 重定向到 `http://123.123.123.123`。客户端访问 `http://123.123.123.123` 后，response 会被重写为 `<script>top.self.location.href='http://192.168.2.135/eportal/index.jsp?`，从而重定向到认证界面。
然而，根据 HTTP 协议规范（RFC 7230），请求头的键名是大小写不敏感的，即 "Host" 和 "host" 应该被视为相同。但是，川大校园网对请求头 `host:123.123.123.123` 会直接断开 TCP 连接，返回`[FIN, ACK]`。
且 Qt 的 QNetworkAccessManager 在发送请求时，会将请求头的键名转换为小写字母，这导致了 QNetworkAccessManager 无法正常工作。
因此，C++ 版本使用了 QTcpSocket 进行 TCP 连接，手动构造 HTTP 请求。

## 使用方法

C++ 版本：
在 [Release](https://github.com/LFWQSP2641/scu_net_auto_login/releases) 中下载 `scu_net_auto_login.exe`。
在命令行中运行 `scu_net_auto_login.exe -u <username> -p <password> -s <service> [--hotspot] [--connect]`，其中：

- `<username>` 和 `<password>` 分别为用户名和密码（必填）
- `<service>` 为服务类型（如 `EDUNET` 、`CHINATELECOM`、`CHINAMOBILE` 或 `CHINAUNICOM`）（必填）
- `retry-count` 重试次数，默认为 0 次，-1 表示无限重试
- `initial-delay` 初始延迟，单位为秒，默认为 0 秒
- `retry-delay` 重试延迟，单位为秒，默认为 5 秒
- `hotspot` 可在连接后自动开启热点。
- `connect` 可在连接前自动连接 SCUNET wifi。

运行 `scu_net_auto_login.exe -h` 查看帮助信息。

设置 `connect` 建议设置 `initial-delay` 否则第一次登录会失败。

示例：

```bash
scu_net_auto_login -u 123 -p 123 -s EDUNET --initial-delay 5 --hotspot --connect
```

Python 版本：
下载源码，在 `python` 目录下运行 `python main.py -u <username> -p <password> -s <service>`，其中 `<username>` 和 `<password>` 分别为用户名和密码，`<service>` 为服务类型（如 `EDUNET` 、`CHINATELECOM`、`CHINAMOBILE` 或 `CHINAUNICOM`）。
在 `python` 目录下运行 `python main.py -h` 查看帮助信息。

## Feature

- 自动开启移动热点
  - 适配 Windows 10 2004 及以上版本
  - macOS 未测试
  - Linux 未实现，PR wellcome
- 调用系统连接 SCUNET wifi
  - 仅适配 Windows，且之前需连接过 SCUNET wifi
  - macOS 未测试
  - Linux 未测试

## TODO

- 共通
- [x] 调用系统连接 SCUNET wifi
- [x] 连接成功后，自动开启移动热点
- [ ] 使用基于 [绕过校园网认证](https://lfwqsp2641.me/bypass-campus-network.html) 的方式，访问川大微服务，将已登录的设备踢出。

---

- ~~程序可传参已加密密码~~（mac 与密码加密强绑定）
- ~~配置文件加密~~（没必要）

---

- C++ 版本
  - [ ] 使用 Qt 进行 UI 开发（不清楚是否真的有必要，如果有必要的话，请在 discuss 中[投票](https://github.com/LFWQSP2641/scu_net_auto_login/discussions/1)）

- Python 版本

## 特别感谢

[faheel/BigInt](https://github.com/faheel/BigInt) 提供了大数运算的实现，感谢作者的辛勤付出。
