import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import siky.ImageViewer 1.0
import siky.HikvisonViewer 1.0


Rectangle {

    width: 640
    height: 480
    visible: true
    color: Qt.rgba(0.9,0.9,0.9,1)

    Rectangle{
        color:  Qt.rgba(0.9,0.9,0.9,1);
        anchors.fill: parent;

        Column{
            anchors.margins: 10;
            anchors.top:parent.top;
            anchors.left: parent.left;
            spacing: 10;
            Row{

                anchors.margins: 10;
                spacing: 10;

                LabeledTextInput{
                    id: inputTextIPAddr
                    name: "IP地址";
                    inputDefault: "172.20.21.88";
                    onEditedFinished: (d)=>{
                                            hikvison.host = d;
                                      }

                }
                LabeledTextInput{
                    id: inputTextPort
                    name: "端口";
                    inputDefault: "6666";
                    onEditedFinished: (d)=>{
                                            hikvison.port = d;
                                      }
                }

            }

            Row{

                anchors.margins: 10;
                spacing: 10;

                LabeledTextInput{
                    id: inputTextUserID
                    name: "账号";
                    inputDefault: "admin";
                    onEditedFinished: (d)=>{
                                            hikvison.userid = d;
                                      }
                }
                LabeledTextInput{
                    id: inputTextPassword
                    name: "密码";
                    inputDefault: "hkvs123456";
                    onEditedFinished: (d)=>{
                                            hikvison.password = d;
                                      }
                }

            }
            Row{
                Button{
                    id: buttonSetupCamera;
                    text: "登入用户";
                    onClicked: hikvison.loginAndSetup();

                }
                Button{
                    id: buttonStartDecode;
                    text: "获取数据";
                    onClicked: hikvison.startDecode();

                }

            }
            HikvisonViewer
            {
                id:hikvison
                width: 1920/2;
                height: 1080/2;
            }
            ImageViewer{
                id: img;
                width: 1920/4;
                height: 1080/4;
            }
        }


    }


}
