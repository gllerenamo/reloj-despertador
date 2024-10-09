#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateTime();
    void on_pushButtonAnadirAlarma_clicked();
    void on_pushButtonEliminarAlarma_clicked();
    void checkAlarms(const QTime &currentTime);
    void on_pushButtonEditarAlarma_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *cronometroTimer;
    QVector<bool> alarmSounded;
};
#endif // MAINWINDOW_H
