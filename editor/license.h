#pragma once

#include <qdialog.h>
#include <qobjectdefs.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class License;
}
QT_END_NAMESPACE

class LicenseDialog : public QDialog
{
    Q_OBJECT

    Ui::License *ui;
public:
    LicenseDialog(QWidget *parent = nullptr);
    ~LicenseDialog() = default;
    
};