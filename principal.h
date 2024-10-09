#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Principal;
}
QT_END_NAMESPACE

class Principal : public QMainWindow
{
    Q_OBJECT

public:
    Principal(QWidget *parent = nullptr);
    ~Principal();

private slots:
    //Cambio de vista
    void on_alarma_button_clicked();
    void on_temp_button_clicked();
    void on_crono_button_clicked();

    //alarma
    void updateTime();
    void on_add_button_clicked();
    void on_delete_button_clicked();
    void on_edit_button_clicked();
    void checkAlarms(const QTime &currentTime);

    //reloj

    //cronometro

private:
    Ui::Principal *ui;
    QTimer *cronometroTimer;
    QVector<bool> alarmSounded;

};
#endif // PRINCIPAL_H
