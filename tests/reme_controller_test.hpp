#include <QObject>
#include <QtTest>
#include <QtCore>

#include <iostream>
 
class reme_controller_test: public QObject
{
    Q_OBJECT
private slots:
    void my_test();
};
 
void reme_controller_test::my_test()
{
  QString str = "Hello";
  QVERIFY(str.toUpper() == "HELLO");
}
 
//QTEST_MAIN(reme_controller_test)