# SCU 校园网自动登录

四川大学校园网自动登录工具（江安）

## 项目概述

本项目提供两种实现方式：

- **C++ (Qt)**: 功能更全面，支持自动连接WiFi、开启热点等高级功能
- **Python**: 仅提供基础登录功能

> Python 版本暂不接受新功能请求，但欢迎提交 PR。

## 使用方法

### C++ 版本

1. 从 [Release页面](https://github.com/LFWQSP2641/scu_net_auto_login/releases) 下载 `scu_net_auto_login_windows_win64_msvc2022_64.7z`

2. **配置方法**：
   - **推荐方式**：直接在配置文件 `Config/setting.json` 中设置参数（v1.1.0版本后支持）
   - **命令行方式**：

   ```bash
   scu_net_auto_login.exe -u <username> -p <password> -s <service> [选项]
   ```

3. **参数说明**：
   - 必填参数：
     - `-u <username>`: 用户名
     - `-p <password>`: 密码
     - `-s <service>`: 服务类型（EDUNET、CHINATELECOM、CHINAMOBILE 或 CHINAUNICOM）
   - 可选参数：
     - `--retry-count <n>`: 重试次数，默认为0，-1表示无限重试
     - `--initial-delay <n>`: 初始延迟(秒)，默认为0
     - `--retry-delay <n>`: 重试延迟(秒)，默认为5
     - `--hotspot`: 连接成功后自动开启热点
     - `--connect`: 自动连接SCUNET WiFi
     - `--use-config <true|false>`: 是否使用配置文件，默认为true

4. **使用示例**：

   ```bash
   scu_net_auto_login -u 123 -p 123 -s EDUNET --initial-delay 5 --hotspot --connect --use-config true
   ```

   > 使用`connect`选项时，建议设置`initial-delay`以避免首次登录失败

5. **查看帮助**：

   ```bash
   scu_net_auto_login.exe -h
   ```

### Python 版本

1. 下载源码
2. 在`python`目录下运行：

   ```bash
   python main.py -u <username> -p <password> -s <service>
   ```

3. 查看帮助：

   ```bash
   python main.py -h
   ```

## 高级功能

### 踢出已登录设备（基于绕过校园网认证）

> 注意：此功能不适合新手用户，不使用此功能可忽略此部分

1. 参考[绕过校园网认证](https://lfwqsp2641.me/bypass-campus-network.html)搭建好服务端
2. 在配置文件`Config/setting.json`中设置：
   - `enableAutoTick`: 是否启用自动踢出(bool)，默认`false`
   - 微服务Cookie配置：
     - `cookieEaiSess`
     - `cookieUUkey`
   - 绕过认证配置：
     - `bypassCampusNetworkCore`: 核心路径，如`Software/hysteria2/hysteria-windows-amd64.exe`
     - `bypassCampusNetworkCoreCommand`: 命令参数，如`"-c", "Software/hysteria2/config.yaml"`
     - `bypassCampusNetworkCorePort`: 端口，如`8080`
     - `bypassCampusNetworkSocks5Proxy`: 是否为Socks5代理(bool)，`false`表示Http代理，默认`true`

## 功能支持

- **自动开启移动热点**
  - ✅ Windows 10 2004及以上版本
  - ⚠️ macOS未测试
  - ❌ Linux未实现(欢迎PR)

- **自动连接SCUNET WiFi**
  - ✅ Windows (需之前已连接过SCUNET)
  - ⚠️ macOS未测试
  - ⚠️ Linux未测试

- **踢出已登录设备**
  - ✅ Windows
  - ✅ macOS
  - ✅ Linux

- **多用户支持**
  - ✅ Windows
  - ✅ macOS
  - ✅ Linux

## 技术说明

- 四川大学江安校区校园网使用锐捷ePortal Web认证，通过HTTP Post方式登录，使用RSA零填充方式加密密码。

- 不建议将`passwordEncrypt`设置为`false`，这会导致密码以明文方式传输，可能被网络攻击者窃取。

- 校园网重写response策略特殊：通常访问`http://192.168.2.135`后会302重定向至`http://123.123.123.123`，但川大校园网对请求头`host:123.123.123.123`会直接断开TCP连接。由于Qt的QNetworkAccessManager会将请求头键名转为小写，导致无法正常工作，因此C++版本使用QTcpSocket手动构造HTTP请求。

## 待办事项

- [x] 调用系统连接SCUNET WiFi
- [x] 连接成功后自动开启移动热点
- [x] 使用绕过校园网认证方式访问川大微服务
- [ ] C++版本UI开发（[投票讨论](https://github.com/LFWQSP2641/scu_net_auto_login/discussions/1)）

## License

This project is licensed under the MIT License, except for the Mini-GMP library, which is licensed under the GNU Lesser General Public License (LGPL) v3. See thirdparty/mini-gmp/COPYING.LESSERv3 for details.

The Mini-GMP source code is included in thirdparty/mini-gmp/ and is available from https://gmplib.org/.
