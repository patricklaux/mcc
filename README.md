# mcc



### 编译运行

```bash
# 下载源码
git clone git@github.com:patricklaux/mcc.git
# 进入目录
cd mcc
# 编译 mcc
gcc -o mcc ./src/mcc/lexer.c ./src/mcc/parser.c ./src/mcc/vm.c ./src/mcc/mcc.c
# 使用编译好的 mcc 运行测试代码
./mcc [-s] [-d] ./src/test/test1.c
```

