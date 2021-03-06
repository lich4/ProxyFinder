# ProxyFinder
http/https proxy find and validate all in one

软件发布：ProxyFinder V1.0.0  
这款软件用于自动化搜索http/https代理，具有如下特点：  
* 界面简洁，傻瓜式操作；自定义配置方式灵活
* 自动收集网上可用代理并自动验证，一键完成
* 验证通过后会在主页面显示最优的N个代理，每行设置左键菜单，可以选择设置为IE代理
* 支持ss/ssr服务器搜索

自定义代理搜索源格式：
```
{
  "proxy_server": [{
		"url": "http://ab57.ru/downloads/proxylist.txt",
		"type": "txt"
	},{
		"url": "http://ip.seofangfa.com/",
		"type": "html",
		"root": "//div[@class='table-responsive']//tbody[1]/tr",
		"host": "td[1]/text()",
		"port": "td[2]/text()",
		"page": "%1:1:10"
	}]
}
```

字段：
* url 网址
* type txt代表url的响应中可以使用正则搜索到代理IP:PORT，html代表可使用XPATH方式获取代理IP:PORT
* root 使用XPATH时搜索IP/PORT的根元素
* host IP相对于root根元素的XPATH
* port PORT相对于root根元素的XPATH
* page url中存在多页代理需要收集

ssr订阅服务器配置
* 主界面选择SSR服务器搜索，直到搜索完毕不再刷新
* ssr中设置订阅服务器为http://127.0.0.1:81，如果不成功检查如下两点：
* * netstat -ano | find ":81" 找出占用端口的进程
* * 确保ssr代理模式处于PAC或者全局模式
* 出现代理订阅成功，即可
* ssr->服务器->服务器连接统计，选择速度最快的服务器


![截图](https://raw.githubusercontent.com/lichao890427/ProxyFinder/master/screenshot/proxyfinder_v1.1.png)

![截图](https://github.com/lichao890427/ProxyFinder/raw/master/screenshot/ssr.png)
