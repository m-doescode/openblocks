#include "license.h"
#include "./ui_license.h"

extern const char* licensetxt;

LicenseDialog::LicenseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::License)
{
    ui->setupUi(this);
    ui->plainTextEdit->setPlainText(licensetxt);
}