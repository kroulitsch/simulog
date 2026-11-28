#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QDir>
#include <QUrl>
#include <QDebug>

struct ApiResponse {
    int statusCode = 0;
    QByteArray body;
    bool ok = false;
};

ApiResponse sendRequest(QNetworkAccessManager &net,
                        const QString &method,
                        const QString &endpoint,
                        const QByteArray &body = QByteArray(),
                        const QString &contentType = "application/json",
                        int timeoutMs = 1500)
{
    QNetworkRequest request(QUrl("http://localhost:8080" + endpoint));
    if (!body.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }

    QNetworkReply *reply = nullptr;
    if (method == "GET") {
        reply = net.get(request);
    } else if (method == "POST") {
        reply = net.post(request, body);
    } else {
        reply = net.sendCustomRequest(request, method.toUtf8(), body);
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(timeoutMs);
    loop.exec();

    ApiResponse res;
    if (timer.isActive()) {
        timer.stop();
        res.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        res.body = reply->readAll();
        res.ok = (reply->error() == QNetworkReply::NoError && res.statusCode >= 200 && res.statusCode < 300);
    } else {
        reply->abort();
    }

    reply->deleteLater();
    return res;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QProcess serverProcess;
    QNetworkAccessManager netManager;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString serverBin = appDir.absoluteFilePath("../../backend/api_app");

    serverProcess.setWorkingDirectory(appDir.absoluteFilePath("../.."));
    serverProcess.setProcessChannelMode(QProcess::MergedChannels);

    QObject::connect(&serverProcess, &QProcess::finished, [](int exitCode, QProcess::ExitStatus status) { });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        if (serverProcess.state() == QProcess::Running) {
            sendRequest(netManager, "POST", "/stop");
            if (!serverProcess.waitForFinished(1500)) {
                serverProcess.terminate();
                serverProcess.waitForFinished(500);
            }
        }
    });

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/Simulog/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    bool initialized = false;
    QObject::connect(&serverProcess, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray out = serverProcess.readAllStandardOutput();
        qDebug().noquote() << out.trimmed();
    });

    serverProcess.start(serverBin);
    if (!serverProcess.waitForStarted(3000)) {
        qWarning() << "Could not start backend server at:" << serverBin
                   << "Error:" << serverProcess.errorString();
    }

    return app.exec();
}