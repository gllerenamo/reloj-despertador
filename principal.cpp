#include "principal.h"
#include "./ui_principal.h"

Principal::Principal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Principal)
{
    ui->setupUi(this);
    connect(ui->alarma_button, &QPushButton::clicked, this, &Principal::on_alarma_button_clicked);
    connect(ui->cronometro_button, &QPushButton::clicked, this, &Principal::on_crono_button_clicked);
    connect(ui->temporizador_button, &QPushButton::clicked, this, &Principal::on_temp_button_clicked);
}

Principal::~Principal()
{
    delete ui;
}

void Principal::on_alarma_button_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page);
}

void Principal::on_crono_button_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void Principal::on_temp_button_clicked() {

}
