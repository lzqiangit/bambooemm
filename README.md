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

## 
[ ]重写更新
[ ]重写融合
[ ]

## 实现细节备忘
ValueEntry中: 
- 同一个value的不同部分通过 '|' 拼接
- 不同value通过 ',' 拼接为values
- values同random通过 '$' 拼接 
- 填充标志 P