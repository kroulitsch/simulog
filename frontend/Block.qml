import QtQuick

Rectangle {
    id: blockRoot

    property var blockData: ({ name: "BLOCK", inputs: [], outputs: [] })
    property bool draggable: true

    signal pinClicked(string compName, string pinName, bool isOutput, point scenePos)

    readonly property int pinSpacing: 24
    readonly property int maxPins: Math.max(
        blockData && blockData.inputs ? blockData.inputs.length : 0,
        blockData && blockData.outputs ? blockData.outputs.length : 0,
        1
    )

    width: 145
    height: Math.max(60, maxPins * pinSpacing + 24)
    radius: 8
    color: "#2b2d30"
    border.color: "#4e5257"
    border.width: 2

    DragHandler {
        enabled: blockRoot.draggable
        target: blockRoot
    }

    Column {
        anchors.centerIn: parent
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: blockData.name || ""
            color: "#ffffff"
            font.bold: true
            font.pixelSize: 13
        }

        Text {
            visible: blockData.type !== undefined && blockData.type !== blockData.name
            anchors.horizontalCenter: parent.horizontalCenter
            text: "(" + (blockData.type || "") + ")"
            color: "#9aa0a6"
            font.pixelSize: 10
        }
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: -6
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Repeater {
            model: blockData.inputs || []

            Row {
                spacing: 6
                height: 16

                Rectangle {
                    id: inDot
                    width: 12
                    height: 12
                    radius: 6
                    anchors.verticalCenter: parent.verticalCenter
                    color: modelData.on ? "#4caf50" : "#1e1f22"
                    border.color: "#a9b7c6"
                    border.width: 1.5

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -4
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var center = inDot.mapToItem(blockRoot.parent, inDot.width / 2, inDot.height / 2)
                            blockRoot.pinClicked(blockData.name, modelData.name, false, center)
                        }
                    }
                }

                Text {
                    text: modelData.name
                    color: "#d0d0d0"
                    font.pixelSize: 11
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.rightMargin: -6
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Repeater {
            model: blockData.outputs || []

            Row {
                anchors.right: parent.right
                spacing: 6
                height: 16

                Text {
                    text: modelData.name
                    color: "#d0d0d0"
                    font.pixelSize: 11
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    id: outDot
                    width: 12
                    height: 12
                    radius: 6
                    anchors.verticalCenter: parent.verticalCenter
                    color: modelData.on ? "#4caf50" : "#1e1f22"
                    border.color: "#a9b7c6"
                    border.width: 1.5

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -4
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var center = outDot.mapToItem(blockRoot.parent, outDot.width / 2, outDot.height / 2)
                            blockRoot.pinClicked(blockData.name, modelData.name, true, center)
                        }
                    }
                }
            }
        }
    }
}