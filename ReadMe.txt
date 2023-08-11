欢迎使用LocalCTP!

本项目是一个部署于本地的仿CTP项目。如果有意见或建议，请联系作者秋水Aura(QQ 1005018695)。独自制作不易，欢迎打赏投食。
本项目不联网，完全开源，实现了大部分交易接口和查询接口。
本项目可以通过API接口来获取外部传入的行情快照，以判断订单的成交情况。
    - 支持全市场的期货/套利组合合约/期权的交易。
    - 支持FAK/FOK订单。
    - 成交撮合逻辑同SimNow，通过是否满足快照中的对手价来判断是否成交。
    - 不支持市价单和部分成交，这一点也同SimNow。
    - 不支持条件单和预埋单。

使用方法：
使用已经生成好的dll,以替换CTP的同名的 交易dll库文件(即 thosttraderapi_se.dll或so)。
(项目中自带的dll是windows版, 64位, 使用v6.5.1版本编译生成。)
或者：
自己根据LocalCTP代码来生成，可以自由选择CTP的版本和位数(32/64).


用大白话来讲就是：LocalCTP 和融航类似，也是用 LocalCTP 自身的dll文件来替换CTP的原始的同名的dll，以实现无缝切换。
LocalCTP内部干了啥：
咱们通过CTP的API去下单，是报单到了CTP服务器，比如SimNow的仿真CTP服务器，或者期货公司的实盘CTP服务器。
而LocalCTP呢，它并不联网，下单时，并不会把你的单子通过网络发出去，它是在API内部，进行撮合和判断成交和更新账户持仓等，然后将成交回报等通过SPI发给用户。


1.
支持的接口如下:
普通接口:
CreateFtdcTraderApi
GetApiVersion
Release
Init
Join
GetTradingDay
RegisterFront
RegisterFensUserInfo -> 接收行情快照
RegisterSpi
ReqAuthenticate
ReqUserLogin
ReqUserLogout
ReqSettlementInfoConfirm

订单相关接口:
ReqOrderInsert
ReqOrderAction

查询相关接口:
ReqQryInstrument
ReqQryDepthMarketData
ReqQryInvestor
ReqQryOrder
ReqQryTrade
ReqQryTradingAccount
ReqQryInvestorPosition
ReqQryInvestorPositionDetail
ReqQrySettlementInfo
ReqQrySettlementInfoConfirm

2.
部分接口说明:
Init:内部并不会连接网络，会从当前目录(或环境变量中的目录)的 instrument.csv 中读取合约信息。格式参见附带的同名文件。
Join:会直接返回。
GetTradingDay:会尽可能正确地返回交易日，无需登录。能处理夜盘(包括周五夜盘)的情况，但无法识别判断长假假期。
RegisterFront:会直接返回，并不会连接到参数中的地址。
RegisterFensUserInfo:【重要】被修改为接收行情快照的接口。内部会将参数转化为 CThostFtdcDepthMarketDataField* 类型并处理以更新行情数据，请在外部收到行情快照时调用此接口，使用方法可参考DEMO。
ReqAuthenticate/ReqUserLogin/ReqUserLogout: 都不会校验参数，即都会直接认证/登录/登出成功。
ReqOrderAction:支持两种撤单方式:
    1.OrderRef+FrontID+SessionID(还需填IntrumentID)
    2.OrderSysID+ExchangeID

3.
支持同一个进程内创建多API，不同API之间数据互相独立。

4.
本API默认都是由6.5.1版本头文件编译生成，如需替换为别的API版本，需修改包含目录并重新编译，具体步骤:

Windows:
(生成目录: bin/win/)
请把 ./LocalCTP/ctp_file/current 设置为你要使用的CTP的版本的头文件文件夹的副本(VS不支持快捷方式作为包含目录)。

Linux:
(生成目录: bin/linux/  通过makefile文件来make生成。已存在的 libthosttraderapi_se.so 文件为在centos7.8中生成)
请把 current 设置为你要使用的CTP的版本的头文件文件夹的副本或软链接。
示例(设为指向 6.3.19 版本的软链接):
cd ./LocalCTP/ctp_file/
ln -snf  ./6.3.19  ./current

注：修改 current 指向的版本后，需要重新编译生成dll或so文件。请做好备份。
切换版本后，可能需要将一些API中的新增的接口的虚函数进行实现，或者移除派生类中此前的继承的(已不存在的)虚函数，同时，可能部分函数或字段名称有调整，请根据实际情况来调整。


5.
账户初始数据:
所有账户的初始资金是2000万，持仓为空。
所有合约的保证金率全部为10%，合约的手续费全部为1元每手。
持仓和资金，会根据订单、成交和行情数据等来动态更新。

目前版本中，账户数据都不会保存到本地，即退出程序后，账户数据会重置。
下个版本中，账户数据会持久化保存到数据库中，敬请期待哦。




关于 TestLocalCTP：

TestLocalCTP 是测试 LocalCTP 的一个 DEMO, 仅用于展示 LocalCTP 的部分功能。