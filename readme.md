## 1. 文件结构
### 1.1 Utils文件包
`AssertUtils.hpp`: 判定可变表达式是否完全正确


### 1.2 code剩余文件

`BaseInfo.hpp`: 基本数据管理类(单例)，负责所有表路径的初始化与保存路径定义


`GenerateDefaultRoute.hpp`: 生成默认路由表的类，目前支持输入结果到txt格式与二进制格式(二进制格式读取更快)


### 1.3 test文件包

`generateRoute.cpp`: 调用`GenerateDefaultRoute.hpp`生成路由表





