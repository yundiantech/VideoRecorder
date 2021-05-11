# VideoRecorder
# Qt+ffmpeg实现的录屏软件
博客地址：http://blog.yundiantech.com/?log=blog&scat=196

![Image text](https://raw.githubusercontent.com/yundiantech/VideoRecorder/master/pic/screenshot.png)


版本说明：  
Qt开发环境的搭建 请参考：  
http://blog.yundiantech.com/?log=blog&id=6  

Qt中引用FFMPEG库文件 请参考：  
http://blog.yundiantech.com/?log=blog&id=7  

学习音视频技术欢迎访问 http://blog.yundiantech.com    
音视频技术交流讨论欢迎加 QQ群 121376426    

PS:记得将ffmpeg/bin目录下的dll文件拷贝到编译生成的exe所在的目录下，否则会无法运行。  

版本一更新日志：  
【V1.0.0】从零开始学习音视频编程技术（十二） 录屏软件开发之视频采集  
Qt4.8.2(mingw) + ffmpeg2.5.2  
博客地址：http://blog.yundiantech.com/?log=blog&id=15  

【V1.1.0】从零开始学习音视频编程技术（十三） 录屏软件开发之屏幕录像  
Qt4.8.2(mingw) + ffmpeg2.5.2  
博客地址：http://blog.yundiantech.com/?log=blog&id=17  

【V1.3.0】从零开始学习音视频编程技术（十八） 录屏软件开发之编码AAC  
Qt4.8.2(mingw) + ffmpeg2.5.2 + SDL2  
博客地址：http://blog.yundiantech.com/?log=blog&id=24  

【V1.4.0】从零开始学习音视频编程技术（十九） 录屏软件开发之YUV AAC合成MP4  
Qt4.8.2(mingw) + ffmpeg2.5.2 + SDL2  
博客地址：http://blog.yundiantech.com/?log=blog&id=26  

【V1.5.0】从零开始学习音视频编程技术（二十） 录屏软件开发之录屏生成MP4  
Qt4.8.2(mingw) + ffmpeg2.5.2 + SDL2  
博客地址：http://blog.yundiantech.com/?log=blog&id=27  

【V1.6.0】从零开始学习音视频编程技术（二十一） 录屏软件开发之最终完善  
Qt4.8.2(mingw) + ffmpeg2.5.2 + SDL2  
博客地址：http://blog.yundiantech.com/?log=blog&id=28  



# PS:从版本2开始，无需手动拷贝dll的操作，且工程已可以自动判断编译器位数。  

【V2.0.0】 2019-03.24  
Qt5.6.2(vs2013) + ffmpeg4.1  
1.将ffmpeg升级到4.1版本，且同时支持32位和64位的编译器编译。  
2.移除SDL  

【V2.0.1】 2019-06.24  
Qt5.6.2(vs2013) + ffmpeg4.1  
1.解决平面PCM数据处理不当导致声音不正常的问题  
修改内容解释如下：  
![Image text](https://raw.githubusercontent.com/yundiantech/VideoRecorder/master/pic/code_1.jpg)

【V2.0.2】 2019-07.22  
Qt5.6.2(vs2013) + ffmpeg4.1  
1.解决windows自带播放器不能播放的问题  
2.修改文件地址为rtmp地址后即可实现推流



【V3.0.0】 2021-05.11  
Qt5.13.2(vs2017) + ffmpeg4.1  
1.功能增强，支持扩展模式  
2.录屏插件支持命令行直接注册  

