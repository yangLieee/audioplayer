# 工程简介
本工程将基于alsa库和开源音频解码库，实现一个支持多种音频格式的音频播放器，同时支持各种功能


# 环境及编译
## 环境
Ubuntu20.04.4 + gcc工具链
默认机器中已安装alsalib库(命令: sudo apt-get install libasound2-dev)

## 编译
1. mkdir build; cd build
2. cmake ..
3. make

## 执行
./mediaplayer wav文件名


# 代码结构
整体播放器的功能将通过c++设计模式中策略模式、单例模式以及一个状态机来实现

# 待优化
1. 目前只支持16BIT位宽PCM数据
2. 播放时跳转音乐参数是时间，目前总时间本身计算存在误差，所以最好使用百分比做参数
3. mp3音乐播放的时候调用stop接口，解码线程与播放线程没有同步，子线程交给了运行时库，不安全需要等待退出

# 调试经验
1. cmakelists编译静态或动态库，一定会在${PROJECT__NAME}前面加lib，后面加.a/.so; 如果自己的工程名字以lib开头则编译出的库依然添加lib字符串(liblibwave.a)


