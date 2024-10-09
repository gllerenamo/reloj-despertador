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
    void on_alarma_button_clicked();
    void on_temp_button_clicked();
    void on_crono_button_clicked();

private:
    Ui::Principal *ui;
};
#endif // PRINCIPAL_H
