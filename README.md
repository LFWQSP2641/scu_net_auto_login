# scu_net_auto_login
四川大学校园网自动登录（江安）

有 Python 和 C++ 两种实现。

## 前言

四川大学江安校区的校园网使用锐捷ePortal Web 认证，使用不加密的 HTTP Post 方式进行登录。使用 RSA 零填充的方式加密密码。

## 注意

不建议将 `passwordEncrypt` 设置为 `false`，因为这样会将密码以明文的方式传输到校园网服务器上，可能会被网络攻击者窃取。

RSA 零填充的方式加密存在安全隐患。
