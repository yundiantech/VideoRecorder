/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "DeviceSettingDialog.h"
#include "ui_DeviceSettingDialog.h"

#include <thread>
#include <QListView>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QDebug>

#include "AppConfig.h"
#include "Base/FunctionTransfer.h"

QStringList DeviceSettingDialog::mCurrentDevicesName;

DeviceSettingDialog::DeviceSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSettingDialog)
{
    ui->setupUi(this);

//    this->setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint);  //使窗口置顶

    mHasAudioOutDevice  = false;
    mHasAudioInDevice   = false;
    mHasCameraDevice    = false;

    m_audioInfo  = NULL;
    m_audioInput = NULL;

//    camera        = NULL;
qDebug()<<__FUNCTION__<<"000";
    initDevice();
qDebug()<<__FUNCTION__<<"111";
//    connect(ui->pushButton_close,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));
//    connect(ui->pushButton_skip,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));

//    connect(ui->pushButton_audioOut_reflesh,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));
//    connect(ui->pushButton_audioIn_reflesh,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));
//    connect(ui->pushButton_camera_reflesh,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));

//    connect(ui->pushbutton_audioIn_add,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));
//    connect(ui->pushbutton_audioIn_sub,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));
//    connect(ui->pushbutton_audioOut_add,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));
//    connect(ui->pushbutton_audioOut_sub,SIGNAL(clicked(bool)),this,SLOT(slotBtnClick(bool)));

//    connect(ui->audio_inVolume,SIGNAL(sig_valueChanged(int)),this,SLOT(slotValueChanged(int)));
//    connect(ui->audio_outVolume,SIGNAL(sig_valueChanged(int)),this,SLOT(slotValueChanged(int)));

//    connect(ui->horizontalSlider_audioOut,SIGNAL(sliderMoved(int)),this,SLOT(slotSliderMoved(int)));
//    connect(ui->horizontalSlider_audioOut,SIGNAL(valueChanged(int)),this,SLOT(slotSliderMoved(int)));

//    connect(ui->horizontalSlider_audioIn,SIGNAL(sliderMoved(int)),this,SLOT(slotSliderMoved(int)));
    connect(ui->horizontalSlider_audioIn,SIGNAL(valueChanged(int)),this,SLOT(slotSliderMoved(int)));

    connect(ui->pushButton_audioTest_start, &QPushButton::clicked, this, &DeviceSettingDialog::slotBtnClick);
    connect(ui->pushButton_audioTest_stop,  &QPushButton::clicked, this, &DeviceSettingDialog::slotBtnClick);

    connect(ui->pushButton_confirm,  &QPushButton::clicked, this, &DeviceSettingDialog::slotBtnClick);

    connect(ui->pushButton_videoPath,  &QPushButton::clicked, this, &DeviceSettingDialog::slotBtnClick);


//    connect(ui->comboBox_camera,SIGNAL(activated(int)),this,SLOT(slotCurrentIndexChanged(int)));
    connect(ui->comboBox_audioIn,SIGNAL(activated(int)),this,SLOT(slotCurrentIndexChanged(int)));
//    connect(ui->comboBox_playdevice,SIGNAL(activated(int)),this,SLOT(slotCurrentIndexChanged(int)));

    ui->horizontalSlider_audioIn->hide();
    ui->pushButton_audioTest_stop->hide();

//    ui->lineEdit_videoPath->hide();
//    ui->pushButton_videoPath->hide();
//    ui->checkBox_virtual_audio_capture->hide();

//    ///获取设备信息
//    std::thread([=]
//    {
//        std::list<DeviceNode> videoDeviceList;
//        std::list<DeviceNode> audioDeviceList;

//        MediaManager::getDeviceList(videoDeviceList, audioDeviceList);

//        FunctionTransfer::runInMainThread([=]()
//        {
////            for (const DeviceNode &node : videoDeviceList)
////            {
////                ui->comboBox_videoDevice->addItem(QString::fromStdString(node.deviceName));
////            }

//            ui->comboBox_audioIn->clear();
//            for (const DeviceNode &node : audioDeviceList)
//            {
//                ui->comboBox_audioIn->addItem(QString::fromStdString(node.deviceName));
//            }
//        });

//    }).detach();

}

DeviceSettingDialog::~DeviceSettingDialog()
{
    stopAudioIn();

    delete ui;
}

