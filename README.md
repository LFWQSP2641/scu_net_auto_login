# SCU 校园网自动登录

四川大学校园网自动登录工具（ePortal Web）

支持江安校区和华西校区 Web 认证。

## 项目概述

本项目用于简化校园网认证流程。应用会根据已保存账号发起登录请求。

## 主要功能

- 多账号配置
- 支持多种服务出口：教育网、电信、移动、联通
- 自动连接 SCUNET 校园网
- 登录成功后可自动打开热点

## 使用方法

---

### 第三方图形客户端

特别感谢以下项目对本项目的支持，适配和拓展：

- [Alswek123/scu-net-auto-login-gui](https://github.com/Alswek123/scu-net-auto-login-gui)
  基于 Godot 引擎开发的跨平台图形界面，提供更友好的用户体验。

---

### C# 版本 (CLI)

1. 从 [Release页面](https://github.com/LFWQSP2641/scu_net_auto_login/releases) 下载合适的版本（Windows/Linux/MacOS）。

2. **配置方法**：
   - **配置文件方式**
   - **命令行方式**

#### 配置文件方式

配置文件位于 `config/config.json`，可以编辑该文件添加账号信息和设置登录选项。

```json
{
  "InitialDelayMs": 0,
  "RetryCount": 0,
  "RetryDelayMs": 0,
  "UserList": [
    {
      "remark": "Example Account",
      "username": "example_user",
      "password": "example_password",
      "service": "edunet"
    }
  ],
  "EnableHotspot": false,
  "ConnectSCUNETWifi": false
}
```

可使用命令行调用运行：

```bash
./Sal.CLI.exe
```

或直接双击 `Sal.CLI.exe` 运行。

#### 命令行方式

```bash
./Sal.CLI.exe login -u <username> -p <password> -s <service> [其他选项]
```

1. **参数说明**：
   - 必填参数：
     - `-u <username>`: 用户名
     - `-p <password>`: 密码
     - `-s <service>`: 服务类型（EDUNET、CHINATELECOM、CHINAMOBILE 或 CHINAUNICOM）
   - 可选参数：
     - `--retry-count <n>`: 重试次数，默认为0，-1表示无限重试
     - `--initial-delay <n>`: 初始延迟(秒)，默认为0
     - `--retry-delay <n>`: 重试延迟(秒)，默认为5
     - `--enable-hotspot`: 连接成功后自动开启热点
     - `--connectscunet-wifi`: 自动连接SCUNET WiFi

**使用示例**：

```bash
./Sal.CLI.exe login -u 123 -p 123 -s EDUNET --initial-delay 5 --enable-hotspot --connectscunet-wifi
```

   > 使用`connectscunet-wifi`选项时，建议设置`initial-delay`以避免首次登录失败

**查看帮助**：

```bash
./Sal.CLI.exe login -h
```

### C# 版本 (GUI)

可界面化配置账号信息和登录选项；同时支持手动连接校园网 WiFi ，开启热点以及手动登录等功能。

可以通过命令行参数 `cli` 启动 GUI 版本的 CLI 模式：

```bash
./Sal.GUI.exe cli
# 使用命令行方式登录
./Sal.GUI.exe cli login -u 123 -p 123 -s EDUNET --initial-delay 5 --enable-hotspot --connectscunet-wifi
```

## 技术栈

- C# (.NET 10)
- NativeAOT
- Avalonia
- ConsoleAppFramework
- CommunityToolkit.Mvvm

## 校园网技术说明

- 四川大学江安校区和华西部分校区校园网使用锐捷 ePortal Web 认证，通过HTTP Post方式登录，使用RSA零填充方式加密密码。

- 不建议将`passwordEncrypt`设置为`false`，这会导致密码以明文方式传输，可能被网络攻击者窃取。

- 校园网重写response策略特殊：通常访问 `http://192.168.2.135` 后会302重定向至 `http://123.123.123.123`，但川大校园网对小写请求头 `host:123.123.123.123` 会直接断开TCP连接，其只允许大写请求头。

## 其他语言实现

- [Qt C++](https://github.com/LFWQSP2641/scu_net_auto_login/tree/LegacyCpp)
- [Python](https://github.com/LFWQSP2641/scu_net_auto_login/tree/LegacyCpp)
- [Go](https://github.com/LFWQSP2641/scunet-auto-login)
- [Android Kotlin](https://github.com/LFWQSP2641/SCUNETlogin)
