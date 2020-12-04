#ifndef SHOTCUTSETTINGS_H
#define SHOTCUTSETTINGS_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

class ShortcutSettings : public QDialog
{
    Q_OBJECT

public:
    ShortcutSettings(QWidget *parent = nullptr);

private:
    void _init();
    void _init_data();

private slots:
    void _save();

private:
    QListWidget *_shortcuts;
    QPushButton *_ok;
    QPushButton *_cancel;
};

#endif // SHOTCUTSETTINGS_H
