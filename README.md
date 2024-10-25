# Bambooemm
## Build and run

```
mkdir build
cd build
cmake ..
cd test
make
./example
```


## 方案介绍
初始化和插入时,将所有的key均按照存在最大容量个数的value填充至emm，对于value不存在(key|counter),value处填充enc(key|counter|0) 

## 
！先填充，后加密
- [x] 在value后拼接用于实现前后向安全的随机数: 完善value的拼接和解析
- [ ] 修改加密方案，先填充，后加密，对于冲突的部分，如果已存储的是空值，那么就覆盖
- [ ] 实现从bemm中取回数据，修改value后的随机数，然后放回到bemm中