void DeviceSettingDialog::initDevice()
{
//    initDevice_Camera();
//    initDevice_AudioOut();
qDebug()<<__FUNCTION__<<"000";
    initDevice_AudioIn();
qDebug()<<__FUNCTION__<<"222";
    ui->checkBox_virtual_audio_capture->setChecked(AppConfig::gEnableVirtualAudioCapture);
    ui->comboBox_videoQuality->setCurrentIndex(AppConfig::gVideoQuality-1);
    ui->lineEdit_videoPath->setText(AppConfig::gVideoDirPath);
}

void DeviceSettingDialog::initDevice_AudioIn(QString defaultDeviceName, bool keepCurrentDevice)
{
    QString audioInDeviceName = AppConfig::gAudioInputDeviceName;

    if (!defaultDeviceName.isEmpty())
    {
        audioInDeviceName = defaultDeviceName;
    }

    bool IsAudioInDeviceExist  = false; //是否存在文件中记录的输入设备ID
qDebug()<<__FUNCTION__<<"000"<<audioInDeviceName;
    ui->comboBox_audioIn->clear();
qDebug()<<__FUNCTION__<<"111";
    mHasAudioInDevice = (QAudioDeviceInfo::availableDevices(QAudio::AudioInput).size() > 0);
qDebug()<<__FUNCTION__<<"111.2222";
    int index = 0;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        ui->comboBox_audioIn->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));
//qDebug()<<audioInDeviceName<<deviceInfo.deviceName()<<index;
        if (audioInDeviceName == deviceInfo.deviceName())
        {
            IsAudioInDeviceExist = true;
            ui->comboBox_audioIn->setCurrentIndex(index);
        }
        index++;
    }
qDebug()<<__FUNCTION__<<"222";
    if (keepCurrentDevice)
    if (!defaultDeviceName.isEmpty() && !IsAudioInDeviceExist) //说明需要设为默认设备的设备——并不存在——没有插入
    {
        ui->comboBox_audioIn->addItem(defaultDeviceName, qVariantFromValue(NULL));
        ui->comboBox_audioIn->setCurrentIndex(index);

        mHasAudioInDevice = true;
        IsAudioInDeviceExist = true;
    }
qDebug()<<__FUNCTION__<<"333";
    if (mHasAudioInDevice)
    {
        if (!IsAudioInDeviceExist) //之前没有设置过麦克风 或者 设置的麦克风已经不存在了
        {
            ///设置成默认不使用USB麦克风 —— 通过名字是否带"USB"来判断 —— Begin
            for (int index = 0; index < ui->comboBox_audioIn->count(); index++)
            {
                QAudioDeviceInfo deviceInfo  = ui->comboBox_audioIn->itemData(index).value<QAudioDeviceInfo>();

                if (!deviceInfo.deviceName().contains("USB")
//                    &&!deviceInfo.deviceName().contains("HD")
                &&!deviceInfo.deviceName().contains("Webcam")
                &&!deviceInfo.deviceName().contains("LifeCam"))
                {
                    ui->comboBox_audioIn->setCurrentIndex(index);
                    break;
                }
            }
            ///设置成默认不使用USB麦克风 —— 通过名字是否带"USB"来判断 —— End
        }
    }
    else
    {
         ui->comboBox_audioIn->addItem(tr("disabled"));
    }
}

