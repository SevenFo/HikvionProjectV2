import QtQuick
import QtQuick.Window 2.15
import QtQuick.Controls 2.12

Rectangle{
    id : p;
    radius: 10;
    width: 200;
    height: 40;
    color: Qt.rgba(1,1,1,0.8)
    property string name: "name";
    property string inputDefault: "input..."
    signal editedFinished(string d);


        Label{
            padding: p.radius;
            anchors.verticalCenter: parent.verticalCenter;

            id: label;
            text: name;
            font.pixelSize: 15;

        }

        TextInput{
            padding: 5;
            anchors.verticalCenter: parent.verticalCenter;

            anchors.left: label.right;

            width: 100;
            id:input;
            text: inputDefault;
            font.pixelSize: 15;
            onEditingFinished: p.editedFinished(input.text);
        }



}
