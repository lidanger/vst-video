#ifndef UTIL_H
#define UTIL_H

#include <QMetaType>
#include <QString>

class Util
{
public:
    Util();
};

// 写日志
void log(const QString &logFile, const QByteArray &data);

// 判断文件是不是存在
bool isFileExists(QString fullFileName);

// 判断文件夹是否存在,不存在则创建，默认假
bool isDirExist(QString fullPath, bool mk = false);

// 调用默认程序打开命令行
void open(QString url);

// 相对路径转绝对路径
QString put_url(QString path, QString url);

// 本地编码转换为 Unicode
QString toUnicode(QString text);

// 取文本 MD5 值
QString toHash(const QString pwd);

// 读文件中的文本
QString Readfile(const QString pfile);

// 写文本数据到文件
bool Writefile(const QString pfile, QString data);

#include <X11/keysym.h>

// 发送按键
int x11SendKey(unsigned long key = XK_Control_L);
// 鼠标单击
int x11MouseClick(int x, int y);
// 鼠标移动
int x11MmouseMove(int x, int y);

#endif // UTIL_H