void DeviceSettingDialog::startAudioIn(bool keepCurrentDevice)
{

//    {
//        while (!mHasAudioInDevice)
//        {
//    //        int ret = MyMessageBox::showText(tr("warning"),tr("If equipment is unconnected, it will affect the class."),tr("Has been plugged in"),tr("Exit APP"));

////            int ret = MyMessageBox::showText(tr("warning"),QStringLiteral("话筒未接入/禁用，请接入/启用话筒"),QStringLiteral("已接入/启用话筒"),QStringLiteral("跳过话筒检测"));
//            int ret = MyMessageBox::showText(tr("warning"),tr("microphone unconnected/disable, please connect/enable microphone"),tr("Has been plugged in"),tr("skip microphone detection"));

//            if (ret == QDialog::Accepted)
//            {
//                initDevice_AudioIn();
//            }
//            else
//            {

//                setTestResult(Audio_In,false);
//                setTestStep(Camera);

//                return;
//            }
//        }
//    }

//    {
//        QString deviceName = ui->comboBox_audioIn->currentText();
//        if (deviceName == tr("disabled"))
//        {
//            deviceName.clear();
//        }
//        initDevice_AudioIn(deviceName, keepCurrentDevice);
//    }

//    ui->pushButton_audio_ok->setEnabled(mHasAudioInDevice);

    stopAudioIn();

    m_format.setSampleRate(8000);
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    int index = ui->comboBox_audioIn->currentIndex();
    QAudioDeviceInfo m_device  = ui->comboBox_audioIn->itemData(index).value<QAudioDeviceInfo>();

    mCurrentAudioInDeviceName = m_device.deviceName();
    mIsCurrentAudioInDeviceReMoved = false;
    mIsCurrentAudioOutDeviceReMoved = false;

qDebug()<<__FUNCTION__<<mCurrentAudioInDeviceName<<index;
    bool IsDeviceExist = false;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        if (mCurrentAudioInDeviceName == deviceInfo.deviceName())
        {
            IsDeviceExist = true;
            break;
        }
    }

    if (IsDeviceExist)
    {
        QAudioDeviceInfo info(m_device);
        if (!info.isFormatSupported(m_format)) {
            qWarning() << "Default format not supported - trying to use nearest";
            m_format = info.nearestFormat(m_format);
        }

//        mMyIoDevice.cleardata();
//        mMyIoDevice.open(QIODevice::WriteOnly);

        m_audioInfo  = new AudioInfo(m_format, this);
        connect(m_audioInfo, SIGNAL(update(QByteArray)), SLOT(slotAudioInVolumeGetted(QByteArray)));

        m_audioInput = new QAudioInput(m_device, m_format, this);

        m_audioInfo->start();
        m_audioInput->start(m_audioInfo);

//        mRecordingTimes = 0;
//        mTimer_Recording->start();

        float volumeLevel = GetAudioInputVolume(mCurrentAudioInDeviceName);

        if (volumeLevel < 0.3)
        {
            volumeLevel = 0.3;
            SetAudioInputVolume(mCurrentAudioInDeviceName, volumeLevel);
        }

        ui->horizontalSlider_audioIn->setValue(volumeLevel * 100);

        ui->pushButton_audioTest_stop->show();
        ui->pushButton_audioTest_start->hide();

//        startPlay(); //开始回放
//        startRePlay(); //开始回放
    }


}

void DeviceSettingDialog::stopAudioIn()
{
//    stopPlay(); //停止回放
//    stopRePlay(); //停止回放

    if (m_audioInput != NULL)
    {
        m_audioInfo->stop();
        m_audioInput->stop();
        m_audioInput->disconnect(this);

        delete m_audioInput;

        m_audioInput = NULL;
    }

    if (m_audioInfo != NULL)
    {
        m_audioInfo->disconnect(this);
        delete m_audioInfo;
        m_audioInfo = NULL;
    }

    ui->pushButton_audioTest_stop->hide();
    ui->pushButton_audioTest_start->show();

}

void DeviceSettingDialog::setAudioWaveValue(int value)
{
    ui->progressBar->setValue(value);
}

void DeviceSettingDialog::slotAudioInVolumeGetted(QByteArray buffer)
{
//    qDebug()<<__FUNCTION__<<buffer.size()<<m_audioInfo->level();
    setAudioWaveValue(m_audioInfo->level()*100);
}

void DeviceSettingDialog::slotAudioDeviceStateChanged(QAudio::State state)
{
    qDebug()<<__FUNCTION__<<state;
    if (state == QAudio::IdleState)
    {
//        startPlay();
//        mTimer_PlayProgress->stop();
    }
}

QStringList DeviceSettingDialog::getCurrentDevicesName()
{
    QString audioInName = "disabled";
    QString audioOutName = "disabled";
    QString cameraName = "disabled";

    if (mHasAudioInDevice)
    {
        int index = ui->comboBox_audioIn->currentIndex();
        QAudioDeviceInfo mDevice  = ui->comboBox_audioIn->itemData(index).value<QAudioDeviceInfo>();

        audioInName = mDevice.deviceName();
    }

//    if (mHasAudioOutDevice)
//    {
//        int index = ui->comboBox_playdevice->currentIndex();
//        QAudioDeviceInfo mDevice  = ui->comboBox_playdevice->itemData(index).value<QAudioDeviceInfo>();
//        audioOutName = mDevice.deviceName();
//    }

//    if (mHasCameraDevice)
//    {
//        int index = ui->comboBox_camera->currentIndex();
//        QCameraInfo cameraInfo  = ui->comboBox_camera->itemData(index).value<QCameraInfo>();
//        cameraName = cameraInfo.description();
//    }

    QStringList strList;

    strList.append(audioInName);
    strList.append(audioOutName);
    strList.append(cameraName);

    return strList;

}
void DeviceSettingDialog::saveToFile()
{
    QStringList deviceNameList = getCurrentDevicesName();
    QString audioInName = deviceNameList.at(0);
    QString audioOutName = deviceNameList.at(1);
    QString cameraName = deviceNameList.at(2);

    DeviceSettingDialog::mCurrentDevicesName = deviceNameList;

    AppConfig::gAudioInputDeviceName = audioInName;
    AppConfig::gEnableVirtualAudioCapture = ui->checkBox_virtual_audio_capture->isChecked();
    AppConfig::gVideoQuality = ui->comboBox_videoQuality->currentText().toInt();

    AppConfig::saveConfigInfoToFile();
}

