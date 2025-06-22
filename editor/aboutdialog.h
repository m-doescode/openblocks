#pragma once

#include <qdialog.h>
#include <qobjectdefs.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class AboutDialog;
}
QT_END_NAMESPACE

class AboutDialog : public QDialog
{
    Q_OBJECT

    Ui::AboutDialog *ui;
public:
    AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() = default;
    
protected:
    void linkActivated(const QString &link);
};