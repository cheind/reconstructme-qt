#include <QObject>
#include <QtTest>
#include <QtCore>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <iostream>
 
class webservice: public QObject
{
    Q_OBJECT
private slots:
    void request();
    void reply(QNetworkReply*);
};


// paramters of the post are:
// 
// token : your sketchfab API token
// fileModel : the model you want to upload
// title : model title
// description (optional) : model description
// tags (optional) : list of tags separated by space
// private (optional) : if set to True, then the model is private
// password (optional) : if private is set to True, you can add a password to protect your file
// returns :
// 
// {success: true, {result: {id: 'xxxxxx'} } when upload OK
// {success: false, error: 'error message'} when upload error

void webservice::request()
{
  // 1. Variant: URL with parameters
  QUrl url("https://api.sketchfab.com/v1/models");
  url.addQueryItem("token", "???");
  url.addQueryItem("fileModel", "test.ply");
  url.addQueryItem("title", "MyDesk");
  //url.addQueryItem("description", "This is a test");
  //url.addQueryItem("tags", "");
  //url.addQueryItem("private", "1");
  //url.addQueryItem("password", "mypassword");
  
  QNetworkRequest req(url);
  
  // 2. Variant: Data as json
  QUrl url2("https://api.sketchfab.com/v1/models");
  QByteArray data;
  data.append("{");
  data.append(  "token:???,");
  data.append(  "fileModel:test.ply,");
  data.append(  "title:MyDesk,");
  data.append("}");
  
  QNetworkRequest req2(url2);
  req.setRawHeader("Content-Type", "application/json");
  
  // POST message
  QNetworkAccessManager* manager = new QNetworkAccessManager(this);
  //QNetworkReply *reply = manager->post(req, url.encodedQuery()); // 1. Variant
  QNetworkReply *reply = manager->post(req2, data); // 2. Variant
  
  // wait for reply
  reply->waitForReadyRead(10000);
  std::cout << reply->readAll().constData() << std::endl;
}

void webservice::reply(QNetworkReply* reply) 
{
  std::cout << "a" << std::endl;
  std::cout << reply->readAll().constData() << std::endl;
}

QTEST_MAIN(webservice)