void DeviceSettingDialog::testFinished()
{
    saveToFile();

    accept();
}

void DeviceSettingDialog::slotBtnClick(bool isChked)
{
    if (QObject::sender() == ui->pushButton_audioTest_start)
    {
        emit sig_ChangeMic(0);

        startAudioIn();
    }
    else if (QObject::sender() == ui->pushButton_audioTest_stop)
    {
        stopAudioIn();

        emit sig_ChangeMic(1);
    }
    else if (QObject::sender() == ui->pushButton_confirm)
    {
        stopAudioIn();

        testFinished();
    }
    else if (QObject::sender() == ui->pushButton_videoPath)
    {
        QString dirPath = QFileDialog::getExistingDirectory(this, QStringLiteral("选择视频保存目录"), AppConfig::gVideoDirPath);

        if (!dirPath.isEmpty())
        {
            ui->lineEdit_videoPath->setText(dirPath);
            AppConfig::gVideoDirPath = dirPath;
        }

    }
}

void DeviceSettingDialog::slotCurrentIndexChanged(int index)
{
    qDebug()<<__FUNCTION__<<index;
    if (index < 0) return;

    int		nCurSel = 0;
    QString strDeviceName;
    QString strDeviceID;

    if (QObject::sender() == ui->comboBox_audioIn)
    {
        stopAudioIn();
//        startAudioIn();

        setAudioWaveValue(0);
    }
}

void DeviceSettingDialog::slotSliderMoved(int value)
{
    if (QObject::sender() == ui->horizontalSlider_audioIn)
    {
//        if (m_audioInput != NULL)
//        {
//            qreal v = value * 1.0 / 255;
//            m_audioInput->setVolume(v);
//        }

        if (value < (100 * 0.3))
        {
            value = 100 * 0.3;
            ui->horizontalSlider_audioIn->setValue(value);
        }

        if (!mCurrentAudioOutDeviceName.isEmpty())
            SetAudioInputVolume(mCurrentAudioOutDeviceName,value / 100.0);
    }
}

#if defined(Q_OS_WIN32)

///获取和设置设备音量 - Begin
#include <WinSock2.h>
#include <Windows.h>
#include <IPHlpApi.h>
#include <MMSystem.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>//PKEY_Device_FriendlyName
//以下代码来自MSDN
//#define EXIT_ON_ERROR(hres)  \
//              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

float DeviceSettingDialog::GetAudioOutputVolume(QString deviceName)
{
    IMMDeviceEnumerator* pEnumerator;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice *pDevice = NULL;
    IPropertyStore *pProperties=NULL;
    IAudioEndpointVolume *pVolumeAPI=NULL;
    UINT deviceCount = 0;

    float fVolume = -1;

    CoInitializeEx( NULL , COINIT_MULTITHREADED );

    HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
    if (hr != S_OK)
    {
        printf("CoCreateInstance Failed!\n");
        return 0;
    }

    //hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
    hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
    if (hr != S_OK)
    {
        printf("EnumAudioEndpoints Failed!\n");
        goto releasepEnumerator;
    }

    hr = pCollection->GetCount(&deviceCount);
    if (hr != S_OK)
    {
        printf("GetCount Failed!\n");
        goto releasepCollection;
    }

    for (UINT dev=0;dev<deviceCount;dev++)
    {
        pDevice = NULL;
        hr = pCollection->Item(dev,&pDevice);
        if (hr == S_OK)
        {

            hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);
            if (hr == S_OK)
            {
                PROPVARIANT varName;
                PropVariantInit(&varName);
                hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
                if( SUCCEEDED(hr) )
                {

                    if (varName.vt != VT_LPWSTR ||
                            !QString::fromWCharArray(varName.pwszVal).contains(deviceName))
                    {
                        continue;
                    }
//                    if (varName.vt != VT_LPWSTR ||
//                        (wstr2str(varName.pwszVal).find(deviceName) != 0)) //传入的值可能不完整，需要前部分匹配
//                    {
//                        continue;
//                    }
                    hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));
                    if (hr==S_OK)
                    {
                        hr = pVolumeAPI->GetMasterVolumeLevelScalar( &fVolume );
                        if (S_OK == hr) break;
                    }
                }

                SAFE_RELEASE(pProperties);
            }

            SAFE_RELEASE(pDevice);
        }
    }

