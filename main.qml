import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import siky.ImageViewer 1.0
import siky.HikvisonViewer 1.0



Rectangle {
    id: mainWindow
    width: 1920
    height: 1080
    visible: true
    color: Qt.rgba(0.94,0.94,0.94,1)
    radius: 30
    /*
 *  1. 所有的大方格都要有 margins 10， 即每个大块之间间隔20px
 *  2. 内部元素拥有 padding 5，即每个元素之间间隔5px
 */
    Rectangle{
        color: Qt.rgba(0.94,0.94,0.94,1)
        anchors.fill: parent;
        Row{
            padding: 20;
            spacing: 20;
            Column{
                padding: 10
                spacing: 10
                Text {
                    id: none
                    font.pixelSize: 30
                    font.bold: true
                    text: qsTr("用户管理")
                }
                Column{
                    id: displayAndLogin
                    spacing: 20; //上下元素间隔20
                    width: 1000;
                    Row{
                        id: loginInfoRow
                        spacing: 10;

                        LabeledTextInput{
                            id: inputTextIPAddr
                            name: "IP地址";
                            inputDefault: "172.20.21.88";
                            width: 210;
                            onEditedFinished: (d)=>{
                                                  hikvison.host = d;
                                              }

                        }
                        LabeledTextInput{
                            id: inputTextPort
                            width: 210;
                            name: "端口";
                            inputDefault: "6666";
                            onEditedFinished: (d)=>{
                                                  hikvison.port = d;
                                              }
                        }

                        LabeledTextInput{
                            id: inputTextUserID
                            width: 210;
                            name: "账号";
                            inputDefault: "admin";
                            onEditedFinished: (d)=>{
                                                  hikvison.userid = d;
                                              }
                        }

                        LabeledTextInput{
                            id: inputTextPassword
                            width: 210;
                            name: "密码";
                            inputDefault: "hkvs123456";
                            onEditedFinished: (d)=>{
                                                  hikvison.password = d;
                                              }
                        }
                        Button{
                            id: buttonSetupCamera;
                            height: 40
                            Text {

                                anchors.centerIn: parent
                                id: buttonSetupCameraText
                                text: qsTr("登入")
                            }
                            width: 120
                            onClicked: {
                                hikvison.loginAndSetup();
                            }

                            background: Rectangle{
                                id: buttonSetupCameraBackground
                                color: buttonSetupCamera.down ? "#d0d0d0" : Qt.rgba(1.0,1.0,1.0,0.8)

                                radius:10;
                            }
                        }

                    }

                    Row{
                        leftPadding: 30
                        rightPadding: 30
                        HikvisonViewer{

                            id:hikvison;
                            host: inputTextIPAddr.inputDefault;
                            port: inputTextPort.inputDefault;
                            userid: inputTextUserID.inputDefault;
                            password: inputTextPassword.inputDefault;
                            width: 1920/2;
                            height: 1080/2;


                        }
                    }


                    Row{
                        ImageViewer{
                            objectName: "snapfaceimage"
                            id: snapedFaceViewer
                        }
                    }
                }
            }
            Column{
                padding: 10
                spacing: 10
                Text {
                    font.pixelSize: 30
                    font.bold: true
                    id: textControlPannelName
                    text: qsTr("控制面板")
                }
                Rectangle
                {
                    width: 300;
                    height: 300;
                    radius: 10;
                    color: Qt.rgba(1,1,1,0.8)
                    Column{
                        padding: 20
                        id: columnControlPanel
                        enabled: true
                        spacing: 10;

                        Row{
                            Switch{
                                id: switchStartDecode;
                                text: "视频已关闭";
                                enabled: false;
                                onClicked: hikvison.startDecode();
                            }
                        }
                        Row{
                            Switch{
                                id: switchModeClassificaton;
                                text: "表情识别已关闭";
                                enabled: false;
                                //                    onClicked: hikvison.startDecode();
                            }
                        }
                        Row{
                            Switch{
                                id: switchTiredJudger;
                                text: "疲劳判断已关闭";
                                enabled: false;
                                //                    onClicked: hikvison.startDecode();
                            }
                        }
                        Row{
                            Switch{
                                id: switchAudioClassification;
                                text: "音频检测已关闭";
                                enabled: false;
                                //                    onClicked: hikvison.startDecode();
                            }
                        }
                    }
                }
            }
        }

    }


    Connections{
        target: hikvison
       function onGetSnapedImage(pixmap,width,height){
           snapedFaceViewer.pixmap = pixmap;
           snapedFaceViewer.width = width;
           snapedFaceViewer.height = height
                          }

       function onLoginStateChanged(){
                                 if(hikvison.is_login === true)
                                 {
                                     buttonSetupCameraText.text="退出";
                                     switchStartDecode.enabled = true;
                                     buttonSetupCameraBackground.color=Qt.rgba(0.8,0.0,0.0,0.5)
                                     buttonSetupCameraText.color = Qt.rgba(1.0,1.0,1.0,1.0)


                                 }
                                 else{
                                     buttonSetupCameraText.text="登入";
                                     switchStartDecode.enabled = true
                                     buttonSetupCameraText.color = "#000000"
                                     buttonSetupCameraText.color= buttonSetupCamera.down ? "#d0d0d0" : Qt.rgba(1.0,1.0,1.0,0.8)

                                 }
                             }
        function onOpenVideoStateChange(){
                                     if(hikvison.is_open_video === true)
                                     {
                                         switchStartDecode.text = "视频已开启";
                                         switchModeClassificaton.enabled = true;
                                         switchAudioClassification.enabled = true;
                                         switchTiredJudger.enabled = true;
                                     }
                                     else
                                     {
                                         switchStartDecode.text = "视频已关闭";
                                         switchModeClassificaton.enabled = false;
                                         switchAudioClassification.enabled = false;
                                         switchTiredJudger.enabled = false;
                                     }
                                 }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/
