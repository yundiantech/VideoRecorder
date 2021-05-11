/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef DeviceSettingDialog_H
#define DeviceSettingDialog_H

#include <QFile>
#include <QDialog>
#include <QDateTime>
#include <QTimer>
#include <QProcess>

#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioInput>

#include "AudioInfo.h"

namespace Ui {
class DeviceSettingDialog;
}

class DeviceSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSettingDialog(QWidget *parent = 0);
    ~DeviceSettingDialog();

    static QStringList mCurrentDevicesName;
    QStringList getCurrentDevicesName(); //获取当前设置中的设备名称

//    bool isEnableVirtualAudioCapture(); //是否启用声卡声音录制

signals:
    void sig_ChangeMic(int state);

private:
    Ui::DeviceSettingDialog *ui;

    bool mHasAudioOutDevice;
    bool mHasAudioInDevice;
    bool mHasCameraDevice;

    bool mCameraTestPassed;   //视频设备检测通过
    bool mNetWorkTestPassed;  //网络检测是否通过
//    bool mAudioTestPassed;    //音频设备检测通过
    bool mAudioOutTestPassed; //播放设备检测通过
    bool mAudioInTestPassed;  //录音设备检测通过

    QString mCurrentCameraDeviceName;    //当前检测的摄像头名称
    QString mCurrentAudioOutDeviceName;   //当前检测的耳机设备名称
    QString mCurrentAudioInDeviceName;    //当前检测的录音设备名称

    bool mIsCurrentCameraDeviceReMoved;  //当前检测的摄像头是否被拔掉过
    bool mIsCurrentAudioOutDeviceReMoved; //当前检测的录音设备是否被拔掉过
    bool mIsCurrentAudioInDeviceReMoved;  //当前检测的录音设备是否被拔掉过

    ///初始化设备信息
    void initDevice();
    void initDevice_Camera(QString defaultDeviceName = "");
    void initDevice_AudioOut(QString defaultDeviceName = "", bool keepCurrentDevice = true);
    void initDevice_AudioIn(QString defaultDeviceName = "", bool keepCurrentDevice = true);

    ///麦克风测试
    AudioInfo *m_audioInfo;
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    /**
     * @brief startAudioIn
     * @param isStartPlay  时候要开始录制 false则 只是初始化设备
     * @param keepCurrentDevice 设备不存在的情况下是否保留当前设备
     */
    void startAudioIn(bool keepCurrentDevice = true); //打开麦克风
    void stopAudioIn();

    void setAudioWaveValue(int value); //显示音频显示时候的 声音强度

    ///设置单个设备输出音量
    float GetAudioOutputVolume(QString deviceName);
    int SetAudioOutputVolume(QString deviceName, float volumnLevel);

    ///设置单个设备输入音量
    float GetAudioInputVolume(QString deviceName);
    int SetAudioInputVolume(QString deviceName, float volumnLevel);

//    ///只是设置当前默认设备的音量（多个设备的情况下比较坑）
//    bool SetVolumeLevel(int level);
//    int GetVolumeLevel();

    void saveToFile();   //将设置保存到文件中
    void testFinished(); //结束检测

private slots:
    void slotBtnClick(bool isChked);
    void slotCurrentIndexChanged(int index);
    void slotSliderMoved(int value);

    void slotAudioInVolumeGetted(QByteArray buffer);
    void slotAudioDeviceStateChanged(QAudio::State state);

};

#endif // VIDEOSETTINGDIALOG_H
