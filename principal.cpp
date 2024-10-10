#include "principal.h"
#include "./ui_principal.h"

#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QTimeEdit>
#include <QDialogButtonBox>

Principal::Principal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Principal)
{
    ui->setupUi(this);
    ui->alarmas_table->setColumnWidth(1, 300);
    ui->crono_table->setColumnWidth(0, 137);
    ui->crono_table->setColumnWidth(1, 137);
    ui->crono_table->setColumnWidth(2, 137);

    //Vista principal
    connect(ui->alarma_button, &QPushButton::clicked, this, &Principal::on_alarma_button_clicked);
    connect(ui->cronometro_button, &QPushButton::clicked, this, &Principal::on_crono_button_clicked);

    //Alarma
    alarmaTimer = new QTimer(this);
    connect(alarmaTimer, &QTimer::timeout, this, &Principal::updateTime);
    alarmaTimer->start(1000); //actualiza cada segundo
    alarmSounded.resize(0);

    //Cronometro
    cronometroTimer = new QTimer(this);
    connect(cronometroTimer, &QTimer::timeout, this, &Principal::updateCrono);

    //conectar botones
    connect(ui->add_button, &QPushButton::clicked, this, &Principal::on_add_button_clicked);
    connect(ui->delete_button, &QPushButton::clicked, this, &Principal::on_delete_button_clicked);
    connect(ui->edit_button, &QPushButton::clicked, this, &Principal::on_edit_button_clicked);

    connect(ui->start_button, &QPushButton::clicked, this, &Principal::startCrono);
    connect(ui->marca_button, &QPushButton::clicked, this, &Principal::saveTime);
    connect(ui->reiniciar_button, &QPushButton::clicked, this, &Principal::restartCrono);
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

void Principal::startCrono() {
    if (!cronometroTimer->isActive()) {
        timeElapsed = 0;
        lastTime = QTime(0,0);
        cronometroTimer->start(1000);
    }
}

void Principal::updateCrono() {
    timeElapsed++;
    QTime time(0, 0);
    time = time.addSecs(timeElapsed);

    ui->hora_cronometro->setText(time.toString("hh:mm:ss"));
}

void Principal::restartCrono() {
    cronometroTimer->stop();
    QTime time(0, 0);
    ui->hora_cronometro->setText(time.toString("hh:mm:ss"));
}

void Principal::saveTime() {
    QTime current(0, 0);
    current = current.addSecs(timeElapsed);
    int diffSeconds = lastTime.secsTo(current);

    QTime diffTime(0, 0);
    diffTime = diffTime.addSecs(diffSeconds);
    lastTime = current;

    int rowCount = ui->crono_table->rowCount();
    ui->crono_table->insertRow(rowCount);
    ui->crono_table->setItem(rowCount, 0, new QTableWidgetItem(QString::number(rowCount)));
    ui->crono_table->setItem(rowCount, 1, new QTableWidgetItem(diffTime.toString("hh:mm:ss")));
    ui->crono_table->setItem(rowCount, 2, new QTableWidgetItem(current.toString("hh:mm:ss")));
}

void Principal::updateTime() {
    QTime currentTime = QTime::currentTime();
    ui->hora_label->setText(currentTime.toString("HH:mm:ss"));
    checkAlarms(currentTime);
}

void Principal::checkAlarms(const QTime &currentTime) {
    for (int row = 0; row < ui->alarmas_table->rowCount(); ++row) {
        QString horaAlarma = ui->alarmas_table->item(row, 0)->text();
        QTime alarmTime = QTime::fromString(horaAlarma, "HH:mm");
        //asegurarse de que la alarma sea valida y mostrar mensaje de alarma solo si no ha sonado antes
        if (alarmTime.isValid() && currentTime >= alarmTime) {
            if (!alarmSounded[row]) {
                QMessageBox::information(this,
                    tr("Alarma"), tr("¡Alarma sonando! \n") + ui->alarmas_table->item(row, 1)->text());
                alarmSounded[row] = true; //marcar como sonada
            }
        }
    }
}

void Principal::on_add_button_clicked() {
    ui->add_button->setEnabled(false);
    //desconectar cualquier conexion existente para evitar duplicados
    disconnect(ui->add_button, &QPushButton::clicked, this, &Principal::on_add_button_clicked);
    //crear un dialogo personalizado para seleccionar la hora
    QDialog dialog(this);
    dialog.setWindowTitle("Seleccionar Hora de la Alarma");
    QVBoxLayout layout(&dialog);
    //crear un widget QTimeEdit y añadirlo
    QTimeEdit timeEdit(&dialog);
    timeEdit.setDisplayFormat("HH:mm");
    layout.addWidget(&timeEdit);
    //añadir un boton de ok y cancelar para confirmar la hora
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout.addWidget(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    //mostrar el dialogo y esperar a que el usuario seleccione una hora
    if (dialog.exec() == QDialog::Accepted) {
        QTime horaAlarma = timeEdit.time();
        QString horaAlarmaStr = horaAlarma.toString("HH:mm");
        QTime currentTime = QTime::currentTime();
        //verificar si la hora de la alarma ya paso y ajustar la hora de la alarma para el siguiente dia
        if (horaAlarma < currentTime) {
            horaAlarma = horaAlarma.addSecs(24 * 3600);
            horaAlarmaStr = horaAlarma.toString("HH:mm");
            QMessageBox::information(this, tr("Hora Ajustada"), tr("La hora seleccionada ya ha pasado. La alarma se ha ajustado para el siguiente día: ") + horaAlarmaStr);
        }
        //pedir la descripcion de la alarma
        bool ok;
        QString descripcion = QInputDialog::getText(this, tr("Añadir Alarma"), tr("Descripción:"), QLineEdit::Normal, "", &ok);
        if (ok && !descripcion.isEmpty()) {
            //añadir la alarma a la tabla
            int rowCount = ui->alarmas_table->rowCount();
            ui->alarmas_table->insertRow(rowCount);
            ui->alarmas_table->setItem(rowCount, 0, new QTableWidgetItem(horaAlarmaStr));
            ui->alarmas_table->setItem(rowCount, 1, new QTableWidgetItem(descripcion));
            alarmSounded.push_back(false);
        }
    }
    //volver a habilitar el boton despues de la operacion
    ui->add_button->setEnabled(true);
    connect(ui->add_button, &QPushButton::clicked, this, &Principal::on_add_button_clicked);
}

void Principal::on_delete_button_clicked() {
    //deshabilitar el boton mientras se ejecuta la operacion
    ui->delete_button->setEnabled(false);
    disconnect(ui->delete_button, &QPushButton::clicked, this, &Principal::on_delete_button_clicked);
    //crear un dialogo de confirmacion
    QMessageBox confirmationBox(this);
    confirmationBox.setWindowTitle(tr("Confirmar Eliminación"));
    confirmationBox.setText(tr("¿Está seguro de que desea eliminar la alarma seleccionada?"));
    confirmationBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmationBox.setDefaultButton(QMessageBox::No);

    if (confirmationBox.exec() == QMessageBox::Yes) {
        QItemSelectionModel *select = ui->alarmas_table->selectionModel();
        if (select->hasSelection()) {
            int selectedRow = ui->alarmas_table->currentRow();
            ui->alarmas_table->removeRow(selectedRow);
            alarmSounded.remove(selectedRow);
            QMessageBox::warning(this, tr("Eliminar Alarma"), tr("Alarma eliminada."));
        } else {
            QMessageBox::warning(this, tr("Eliminar Alarma"), tr("Selecciona una alarma para eliminar."));
        }
    }
    //volver a habilitar el boton despues de la operacion
    ui->delete_button->setEnabled(true);
    connect(ui->delete_button, &QPushButton::clicked, this, &Principal::on_delete_button_clicked);
}

void Principal::on_edit_button_clicked() {
    //deshabilitar el boton mientras se ejecuta la operacion
    ui->edit_button->setEnabled(false);
    //desconectar cualquier conexion existente para evitar duplicados
    disconnect(ui->edit_button, &QPushButton::clicked, this, &Principal::on_edit_button_clicked);
    //obtener la fila seleccionada
    int currentRow = ui->alarmas_table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("Advertencia"), tr("Por favor, selecciona una alarma para editar."));
        ui->edit_button->setEnabled(true);
        return;
    }
    //obtener los valores actuales de la alarma seleccionada
    QString horaActual = ui->alarmas_table->item(currentRow, 0)->text();
    QString descripcionActual = ui->alarmas_table->item(currentRow, 1)->text();
    //crear un dialogo personalizado para editar la alarma
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Editar Alarma"));
    QVBoxLayout layout(&dialog);
    //crear un widget QTimeEdit para seleccionar la nueva hora
    QTimeEdit timeEdit(&dialog);
    timeEdit.setDisplayFormat("HH:mm");
    timeEdit.setTime(QTime::fromString(horaActual, "HH:mm"));
    layout.addWidget(&timeEdit);
    //crear un cuadro de texto para editar la descripcion
    QLineEdit lineEdit(&dialog);
    lineEdit.setText(descripcionActual);
    layout.addWidget(&lineEdit);
    //añadir un boton de ok y cancelar para confirmar cambios
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout.addWidget(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    //mostrar el dialogo y esperar a que el usuario confirme
    if (dialog.exec() == QDialog::Accepted) {
        QTime nuevaHora = timeEdit.time();
        QString nuevaDescripcion = lineEdit.text();
        //actualizar la tabla con los nuevos valores
        ui->alarmas_table->item(currentRow, 0)->setText(nuevaHora.toString("HH:mm"));
        ui->alarmas_table->item(currentRow, 1)->setText(nuevaDescripcion);
    }
    //volver a habilitar el boton despues de la operacion
    ui->edit_button->setEnabled(true);
    connect(ui->edit_button, &QPushButton::clicked, this, &Principal::on_edit_button_clicked);
}
