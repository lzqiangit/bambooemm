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