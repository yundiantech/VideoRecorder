/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */
#ifndef VideoRecorderEventHandle_H
#define VideoRecorderEventHandle_H

class VideoRecorderEventHandle
{
public:
    virtual ~VideoRecorderEventHandle();

//    /**
//     * @brief 输出状态
//     * @param state
//     */
//    virtual void OnStateChanged(const CallBackState &state) = 0;

    /**
     * @brief 输出音频音量
     * @param volumeL 左声道音量大小:0~100
     * @param volumeR 右声道音量大小:0~100
     */
    virtual void OnAudioVolumeUpdated(const int &volumeL, const int &volumeR) = 0;

};

#endif // VideoRecorderEventHandle_H
