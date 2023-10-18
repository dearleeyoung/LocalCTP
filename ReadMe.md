# 欢迎使用  LocalCTP

[csdn文章](https://blog.csdn.net/baidu_37097818/article/details/132944895) [公众号文章](https://mp.weixin.qq.com/s/-XndrmQ9UGNLz-E1cHZHSA)

`LocalCTP`是一个部署于本地的仿CTP项目。本项目不联网，完全开源，接口完全同CTP，实现了大部分CTP柜台的功能。

如果有意见或建议，请联系作者秋水Aura(QQ 1005018695)。独自制作不易，欢迎打赏投食。

三人行，必有我师焉。欢迎加入QQ群 [736174420](http://qm.qq.com/cgi-bin/qm/qr?_wv=1027&k=HbTq4dfRMNyZNe9PoB4qAek-U0YVrmbx&authKey=XSXgfCnhpIESibRkbE0%2BswD3er9rN6HjbaALy%2BDg4dSww7Qr82TEKE6xBoSzls7O&noverify=0&group_code=736174420) 一起交流讨论 `LocalCTP`。


## LocalCTP特点
1. 支持全市场的期货/套利组合合约/期权的交易
1. 本地部署, 稳定运行, 策略安全得到彻底保障
1. 支持`windows/linux/MacOS/UNIX/FreeBSD`等多个操作系统
1. 支持FAK/FOK订单, 支持条件单.
1. 成交撮合逻辑同SimNow, 通过是否满足行情中的对手价来判断是否成交。
1. 支持多个CTP版本, 切换CTP版本后 仅需使用项目中的脚本自动生成CTP相关代码即可, 无需手动改代码适配
1. 可以通过特定API接口来获取外部传入的行情快照, 以更新账户的订单和资金等数据
* 可以投喂给它实时行情以实现 实时仿真交易
* 也可以投喂给它历史行情以实现 回测


## 使用方法
**用LocalCTP它的dll（或so）文件（交易的dll库文件，即 `thosttraderapi_se.dll或so`），来替换你使用的CTP的同名的库文件。**
请做好原始文件的备份。

### 懒人版
直接使用项目中已经生成好的dll或so。也可以直接下载右侧的`Releases`栏目中的Release包(包中包含若干其他版本的库文件).

默认windows版dll（`thosttraderapi_se.dll`）是：

* 64位, 使用VS2019和 CTP v6.5.1 版本头文件编译生成
* （更多版本正在赶来，敬请期待……）

默认linux版等so（`thosttraderapi_se.so`，前面可能带有lib前缀，不影响使用）是：

* 64位, 使用CTP v6.5.1 版本头文件编译生成，在CentOS7.8 和 Ubuntu18.04上测试通过。
* （更多版本正在赶来，敬请期待……）

### DIY版
根据LocalCTP库的代码来编译生成dll或so库并拿来使用，可以自由选择CTP（头文件）的版本和平台位数（32/64）。这种方法适合于有一定动手能力的玩家。

生成路径: 

windows: bin/win/  (通过 Visual Studio 2019 工程生成)

linux等其他系统: bin/linux/ (通过`makefile`文件来 make 生成)

**本API默认都是由v6.5.1版本头文件编译生成，如需替换为别的API版本，需修改包含目录并重新生成代码文件和编译，具体步骤:**

**第一步, 替换包含目录current中的文件内容.**

将 `./LocalCTP/ctp_file/current/`目录 中的文件设置为你要使用的CTP的版本的头文件。

Linux中, 还可以将`current`设置为指向实际CTP头文件目录软链接, 示例(设为指向 v6.6.9 版本的软链接):

`cd ./LocalCTP/ctp_file/`

`ln -snf  ./6.6.9  ./current`


**第二步, 使用脚本自动生成CTP相关代码.**

在切换CTP头文件(`current` 目录中的文件) 后, 请执行python脚本(`GenScript/ParseCTPHeaders.py`)以自动生成对应CTP版本的C++代码.

使用方法( 先`cd`切换到`GenScript`目录中 ):

`python3 ParseCTPHeaders.py`

**完成以上两步之后, 就可以重新编译生成啦！注意做好备份哦.**

项目中已准备好CTP全版本自动编译的脚本: `buildLinux.sh` 能一次性生成多个版本的库


## LocalCTP内部干了啥：
咱们通过CTP的API去下单，是报单到了CTP服务器，比如SimNow的仿真CTP服务器，或者期货公司的实盘CTP服务器。

而LocalCTP呢，它并不联网，下单时，并不会把你的单子通过网络发出去，它是在API内部，进行撮合和判断成交和更新账户持仓等，然后将成交回报等通过SPI发给用户。

**LocalCTP包含交易API，而不包含行情API。**

**用户可以通过CTP的行情API从实盘获取行情快照来传入LocalCTP中。**

**（友情提示：行情API实盘登录时并不会校验用户名和密码，实盘行情地址可咨询期货公司，也可以加入QQ群获取地址。）**


### 支持的接口如下:
#### 普通接口:
1. `CreateFtdcTraderApi`
1. `GetApiVersion`
1. `Release`
1. `Init`
1. `Join`
1. `GetTradingDay`
1. `RegisterFront`
1. `RegisterFensUserInfo` -> 接收行情快照
1. `RegisterSpi`
1. `ReqAuthenticate`
1. `ReqUserLogin`
1. `ReqUserLogout`
1. `ReqSettlementInfoConfirm`

#### 订单相关接口:
1. `ReqOrderInsert`
1. `ReqOrderAction`
1. `ReqQuoteInsert` -> 接收行情快照

#### 查询相关接口:
1. `ReqQryInstrument`
1. `ReqQryDepthMarketData`
1. `ReqQryInvestor`
1. `ReqQryOrder`
1. `ReqQryTrade`
1. `ReqQryTradingAccount`
1. `ReqQryInvestorPosition`
1. `ReqQryInvestorPositionDetail`
1. `ReqQrySettlementInfo`
1. `ReqQrySettlementInfoConfirm`
1. `ReqQryClassifiedInstrument`
1. `ReqQryExchange`
1. `ReqQryProduct`
1. `ReqQryInstrumentMarginRate`
1. `ReqQryInstrumentCommissionRate`

### 部分接口说明:
1. `Init`: 内部并不会连接网络。系统启动时会从当前目录(或环境变量中的目录)的 `instrument.csv`和数据库合约表 中读取合约信息。格式参见附带的同名文件。
1. `Join`: 会直接返回。
1. `GetTradingDay`: 会尽可能正确地返回交易日，无需登录。能处理夜盘(包括周五夜盘)的情况，但无法识别判断长假假期。
1. `RegisterFront`: 会直接返回，并不会连接到参数中的地址也不会校验地址合法性。
1. `RegisterFensUserInfo`: 【重要】被修改为接收行情快照的接口。内部会将参数转化为 `CThostFtdcDepthMarketDataField*` 类型并处理以更新行情数据，请在外部收到行情快照时调用此接口。
1. `ReqQuoteInsert`: 【重要】也被修改为接收行情快照的接口。内部会将一些参数中的字段转换为行情快照中的字段，数据字段转换规则见附录。
用户可以使用上述这两个接口中的任意一个来为本系统提供行情快照数据. 使用方法可参考DEMO.
1. `ReqAuthenticate/ReqUserLogin/ReqUserLogout`: 都不会校验参数，即都会直接认证/登录/登出成功。
1. `ReqOrderInsert`: 支持条件单(支持四种价格条件TThostFtdcContingentConditionType,即用最新价LastPrice和条件价StopPrice的四种比较类型)
1. `ReqQryInstrumentMarginRate/ReqQryInstrumentCommissionRate`: 会返回所有符合条件的合约的保证金率或手续费率数据,而不像CTP中只能按合约查询并只返回一条
1. `ReqOrderAction`: 支持两种撤单方式:
    * ` OrderRef + FrontID + SessionID (还需填IntrumentID)`
    * ` OrderSysID + ExchangeID`


### 账户数据说明:
账户初始数据:
所有账户的初始资金都是2000万，初始持仓为空。

合约的费率（保证金率和手续费率）从数据库的两张费率表中读取，如果费率表中没有该合约数据，则按默认值处理（保证金率全为10%，手续费全为1元每手）。

持仓和资金，会根据订单、成交和行情数据等来动态更新。

账户数据会自动持久化保存到本地的sqlite3数据库(LocalCTP.db,会自动创建数据库及表,无需手动创建)中. 分为资金,持仓,持仓明细,订单,成交等表.

系统使用sqlite内存数据库以提高性能，数据会定时从内存数据库中持久化写入到硬盘中。

1. 持仓表 - `CThostFtdcInvestorPositionField`
1. 持仓明细表 - `CThostFtdcInvestorPositionDetailField`
1. 委托表 - `CThostFtdcOrderField`
1. 成交表 - `CThostFtdcTradeField`
1. 资金表 - `CThostFtdcTradingAccountField`
1. 合约表 - `CThostFtdcInstrumentField`
1. 合约保证金率表 - `CThostFtdcInstrumentMarginRateField`
1. 合约手续费率表 - `CThostFtdcInstrumentCommissionRateField`
1. 平仓明细表 - `CloseDetail`
1. 结算单表 - `SettlementData`

如果登录数据库中还不存在的账户,则会添加该账户到表中并初始化为资金为2000万和持仓为空.

Windows中可使用 [DB Browser for SQLite](https://sqlitebrowser.org/) 等软件以查看和修改数据库中的账户数据.


#### 结算功能
每个交易日的下午17点左右，系统会对所有账户进行结算。结算单会存储在数据库中。

报单时，不会校验账户是否已经确认过结算单。


### 本项目还存在的一些可以改进的小的点：

1. 算法拆单功能
1. 支持其他行情投喂方式
1. 社交、短视频功能(？)


### 附录

**`ReqQuoteInsert` 投喂行情的数据字段转换规则:**

具体字段对应:

|  字段含义 |  CThostFtdcDepthMarketDataField 中字段   | CThostFtdcInputQuoteField 中字段  | 说明 | 
|  ---- |  ----   | ----   | ----  |
|  交易日 | TradingDay  | BrokerID | |
| 业务日期 | ActionDay  | InvestorID | |
|  交易所代码 | ExchangeID  | ExchangeID | 名字不变 |
|  合约代码 | InstrumentID  | InstrumentID | 名字不变 |
|  最后修改时间 | UpdateTime  | ClientID | |
|  最后修改毫秒 | UpdateMillisec  | RequestID | 不是 函数参数 nRequestID 哦 |
|  数量(今日的成交量) | Volume  | 函数参数 nRequestID | 不是 RequestID 哦 |
|  申买价一 | BidPrice1  | BidPrice | |
|  申卖价一 | AskPrice1  | AskPrice | |
|  申买量一 | BidVolume1  | BidVolume | |
|  申卖量一 | AskVolume1  | AskVolume | |
|  最新价 | LastPrice  | UserID | 浮点数<->字符串 |
|  涨停价 | UpperLimitPrice  | BidOrderRef | 浮点数<->字符串 |
|  跌停价 | LowerLimitPrice  | AskOrderRef | 浮点数<->字符串 |
|  上次结算价(昨结算价) | PreSettlementPrice  | QuoteRef | 浮点数<->字符串 |
|  结算价 | SettlementPrice  | ForQuoteSysID  | 浮点数<->字符串 |
|  持仓量 | OpenInterest  | BusinessUnit | 浮点数<->字符串 |


      如: LastPrice (100.5) -> UserID ("100.5")

      转换示例(x 是一个CThostFtdcInputQuoteField实例):

                  python: x.UserID = str(100.5)

                  Java:   x.UserID = Float.toString(100.5);

                  C#:     x.UserID = 100.5.ToString();

                  C++:    strcpy(x.UserID, std::to_string(100.5).c_str());

**制作不容易，请一定要多多支持！欢迎打赏投食鼓励！~**

<div align=center>
    <img src="https://i0.hdslb.com/bfs/article/87ce53f38c1d5c8272dd59cf2830cb648a46c85f.jpg" alt="Support">
</div>

