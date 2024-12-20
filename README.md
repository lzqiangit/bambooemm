# Bambooemm
## Build and run

```sh
mkdir build
cd build
cmake ..
cd test
make
./demo
```


## 方案介绍
初始化和插入时,将所有的key均按照存在最大容量个数的value填充至emm,对于value不存在(key|counter),value处填充enc(key|counter|0) 
## 待办
##### 测试
- [ ] Client端空间开销
- [ ] Server端空间开销
- [ ] 查询时间
- [ ] 更新时间
##### 处理特殊情况
- [x] l变大的逻辑
- [x] l变小的逻辑
- [x] 插入新的key的时候的逻辑

##### 修复bug
- [x] KV构造函数修改为深拷贝方式, 传入的指针需要手动在类外释放, 需要修改！！！
- [x] 一次融合中存在对同一个key的多次删除操作,会造成操作错位
- [x] 一次融合中存在对同一个key的多次添加元素操作,只有最后一个值能够正常添加
时不时解密失败???

##### 实现update
- [x] EMMu,st的数据结构定义    (每次只有一个value!)     -- EMMu的大小?? EMMu设置为哈希表，碰撞了咋整？
- [x] 实现更新函数 Clien::Update(key,op,value) - Server::Update(y, EMMu元素)
- [x] 实现在update操作中融合更新 ！！！

##### 修改value的存储方案
- [x] 冲突的value存储到同一个entry的value中
- [x] 修复reinsert功能的bug (更名为EncryptAndUpload)
- [x] 实现拼接, 拆分ValueEntry的工具方法 (一个valueEntry中存在多个value,只添加一个random, 减少存储开销 )
##### 实现reinsert 
! 先填充，后加密    
- [x] 在value后拼接用于实现前后向安全的随机数: 完善value的拼接和解析
- [x] 修改加密方案，先填充，后加密，对于冲突的部分，如果已存储的是空值，那么就覆盖
- [x] 实现从bemm中取回数据,修改value后的随机数,然后放回到bemm中

## 实现细节备忘
ValueEntry中: 
- 同一个value的不同部分通过 '|' 拼接
- 不同value通过 ',' 拼接为values
- values同random通过 '$' 拼接 
- 填充标志 P