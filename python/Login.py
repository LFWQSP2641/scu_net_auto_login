import requests
import re
from typing import Optional, Union
import RSAEncryptor

session = requests.Session()
main_url = "http://192.168.2.135/"

http_header = {
    "Accept": "*/*",
    "Accept-Encoding": "gzip, deflate",
    "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/114.0.0.0 Safari/537.36",
}

# 服务类型映射表
SERVICE_CODES = {
    "CHINATELECOM": "%E7%94%B5%E4%BF%A1%E5%87%BA%E5%8F%A3",  # 电信出口
    "CHINAMOBILE": "%E7%A7%BB%E5%8A%A8%E5%87%BA%E5%8F%A3",  # 移动出口
    "CHINAUNICOM": "%E8%81%94%E9%80%9A%E5%87%BA%E5%8F%A3",  # 联通出口
    "EDUNET": "internet",
}

retry_time = 1
proxies = {"http": None, "https": None}


def getQueryString() -> str:
    """获取登录所需的查询字符串参数"""
    # 获取重定向的最终地址
    redirect_list = session.get(
        url=main_url, headers=http_header, proxies=proxies
    ).history

    if not redirect_list:
        raise Exception("无法获取重定向信息，请检查网络连接")

    redirect_host = redirect_list[-1].headers.get("location")
    if not redirect_host:
        raise Exception("无法获取重定向地址")

    if "success.jsp" in redirect_host:
        raise Exception("已登录，无法获取必要数据")

    # 从最终地址中解析出登录所需queryString参数
    redirect_host_response = session.get(
        url=redirect_host, headers=http_header, proxies=proxies
    ).text

    # 使用正则表达式提取queryString，更加健壮
    match = re.search(r"/index\.jsp\?([^\'\"]+)", redirect_host_response)
    if not match:
        # 回退到原来的方法
        try:
            login_querystring = redirect_host_response[
                redirect_host_response.find("/index.jsp?")
                + 11 : redirect_host_response.find("'</script>")
            ]
            return login_querystring
        except:
            raise Exception("无法从页面提取登录参数")

    return match.group(1)


def login(username: str, password: str, service: str) -> Union[str, int]:
    """
    执行登录操作

    Args:
        username: 用户名
        password: 密码
        service: 服务类型 (CHINATELECOM/CHINAMOBILE/CHINAUNICOM/EDUNET)

    Returns:
        成功时返回userIndex，失败时返回-1
    """
    # 从字典中获取服务代码，默认为空字符串
    service_code = SERVICE_CODES.get(service, "")
    if not service_code:
        print(f"\033[1;93m[警告]\033[0m 未知的服务类型: {service}")

    try:
        login_querystring = getQueryString()

        login_post_url = main_url + "eportal/InterFace.do?method=login"
        login_post_data = {
            "userId": username,
            "password": RSAEncryptor.encryptedPassword(password),
            "service": service_code,
            "queryString": login_querystring,
            "operatorPwd": "",
            "operatorUserId": "",
            "validcode": "",
            "passwordEncrypt": "true",
        }

        login_post_response = session.post(
            url=login_post_url,
            data=login_post_data,
            headers=http_header,
            proxies=proxies,
        )
        login_status = login_post_response.text.encode("raw_unicode_escape").decode()

        if '"result":"success"' in login_status:
            print("\033[1;92m[登录成功]\033[0m")

            # 提取 userIndex，使用正则表达式更安全
            match = re.search(r'userIndex":"([^"]+)"', login_status)
            if match:
                return match.group(1)

            # 回退到原来的方法
            try:
                user_index = login_status[
                    login_status.find("userIndex")
                    + 12 : login_status.find('","result"')
                ]
                return user_index
            except:
                print("\033[1;93m[警告]\033[0m 登录成功但无法获取用户索引")
                return ""
        else:
            # 提取错误信息
            error_msg = "未知错误"
            if '"message"' in login_status:
                match = re.search(r'"message":"([^"]+)"', login_status)
                if match:
                    error_msg = match.group(1)
                else:
                    try:
                        error_msg = login_status[
                            login_status.find('"message"')
                            + 11 : login_status.find('","forwordurl"')
                        ]
                    except:
                        pass

            print(f"\033[1;91m[登录失败]\033[0m {error_msg}")
            return -1

    except requests.exceptions.ConnectionError:
        raise Exception("向登录接口请求数据失败，请检查WLAN连接情况")
    except Exception as e:
        raise Exception(f"登录过程发生错误: {str(e)}")
