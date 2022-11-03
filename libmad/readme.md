# 另外一种编译方式

1. 进入libmad-0.15.1b目录
2. ./configure --prefix=$HOME/local/prior
3. 进入 Makefile 找到-fforce-mem并删除
4. make; 编译
5. make install; 安装
