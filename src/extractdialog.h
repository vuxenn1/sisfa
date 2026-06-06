#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ExtractDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractDialog(QWidget* parent = nullptr);

private slots:
    void onBrowseStego();
    void onBrowseOutput();
    void onExtract();

private:
    QLineEdit*   stegoEdit;
    QLineEdit*   passwordEdit;
    QLineEdit*   outputEdit;
    QPushButton* browseStegoBtn;
    QPushButton* browseOutputBtn;
    QPushButton* extractBtn;
    QLabel*      statusLabel;

    void setupUI();
    void setupStyles();
};