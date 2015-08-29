#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = NULL);
    ~MainWindow();

protected:
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void showFindAndReplaceWidget();
    void showFindInFilesWidget();
    void setWordWrapMode(bool enabled);
    void openTerminal();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
