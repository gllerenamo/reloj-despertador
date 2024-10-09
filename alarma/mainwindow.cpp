#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QTime>
#include <QTableWidgetItem>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QTimeEdit>
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    //inicializar el temporizador
    cronometroTimer = new QTimer(this);
    connect(cronometroTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    cronometroTimer->start(1000); //actualiza cada segundo
    alarmSounded.resize(0);
    //conectar botones
    connect(ui->pushButtonAnadirAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonAnadirAlarma_clicked);
    connect(ui->pushButtonEliminarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEliminarAlarma_clicked);
    connect(ui->pushButtonEditarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEditarAlarma_clicked);
    //tabla de alarmas
    ui->tableWidgetAlarmas->setColumnCount(2);
    QStringList headers = {"Hora", "Descripción"};
    ui->tableWidgetAlarmas->setHorizontalHeaderLabels(headers);
}
MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::updateTime() {
    QTime currentTime = QTime::currentTime();
    ui->labelHoraActual->setText(currentTime.toString("HH:mm:ss"));
    checkAlarms(currentTime);
}
void MainWindow::on_pushButtonAnadirAlarma_clicked() {
    ui->pushButtonAnadirAlarma->setEnabled(false);
    //desconectar cualquier conexion existente para evitar duplicados
    disconnect(ui->pushButtonAnadirAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonAnadirAlarma_clicked);
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
            int rowCount = ui->tableWidgetAlarmas->rowCount();
            ui->tableWidgetAlarmas->insertRow(rowCount);
            ui->tableWidgetAlarmas->setItem(rowCount, 0, new QTableWidgetItem(horaAlarmaStr));
            ui->tableWidgetAlarmas->setItem(rowCount, 1, new QTableWidgetItem(descripcion));
            alarmSounded.push_back(false);
        }
    }
    //volver a habilitar el boton despues de la operacion
    ui->pushButtonAnadirAlarma->setEnabled(true);
    connect(ui->pushButtonAnadirAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonAnadirAlarma_clicked);
}
void MainWindow::on_pushButtonEliminarAlarma_clicked() {
    //deshabilitar el boton mientras se ejecuta la operacion
    ui->pushButtonEliminarAlarma->setEnabled(false);
    disconnect(ui->pushButtonEliminarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEliminarAlarma_clicked);
    //crear un dialogo de confirmacion
    QMessageBox confirmationBox(this);
    confirmationBox.setWindowTitle(tr("Confirmar Eliminación"));
    confirmationBox.setText(tr("¿Está seguro de que desea eliminar la alarma seleccionada?"));
    confirmationBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmationBox.setDefaultButton(QMessageBox::No);

    if (confirmationBox.exec() == QMessageBox::Yes) {
        QItemSelectionModel *select = ui->tableWidgetAlarmas->selectionModel();
        if (select->hasSelection()) {
            //obtener la fila seleccionada
            QModelIndexList selectedRows = select->selectedRows();
            if (!selectedRows.isEmpty()) {
                int row = selectedRows.first().row();
                ui->tableWidgetAlarmas->removeRow(row);
                alarmSounded.remove(row);
            }
        } else {
            QMessageBox::warning(this, tr("Eliminar Alarma"), tr("Selecciona una alarma para eliminar."));
        }
    }
    //volver a habilitar el boton despues de la operacion
    ui->pushButtonEliminarAlarma->setEnabled(true);
    connect(ui->pushButtonEliminarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEliminarAlarma_clicked);
}
void MainWindow::checkAlarms(const QTime &currentTime) {
    for (int row = 0; row < ui->tableWidgetAlarmas->rowCount(); ++row) {
        QString horaAlarma = ui->tableWidgetAlarmas->item(row, 0)->text();
        QTime alarmTime = QTime::fromString(horaAlarma, "HH:mm");
        //asegurarse de que la alarma sea valida y mostrar mensaje de alarma solo si no ha sonado antes
        if (alarmTime.isValid() && currentTime >= alarmTime) {
            if (!alarmSounded[row]) {
                QMessageBox::information(this, tr("Alarma"), tr("¡Alarma sonando! ") + ui->tableWidgetAlarmas->item(row, 1)->text());
                alarmSounded[row] = true; //marcar como sonada
            }
        }
    }
}
void MainWindow::on_pushButtonEditarAlarma_clicked() {
    //deshabilitar el boton mientras se ejecuta la operacion
    ui->pushButtonEditarAlarma->setEnabled(false);
    //desconectar cualquier conexion existente para evitar duplicados
    disconnect(ui->pushButtonEditarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEditarAlarma_clicked);
    //obtener la fila seleccionada
    int currentRow = ui->tableWidgetAlarmas->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("Advertencia"), tr("Por favor, selecciona una alarma para editar."));
        ui->pushButtonEditarAlarma->setEnabled(true);
        return;
    }
    //obtener los valores actuales de la alarma seleccionada
    QString horaActual = ui->tableWidgetAlarmas->item(currentRow, 0)->text();
    QString descripcionActual = ui->tableWidgetAlarmas->item(currentRow, 1)->text();
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
        ui->tableWidgetAlarmas->item(currentRow, 0)->setText(nuevaHora.toString("HH:mm"));
        ui->tableWidgetAlarmas->item(currentRow, 1)->setText(nuevaDescripcion);
    }
    //volver a habilitar el boton despues de la operacion
    ui->pushButtonEditarAlarma->setEnabled(true);
    connect(ui->pushButtonEditarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEditarAlarma_clicked);
}
