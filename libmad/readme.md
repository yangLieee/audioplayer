# 源码链接
https://www.linuxfromscratch.org/blfs/view/svn/multimedia/libmad.html

# 编译方式
1. 进入libmad-0.15.1b目录
2. ./configure --prefix=$HOME/local/prior
3. 进入 Makefile 找到-fforce-mem并删除
4. make; 编译
5. make install; 安装

# 改动
目前libmad-0.15.1b目录将源码中所有.c .h文件拷贝编译
同时针对编译不通过需要.dat的文件进行拷贝，最终形成目前的目录结构，其中mad.h是api头文件将其导出

