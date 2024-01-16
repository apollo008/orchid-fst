### Orchid-Fst  编译和安装

目前支持类Unix环境下编译安装，Orchid-Fst 项目依赖的第三方库有：

- cppunit 
- tulip-log

编译安装Orchid-Fst工程之前先要安装以上2个依赖库, 1个自研日志库tulip-log,1个单元测试库cppunit，都是share目录下，提前编译好即可。


```
git clone https://github.com/apollo008/orchid-fst.git orchid-fst.git
cd orchid-fst.git 

make build-dir
cd build-dir
sudo cmake -DENABLE_BUILD_SHARE=ON ../src      #编译share中的依赖库并安装到/usr/local，cppunit和tulip
sudo rm -rf *    #重新清空build-dir目录下内容，开始编译orchid-fst工程
cmake ../src
make -j4
sudo make install  # 生成fst和lfsort命令到/usr/local/bin目录下，分别运行--help可以了解用法
sudo ldconfig
make test    #运行单元测试
```

```
xxxxxx@cent7ay:~/work/projects/orchid-fst.git/build-dir$ make test
Running tests...
Test project ~/work/projects/orchid-fst.git/build-dir
    Start 1: common_util_unittest
1/4 Test #1: common_util_unittest .............   Passed    0.01 sec
    Start 2: cache_unittest
2/4 Test #2: cache_unittest ...................   Passed  280.42 sec
    Start 3: fst_unittest
3/4 Test #3: fst_unittest .....................   Passed   21.83 sec
    Start 4: large_file_sorter_unittest
4/4 Test #4: large_file_sorter_unittest .......   Passed    0.01 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) = 302.27 sec

```

执行完以上，即可在/path/to/install目录下生成bin、lib、include3个目录。其中lib目录是libfst_core_lib.so bin目录下是lfsort和ofst可执行程序； include目录是orchid-fst库的头文件。

注意：运行ofst和lfsort程序时，需要在当前目录放置tulip-log的日志配置文件logger.conf ，同时该目录下要已经创建好logs 目录供输出文件名滚动的日志文件。

一个logger.conf内容供参考如下,或者可以从工程的misc/config/logger.conf或者示例配置，同时不要忘记根据logger.conf的配置在运行程序文件的当前目录下创建日志目录logs,另外控制logger.conf的第三行日志级别INFO或DEBUG、ERROR或WARN将使你获得不同详细程序的日志输出信息。

```
#Tulip log配置文件

tlog.rootLogger=INFO, tulipAppender,consoleAppender
tlog.appender.tulipAppender=FileAppender
tlog.appender.tulipAppender.max_file_size=3024
tlog.appender.tulipAppender.fileName=logs/app.log
tlog.appender.tulipAppender.flush=false
tlog.appender.tulipAppender.delay_time=1
tlog.appender.tulipAppender.layout=PatternLayout
tlog.appender.tulipAppender.layout.LogPattern=[%%d] [%%t,%%F:%%n -- %%f() %%l] [%%m]

tlog.appender.consoleAppender=ConsoleAppender
tlog.appender.consoleAppender.delay_time=1
tlog.appender.consoleAppender.layout=PatternLayout
tlog.appender.consoleAppender.layout.LogPattern=[%%d] [%%t,%%F:%%n -- %%f() %%l] [%%m]

tlog.appender.udpAppender=UdpAppender
tlog.appender.udpAppender.ip=192.168.0.211
tlog.appender.udpAppender.port=14878
tlog.appender.udpAppender.layout=PatternLayout
tlog.appender.udpAppender.layout.LogPattern=[%%d] [%%t,%%F:%%n -- %%f() %%l] [%%m]
####################################################################
```

### 其它

相关细节或其它未尽事宜，可联系 dingbinthu@163.com 探讨咨询。