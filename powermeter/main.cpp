#include <QCoreApplication>
#include <QDebug>
#include <QtSerialPort>
#include <QList>
#include <QtNetwork>
#include <QUrl>
#include <QUrlQuery>
#include <windows.h>

QByteArray readSerial();
float getDatafromAddr(QByteArray data, uint16_t addr);
void sendServer(QByteArray bytearr);

const char port[]="/dev/ttyUSB0";

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //Liet ke nhung port dang co
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << info.portName() << ":"  << info.description();
    }

    //Vong lap loop
    while(1){
        //Doc du lieu tu Serial
        QByteArray serialData = readSerial();
        //Kiem tra neu co du lieu thi gui len Server
        if( serialData != NULL )
        {
            //qDebug() << serialData.toHex();

            sendServer(serialData);
        }
        //Ngu 3s
        Sleep(3000);
    }

    return a.exec();
}

QByteArray readSerial()
{
    QSerialPort *Serial = new QSerialPort(port);
    qDebug() << "\n\nOpen" << port << "...";
    if(Serial->open(QIODevice::ReadWrite)){
        qDebug() << "success";
        Serial->setBaudRate(QSerialPort::Baud9600);
        Serial->setParity(QSerialPort::NoParity);
        Serial->setStopBits(QSerialPort::OneStop);
        Serial->setFlowControl(QSerialPort::NoFlowControl);
        Serial->setDataBits(QSerialPort::Data8);
        Serial->setReadBufferSize(255);

        qDebug() << "Write...";
        const char arr[8] = {0x01,0x03,0x0f,0x3c,0x00,0x64,0x87,0x39};
        Serial->write(arr,8);
        Serial->waitForBytesWritten(-1);

        qDebug() << "Wait...";
        Serial->waitForReadyRead(2000);
        QByteArray ba = Serial->readAll();
        while(ba.length() < 205)
        {
            Serial->waitForReadyRead(2000);
            QByteArray ap = Serial->readAll();
            if(ap.isEmpty()) break;
            ba.append(ap);
        }

        qDebug() << "ReadedBytes = " << QString::number(ba.length());
        if(ba.length() < 205)
        {
            Serial->close();
            qDebug() << "Close" << port ;
            qDebug() << "Return NULL";
            return NULL;

        }

        Serial->close();
        qDebug() << "Close" << port ;
        qDebug() << "Return Success";
        return ba;
    }
    qDebug() << "error";
    return NULL;

}

float getDatafromAddr(QByteArray data, uint16_t addr)
{
    //uint16_t addr = 3915;
    uint16_t id = (addr-3901)*2 + 3;

    uint8_t n0 = data[id+2];
    uint8_t n1 = data[id+3];
    uint8_t n2 = data[id];
    uint8_t n3 = data[id+1];
    uint32_t num32 = n0 << 24 | n1 << 16 | n2 << 8 | n3;

    float numf = *((float*)&num32);

    //qDebug() << QString::number(num32,16);
    //qDebug() << QString::number(numf);
    //return  *((float*)&num32);
    return ((uint32_t)(numf*10.0))/10.0;
}

void sendServer(QByteArray bytearr)
{
    //Khai bao domain
    //QNetworkRequest request(QUrl( QString("http://ip.jsontest.com/") ));
    QNetworkRequest request(QUrl( QString("http://datnnam.000webhostapp.com/hau.php") ));
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    /*QJsonObject json;
    json.insert("tag", "piup");
    json.insert("VLN", getDatafromAddr(bytearr, 3911));
    json.insert("A", getDatafromAddr(bytearr, 3913));
    json.insert("Wh", getDatafromAddr(bytearr, 3961));
    json.insert("F", getDatafromAddr(bytearr, 3915));

    json.insert("tag", "espup");
    json.insert("email", "datn@gmail.com");
    json.insert("kwh1", getDatafromAddr(bytearr, 3911));
    json.insert("kwh2", getDatafromAddr(bytearr, 3913));
    json.insert("kwh3", getDatafromAddr(bytearr, 3915));
    json.insert("kwh4", getDatafromAddr(bytearr, 3961));
    qDebug() << "Json String\n" << QJsonDocument(json).toJson();
    */

    //Chen du lieu vao chuoi, dinh dang Json vao query
    QUrlQuery query;
    //DInh dang Tendulieu, du lieu
    query.addQueryItem("tag", "espup");
    query.addQueryItem("email", "datn@gmail.com");
    query.addQueryItem("kwh1", QString::number(getDatafromAddr(bytearr, 3911)));
    query.addQueryItem("kwh2", QString::number(getDatafromAddr(bytearr, 3913)));
    query.addQueryItem("kwh3", QString::number(getDatafromAddr(bytearr, 3915)));
    query.addQueryItem("kwh4", QString::number(getDatafromAddr(bytearr, 3961)));
    qDebug() << "Query String\n" << query.query(QUrl::FullyEncoded).toUtf8();

    //Khai bao doi tuong quan ly mang
    QNetworkAccessManager mgr;
    qDebug() << "Post to Server...";
    //QNetworkReply *reply = mgr.post(request, QJsonDocument(json).toJson());
    //Gui di bang POST
    QNetworkReply *reply = mgr.post(request, query.query(QUrl::FullyEncoded).toUtf8());

    //Cho den khi hoan thanh
    while(!reply->isFinished()) qApp->processEvents();//Van cho cac tien trinh khac hoat dong bthuong

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "success";
        qDebug() << reply->readAll();
    }
    else {
        qDebug() << "error";
        qDebug() << reply->readAll();
    }
    reply->deleteLater();
}
