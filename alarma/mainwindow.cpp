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

    // Inicializar el temporizador
    cronometroTimer = new QTimer(this);
    connect(cronometroTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    cronometroTimer->start(1000); // Actualiza cada segundo

    // Inicializar el vector alarmSounded con el número de alarmas
    alarmSounded.resize(0);

    // Conectar botones
    connect(ui->pushButtonAnadirAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonAnadirAlarma_clicked);
    connect(ui->pushButtonEliminarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEliminarAlarma_clicked);
    connect(ui->pushButtonEditarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEditarAlarma_clicked);

    // Configurar la tabla de alarmas
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
    // Verificar si alguna alarma ha sonado
    checkAlarms(currentTime);
}

void MainWindow::on_pushButtonAnadirAlarma_clicked() {
    ui->pushButtonAnadirAlarma->setEnabled(false);

    // Desconectar cualquier conexión existente para evitar duplicados
    disconnect(ui->pushButtonAnadirAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonAnadirAlarma_clicked);

    // Crear un diálogo personalizado para seleccionar la hora
    QDialog dialog(this);
    dialog.setWindowTitle("Seleccionar Hora de la Alarma");

    QVBoxLayout layout(&dialog);

    // Crear un widget QTimeEdit y añadirlo al diálogo
    QTimeEdit timeEdit(&dialog);
    timeEdit.setDisplayFormat("HH:mm");
    layout.addWidget(&timeEdit);

    // Añadir un botón de OK y Cancelar para confirmar la hora
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout.addWidget(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Mostrar el diálogo y esperar a que el usuario seleccione una hora
    if (dialog.exec() == QDialog::Accepted) {
        QTime horaAlarma = timeEdit.time();  // Obtener la hora seleccionada
        QString horaAlarmaStr = horaAlarma.toString("HH:mm");
        QTime currentTime = QTime::currentTime(); // Obtener la hora actual

        // Verificar si la hora de la alarma ya pasó
        if (horaAlarma < currentTime) {
            // Ajustar la hora de la alarma para el siguiente día
            horaAlarma = horaAlarma.addSecs(24 * 3600); // Sumar 24 horas en segundos
            horaAlarmaStr = horaAlarma.toString("HH:mm");
            QMessageBox::information(this, tr("Hora Ajustada"), tr("La hora seleccionada ya ha pasado. La alarma se ha ajustado para el siguiente día: ") + horaAlarmaStr);
        }

        // Pedir la descripción de la alarma
        bool ok;
        QString descripcion = QInputDialog::getText(this, tr("Añadir Alarma"),
                                                    tr("Descripción:"), QLineEdit::Normal,
                                                    "", &ok);
        if (ok && !descripcion.isEmpty()) {
            // Añadir la alarma a la tabla
            int rowCount = ui->tableWidgetAlarmas->rowCount();
            ui->tableWidgetAlarmas->insertRow(rowCount);
            ui->tableWidgetAlarmas->setItem(rowCount, 0, new QTableWidgetItem(horaAlarmaStr));
            ui->tableWidgetAlarmas->setItem(rowCount, 1, new QTableWidgetItem(descripcion));
            alarmSounded.push_back(false); // Añadir estado de alarma no sonada
        }
    }

    // Volver a habilitar el botón después de la operación
    ui->pushButtonAnadirAlarma->setEnabled(true);
    // Reconectar la señal
    connect(ui->pushButtonAnadirAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonAnadirAlarma_clicked);
}

void MainWindow::on_pushButtonEliminarAlarma_clicked() {
    // Deshabilitar el botón mientras se ejecuta la operación
    ui->pushButtonEliminarAlarma->setEnabled(false);

    // Desconectar cualquier conexión existente para evitar duplicados
    disconnect(ui->pushButtonEliminarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEliminarAlarma_clicked);

    // Crear un diálogo de confirmación
    QMessageBox confirmationBox(this);
    confirmationBox.setWindowTitle(tr("Confirmar Eliminación"));
    confirmationBox.setText(tr("¿Está seguro de que desea eliminar la alarma seleccionada?"));
    confirmationBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmationBox.setDefaultButton(QMessageBox::No);

    // Mostrar el diálogo y esperar a la respuesta del usuario
    if (confirmationBox.exec() == QMessageBox::Yes) {
        QItemSelectionModel *select = ui->tableWidgetAlarmas->selectionModel();

        if (select->hasSelection()) {
            // Obtener la fila seleccionada
            QModelIndexList selectedRows = select->selectedRows();
            if (!selectedRows.isEmpty()) {
                int row = selectedRows.first().row();  // Obtener la primera fila seleccionada
                ui->tableWidgetAlarmas->removeRow(row); // Eliminar la fila
                alarmSounded.remove(row); // También eliminar el estado de la alarma correspondiente
            }
        } else {
            QMessageBox::warning(this, tr("Eliminar Alarma"), tr("Selecciona una alarma para eliminar."));
        }
    }

    // Volver a habilitar el botón después de la operación
    ui->pushButtonEliminarAlarma->setEnabled(true);

    // Reconectar la señal
    connect(ui->pushButtonEliminarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEliminarAlarma_clicked);
}

void MainWindow::checkAlarms(const QTime &currentTime) {
    for (int row = 0; row < ui->tableWidgetAlarmas->rowCount(); ++row) {
        QString horaAlarma = ui->tableWidgetAlarmas->item(row, 0)->text();
        QTime alarmTime = QTime::fromString(horaAlarma, "HH:mm");

        // Asegurarse de que la alarma sea válida
        if (alarmTime.isValid() && currentTime >= alarmTime) {
            // Mostrar mensaje de alarma solo si no ha sonado antes
            if (!alarmSounded[row]) {
                QMessageBox::information(this, tr("Alarma"), tr("¡Alarma sonando! ") + ui->tableWidgetAlarmas->item(row, 1)->text());
                alarmSounded[row] = true; // Marcar como sonada
            }
        }
    }
}

void MainWindow::on_pushButtonEditarAlarma_clicked() {
    // Deshabilitar el botón mientras se ejecuta la operación
    ui->pushButtonEditarAlarma->setEnabled(false);

    // Desconectar cualquier conexión existente para evitar duplicados
    disconnect(ui->pushButtonEditarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEditarAlarma_clicked);

    // Obtener la fila seleccionada
    int currentRow = ui->tableWidgetAlarmas->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("Advertencia"), tr("Por favor, selecciona una alarma para editar."));
        ui->pushButtonEditarAlarma->setEnabled(true); // Habilitar el botón si no hay selección
        return; // Salir si no hay una fila seleccionada
    }

    // Obtener los valores actuales de la alarma seleccionada
    QString horaActual = ui->tableWidgetAlarmas->item(currentRow, 0)->text();
    QString descripcionActual = ui->tableWidgetAlarmas->item(currentRow, 1)->text();

    // Crear un diálogo personalizado para editar la alarma
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Editar Alarma"));

    QVBoxLayout layout(&dialog);

    // Crear un widget QTimeEdit para seleccionar la nueva hora
    QTimeEdit timeEdit(&dialog);
    timeEdit.setDisplayFormat("HH:mm");
    timeEdit.setTime(QTime::fromString(horaActual, "HH:mm")); // Establecer la hora actual
    layout.addWidget(&timeEdit);

    // Crear un cuadro de texto para editar la descripción
    QLineEdit lineEdit(&dialog);
    lineEdit.setText(descripcionActual); // Establecer la descripción actual
    layout.addWidget(&lineEdit);

    // Añadir un botón de OK y Cancelar para confirmar la edición
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout.addWidget(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Mostrar el diálogo y esperar a que el usuario confirme
    if (dialog.exec() == QDialog::Accepted) {
        QTime nuevaHora = timeEdit.time(); // Obtener la nueva hora
        QString nuevaDescripcion = lineEdit.text(); // Obtener la nueva descripción

        // Actualizar la tabla con los nuevos valores
        ui->tableWidgetAlarmas->item(currentRow, 0)->setText(nuevaHora.toString("HH:mm"));
        ui->tableWidgetAlarmas->item(currentRow, 1)->setText(nuevaDescripcion);
    }

    // Volver a habilitar el botón después de la operación
    ui->pushButtonEditarAlarma->setEnabled(true);

    // Reconectar la señal
    connect(ui->pushButtonEditarAlarma, &QPushButton::clicked, this, &MainWindow::on_pushButtonEditarAlarma_clicked);
}