releasepCollection:
    SAFE_RELEASE(pCollection);
releasepEnumerator:
    SAFE_RELEASE(pEnumerator);
    return  fVolume;
}

int DeviceSettingDialog::SetAudioOutputVolume(QString deviceName, float volumnLevel)
{
    IMMDeviceEnumerator* pEnumerator;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice *pDevice = NULL;
    IPropertyStore *pProperties=NULL;
    IAudioEndpointVolume *pVolumeAPI=NULL;
    UINT deviceCount = 0;

    float fVolume = -1;

    CoInitializeEx( NULL , COINIT_MULTITHREADED );

    HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
    if (hr != S_OK)
    {
        printf("CoCreateInstance Failed!\n");
        return 0;
    }

    //hr = pEnumerator->EnumAudioEndpoints(eCapture /*eRender*/, DEVICE_STATE_ACTIVE, &pCollection);
    hr = pEnumerator->EnumAudioEndpoints( eRender , DEVICE_STATE_ACTIVE , &pCollection );
    if (hr != S_OK)
    {
        printf("EnumAudioEndpoints Failed!\n");
        goto releasepEnumerator;
    }

    hr = pCollection->GetCount(&deviceCount);
    if (hr != S_OK)
    {
        printf("GetCount Failed!\n");
        goto releasepCollection;
    }

    for (UINT dev=0;dev<deviceCount;dev++)
    {
        pDevice = NULL;
        hr = pCollection->Item(dev,&pDevice);
        if (hr == S_OK)
        {

            hr = pDevice->OpenPropertyStore(STGM_READ,&pProperties);
            if (hr == S_OK)
            {
                PROPVARIANT varName;
                PropVariantInit(&varName);
                hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
                if( SUCCEEDED(hr) )
                {

                    if (varName.vt != VT_LPWSTR ||
                            !QString::fromWCharArray(varName.pwszVal).contains(deviceName))
                    {
                        continue;
                    }

                    hr=pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL,NULL,(void **)(&pVolumeAPI));
                    if (hr==S_OK)
                    {
                        hr = pVolumeAPI->SetMasterVolumeLevelScalar( volumnLevel, NULL );
                    }
                }
                SAFE_RELEASE(pProperties);
            }
            SAFE_RELEASE(pDevice);
        }
    }
releasepCollection:
    SAFE_RELEASE(pCollection);
releasepEnumerator:

    SAFE_RELEASE(pEnumerator);
    return  fVolume*100;
}

float DeviceSettingDialog::GetAudioInputVolume(QString deviceName)
{
    float value = 0.0;

    if (m_audioInput != NULL)
        value = m_audioInput->volume();

    return  value;
}

int DeviceSettingDialog::SetAudioInputVolume(QString deviceName, float volumnLevel)
{
    if (m_audioInput != NULL)
        m_audioInput->setVolume(volumnLevel);

    return 100;
}

#elif defined(Q_OS_MAC)

float DeviceSettingDialog::GetAudioOutputVolume(QString deviceName)
{
    float value = 0.0;

//    if (mAudioOutput != NULL)
//        value = mAudioOutput->volume();

    IRtcEngine * lpRtcEngine = CAgoraObject::GetEngine();

    if (Agora_Video_Model::m_agPlayout.checkCreate(lpRtcEngine))
    {
        value = Agora_Video_Model::m_agPlayout.GetVolume() / 255.0;
    }

    return  value;
}

int DeviceSettingDialog::SetAudioOutputVolume(QString deviceName, float volumnLevel)
{

//    if (mAudioOutput != NULL)
//        mAudioOutput->setVolume(volumnLevel);

    IRtcEngine * lpRtcEngine = CAgoraObject::GetEngine();

    if (Agora_Video_Model::m_agPlayout.checkCreate(lpRtcEngine))
    {
        Agora_Video_Model::m_agPlayout.SetVolume(volumnLevel*255);
    }

    return  100;
}


