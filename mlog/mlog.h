#ifndef _MLOG_H_
#define _MLOG_H_

/* 文件说明：自定义一套等级打印宏
 * 使用方法：用户自定义MLOG_LEVEL ERROR/WARNING/INFO/DEBUG即可
 */

#define ERROR   0
#define WARNING 1
#define INFO    2
#define DEBUG   3

#define FMT     "(%s)<%s>[%d]  "
#define CONTENT __FILE__,__func__,__LINE__

//#define MLOG(format,...) fprintf(stderr, FMT format "\n", CONTENT,##__VA_ARGS__)
//#define MLOG(format,args...) fprintf(stderr, FMT format "\n", CONTENT,##args)
/* for debug */
#define MLOG()                              \
    do{                                     \
        fprintf(stderr, FMT "\n", CONTENT); \
    }while(0)

#define MLOGE(format, args...)                                              \
    do{                                                                     \
        if(MLOG_LEVEL >= ERROR)                                             \
            fprintf(stderr, "Error ==> " FMT format "\n", CONTENT, ##args); \
    } while(0)

#define MLOGW(format, args...)                                                  \
    do{                                                                         \
        if(MLOG_LEVEL >= WARNING)                                               \
            fprintf(stderr, "Warning ==> " FMT format "\n", CONTENT, ##args);   \
    } while(0)

#define MLOGI(format, args...)                                              \
    do{                                                                     \
        if(MLOG_LEVEL >= INFO)                                              \
            fprintf(stderr, "Info ==> " FMT format "\n", CONTENT, ##args);  \
    } while(0)

#define MLOGD(format, args...)                                              \
    do{                                                                     \
        if(MLOG_LEVEL >= DEBUG)                                             \
            fprintf(stderr, "Debug ==> " FMT format "\n", CONTENT, ##args); \
    } while(0)

#endif /* _MLOG_H_ */

