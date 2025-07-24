#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PFA.h"

class PFA : public QMainWindow
{
    Q_OBJECT

public:
    PFA(QWidget *parent = nullptr);
    ~PFA();

private:
    Ui::PFAClass ui;
};