///设置单个设备输入音量
float DeviceSettingDialog::GetAudioInputVolume(QString deviceName)
{
    float value = 0.0;

    IRtcEngine * lpRtcEngine = CAgoraObject::GetEngine();

    if (Agora_Video_Model::m_agAudioin.checkCreate(lpRtcEngine))
    {
        value = Agora_Video_Model::m_agAudioin.GetVolume() / 255.0;
    }

    return value;
}

int DeviceSettingDialog::SetAudioInputVolume(QString deviceName, float volumnLevel)
{
    IRtcEngine * lpRtcEngine = CAgoraObject::GetEngine();

    if (Agora_Video_Model::m_agAudioin.checkCreate(lpRtcEngine))
    {
        Agora_Video_Model::m_agAudioin.SetVolume(volumnLevel*255);
    }
}

#endif

//#include <ShellAPI.h>
//#include <Windows.h>
//#include <shlobj.h>

//#include <mmdeviceapi.h>
//#include <endpointvolume.h>
//#include <audioclient.h>

//bool DevTestDialog::SetVolumeLevel(int level) //设置默认设备的音量-就是右下角看到的
//{
//    HRESULT hr;
//    IMMDeviceEnumerator* pDeviceEnumerator = 0;
//    IMMDevice* pDevice = 0;
//    IAudioEndpointVolume* pAudioEndpointVolume = 0;
//    IAudioClient* pAudioClient = 0;

//    try {
//        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
//        if (FAILED(hr)) throw "CoCreateInstance";
//        hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
//        if (FAILED(hr)) throw "GetDefaultAudioEndpoint";
//        hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndpointVolume);
//        if (FAILED(hr)) throw "pDevice->Active";
//        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
//        if (FAILED(hr)) throw "pDevice->Active";

//        if (level == -2) {
//            hr = pAudioEndpointVolume->SetMute(FALSE, NULL);
//            if (FAILED(hr)) throw "SetMute";
//        }
//        else if (level == -1) {
//            hr = pAudioEndpointVolume->SetMute(TRUE, NULL);
//            if (FAILED(hr)) throw "SetMute";
//        }
//        else {
//            if (level<0 || level>100) {
//                hr = E_INVALIDARG;
//                throw "Invalid Arg";
//            }

//            float fVolume;
//            fVolume = level / 100.0f;
//            hr = pAudioEndpointVolume->SetMasterVolumeLevelScalar(fVolume, &GUID_NULL);
//            if (FAILED(hr)) throw "SetMasterVolumeLevelScalar";

//            pAudioClient->Release();
//            pAudioEndpointVolume->Release();
//            pDevice->Release();
//            pDeviceEnumerator->Release();
//            return true;
//        }
//    }
//    catch (...) {
//        if (pAudioClient) pAudioClient->Release();
//        if (pAudioEndpointVolume) pAudioEndpointVolume->Release();
//        if (pDevice) pDevice->Release();
//        if (pDeviceEnumerator) pDeviceEnumerator->Release();
//        throw;
//    }
//    return false;
//}

//int DevTestDialog::GetVolumeLevel() //获取默认设备的音量-就是右下角看到的
//{
//    float pfLevelDB = 0;

//    HRESULT hr;
//    IMMDeviceEnumerator* pDeviceEnumerator = 0;
//    IMMDevice* pDevice = 0;
//    IAudioEndpointVolume* pAudioEndpointVolume = 0;
//    IAudioClient* pAudioClient = 0;

//    try {
//        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
//        if (FAILED(hr)) throw "CoCreateInstance";
//        hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
//        if (FAILED(hr)) throw "GetDefaultAudioEndpoint";
//        hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndpointVolume);
//        if (FAILED(hr)) throw "pDevice->Active";
//        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
//        if (FAILED(hr)) throw "pDevice->Active";

//        hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&pfLevelDB);
//        if (FAILED(hr)) throw "GetMasterVolumeLevelScalar";

//        pfLevelDB *= 255; //最大值255

//        pAudioClient->Release();
//        pAudioEndpointVolume->Release();
//        pDevice->Release();
//        pDeviceEnumerator->Release();
//        return pfLevelDB;

//    }
//    catch (...) {
//        if (pAudioClient) pAudioClient->Release();
//        if (pAudioEndpointVolume) pAudioEndpointVolume->Release();
//        if (pDevice) pDevice->Release();
//        if (pDeviceEnumerator) pDeviceEnumerator->Release();
//        throw;
//    }
//    return pfLevelDB;
//}

///获取和设置设备音量 - End
