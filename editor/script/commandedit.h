#pragma once

#include <qlineedit.h>
#include <vector>

class CommandEdit : public QLineEdit {
    Q_OBJECT

    std::vector<std::string> commandHistory;
    int historyIndex = 0;

    void executeCommand();
public:
    CommandEdit(QWidget* parent = nullptr);
    ~CommandEdit();

    void keyPressEvent(QKeyEvent *) override;
};