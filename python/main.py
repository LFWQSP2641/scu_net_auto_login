import Login
import argparse
import sys


def main():
    # 创建命令行参数解析器
    parser = argparse.ArgumentParser(description="四川大学校园网自动登录工具")

    # 添加命令行参数
    parser.add_argument("-u", "--username", required=True, help="校园网账号")
    parser.add_argument("-p", "--password", required=True, help="校园网密码")
    parser.add_argument(
        "-s",
        "--service",
        required=True,
        choices=["CHINATELECOM", "CHINAMOBILE", "CHINAUNICOM", "EDUNET"],
        help="网络服务类型",
    )

    # 解析命令行参数
    args = parser.parse_args()

    try:
        # 使用参数登录
        Login.login(args.username, args.password, args.service)
    except KeyboardInterrupt:
        print("\033[1;91m" + "[登录失败]" + "\033[0m" + " 用户终止程序")
        return 1
    except Exception as e:
        print("\033[1;91m" + "[登录失败]" + "\033[0m" + str(e))
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
