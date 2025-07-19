#include "aboutdialog.h"
#include "./ui_aboutdialog.h"
#include "version.h"
#include <qdialogbuttonbox.h>
#include <qnamespace.h>
#include <qplaintextedit.h>
#include <qfile.h>

class LicenseDialog : public QDialog {
public:
    LicenseDialog(QWidget* parent = nullptr) : QDialog(parent) {
        this->resize(700, 500);
        this->setMinimumSize(QSize(500, 500));
        this->setMaximumSize(QSize(500, 500));

        setWindowTitle("License");

        QFile licenseFile(":/LICENSE");
        licenseFile.open(QFile::ReadOnly);
        QString licenseContent = licenseFile.readAll();
        licenseFile.close();

        QPlainTextEdit* licenseText = new QPlainTextEdit(this);
        licenseText->setGeometry(QRect(10, 10, 500-20, 500-20-32-10));
        licenseText->setPlainText(licenseContent);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(10, 500-32-10, 500-20, 32));
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Close);

        QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    };
};

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->titleString->setText(QString() + "Openblocks Engine " + BUILD_VERSION);
    ui->versionString->setText(BUILD_VERSION_LONG);
    
    connect(ui->viewLicense, &QLabel::linkActivated, [this]() {
        (new LicenseDialog(this))->open();
    });
}