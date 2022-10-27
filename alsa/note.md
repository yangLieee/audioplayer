# 官网： https://www.alsa-project.org/alsa-doc/alsa-lib/

# 待改进
1. 考虑snd_config_update_free_global() 函数的调用
   snd_config 变量由snd_config_update初始化或更新。
   像snd_pcm_open这样的函数（使用全局配置中的设备名称）会自动调用snd_config_update。在第一次调用snd_config_update之前，此变量为NULL
   全局配置文件在环境变量ALSA_CONFIG_PATH中指定。如果未设置，默认值为“/usr/share/alsa/alsa.conf”

2. underrun?
   ALSA 驱动 buffer 没有数据可以丢给 codec , 上层喂数据的速度太慢了导致"饿死"codec。目前只做了简单的处理，后续须避免underrun

