# scu_net_auto_login
四川大学校园网自动登录（江安）

有 Python 和 C++ (Qt) 两种实现。

## 注意

四川大学江安校区的校园网使用锐捷ePortal Web 认证，使用不加密的 HTTP Post 方式进行登录。使用 RSA 零填充的方式加密密码。

不建议将 `passwordEncrypt` 设置为 `false`，因为这样会将密码以明文的方式传输到校园网服务器上，可能会被网络攻击者窃取。

RSA 零填充的方式加密存在安全隐患。

校园网重写 response 的策略非常规。通常情况下，访问 `http://192.168.2.135` 后，会 302 重定向到 `http://123.123.123.123`。客户端访问 `http://123.123.123.123` 后，response 会被重写为 `<script>top.self.location.href='http://192.168.2.135/eportal/index.jsp?`，从而重定向到认证界面。

然而，根据 HTTP 协议规范（RFC 7230），请求头的键名是大小写不敏感的，即 "Host" 和 "host" 应该被视为相同。但是，川大校园网对请求头 `host:123.123.123.123` 会直接断开 TCP 连接，返回`[FIN, ACK]`。

且 Qt 的 QNetworkAccessManager 在发送请求时，会将请求头的键名转换为小写字母，这导致了 Qt 版本无法正常工作。

因此，C++ 版本使用了 QTcpSocket 进行 TCP 连接，手动构造 HTTP 请求。

## TODO

- 共通
- [ ] 调用系统 api 连接 SCUNET wifi
- [ ] 连接成功后，自动开启移动热点
- [ ] 程序传参可加密
- [ ] 配置文件加密
- [ ] 使用基于 [绕过校园网认证](https://lfwqsp2641.me/bypass-campus-network.html) 的方式，访问川大微服务，将已登录的设备踢出。

- C++ 版本
  - [ ] 使用 Qt 进行 UI 开发（不清楚是否真的有必要，如果有必要的话，请在 discuss 中告诉我）

- Python 版本

## 特别感谢

[faheel/BigInt](https://github.com/faheel/BigInt) 提供了大数运算的实现，感谢作者的辛勤付出。
