import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Simulog")

    property var initialDatabase: ({ database: [] })

    Component.onCompleted: {
        console.log("Načteno bloků při startu:", initialDatabase.database.length)
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