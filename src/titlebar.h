#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>

class TitleBar : public QFrame
{
    Q_OBJECT

public:
    TitleBar(QMainWindow* parent = nullptr);

public:
    void setWindowTitle(const QString &title);
    void setWindowIcon(const QIcon &icon);
    void setMainMenu(QMenu *menu);
    void addWidget(QWidget *widget);
    void setAboutDialog(QDialog *dialog);
    void setWidgetStart(int pos);

    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void _show_main_menu();
    void _show_context_menu(const QPoint &pos);
    void _bring_window_to_top();
    void _show_window_max_normal();

    void _select_theme(QAction *action);

    void _show_about();

private:
    void _init_control();
    void _init_layout();

    void _createMainMenu(QMenu *insertAtStart = nullptr);

private:
    QPushButton *_icon = nullptr;
    QLabel *_title = nullptr;
    QWidget *_content = nullptr;
    QPushButton *_close = nullptr;
    QPushButton *_min = nullptr;
    QPushButton *_max = nullptr;
    QPushButton *_menu = nullptr;

    QMenu *_mainMenu = nullptr;
    QMenu *_contextMenu = nullptr;

    QDialog *_about = nullptr;

    QMainWindow *_window = nullptr;
};

#endif // TITLEBAR_H
