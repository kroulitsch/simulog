import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1100
    height: 720
    visible: true
    title: "Simulog"
    color: "#1e1f22"

    property var database: []
    property var activeCircuit: null

    Component.onCompleted: {
        retryTimer.start()
    }

    Timer {
        id: retryTimer
        interval: 100
        repeat: true
        onTriggered: {
            root.sendRequest("GET", "/database", null, function(ok, data) {
                if (ok && data && data.database) {
                    retryTimer.stop()
                    root.database = data.database

                    root.sendRequest("POST", "/circuit/create", {
                        name: "NEW_CIRCUIT",
                        inputs: ["A", "B"],
                        outputs: ["Y"]
                    }, function(okCreate, circuit) {
                        if (okCreate) root.activeCircuit = circuit
                    })
                }
            })
        }
    }

    Rectangle {
        id: sidebar
        width: 220
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#252629"

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Text {
                text: "Inventory (" + root.database.length + ")"
                color: "#ffffff"
                font.bold: true
                font.pixelSize: 15
            }

            ListView {
                width: parent.width
                height: parent.height - 30
                spacing: 8
                clip: true
                model: root.database

                delegate: Button {
                    width: ListView.view.width
                    text: modelData.name + " (" + modelData.inputs.length + " -> " + modelData.outputs.length + ")"
                    onClicked: {
                        if (!root.activeCircuit) return;
                        var instName = modelData.name.toLowerCase() + "_" + (root.activeCircuit.components.length + 1);
                        root.sendRequest("POST", "/circuit/component", {
                            type: modelData.name,
                            name: instName
                        }, function(ok, circuit) {
                            if (ok) root.activeCircuit = circuit;
                        });
                    }
                }
            }
        }
    }

    Item {
        id: workspace
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        clip: true

        Text {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 16
            text: root.activeCircuit
                  ? ("Obvod: " + root.activeCircuit.name + " | Components: " + root.activeCircuit.components.length)
                  : "No active circuit"
            color: "#9aa0a6"
            font.pixelSize: 14
        }

        Repeater {
            model: root.activeCircuit ? root.activeCircuit.components : []

            Block {
                x: 80 + (index % 4) * 180
                y: 80 + Math.floor(index / 4) * 140
                blockData: modelData

                onPinClicked: function(compName, pinName, isOutput, scenePos) {
                    console.log("Pin:", compName + "." + pinName, "isOutput:", isOutput, "pos:", scenePos);
                }
            }
        }
    }

    function loadStartup() {
        sendRequest("GET", "/database", null, function(ok, data, status) {
            if (ok && data && data.database) {
                root.database = data.database;
                sendRequest("POST", "/circuit/create", {
                    name: "NEW_CIRCUIT",
                    inputs: ["A", "B"],
                    outputs: ["Y"]
                }, function(okCreate, circuit) {
                    if (okCreate) root.activeCircuit = circuit;
                });
            } else {
                console.warn("Error loading the fatabase:", status);
            }
        });
    }

    function sendRequest(method, endpoint, payload, callback, contentType) {
        var xhr = new XMLHttpRequest();
        xhr.open(method, "http://localhost:8080" + endpoint, true);

        if (payload !== null && payload !== undefined) {
            xhr.setRequestHeader("Content-Type", contentType || "application/json");
        }

        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                var data = null;
                try {
                    data = xhr.responseText ? JSON.parse(xhr.responseText) : null;
                } catch (e) {
                    data = xhr.responseText;
                }
                if (callback) {
                    callback(xhr.status >= 200 && xhr.status < 300, data, xhr.status);
                }
            }
        };

        if (payload === null || payload === undefined) {
            xhr.send();
        } else if (typeof payload === "string") {
            xhr.send(payload);
        } else {
            xhr.send(JSON.stringify(payload));
        }
    }
}