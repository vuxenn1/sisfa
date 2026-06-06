#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class MainWindow;

class EmbedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmbedDialog(MainWindow* parent = nullptr);

private slots:
    void onBrowseCarrier();
    void onBrowseSecret();
    void onBrowseOutput();
    void onEmbed();

private:
    MainWindow*  mainWindow;

    QLineEdit*   carrierEdit;
    QLineEdit*   secretEdit;
    QLineEdit*   passwordEdit;
    QLineEdit*   outputEdit;

    QPushButton* browseCarrierBtn;
    QPushButton* browseSecretBtn;
    QPushButton* browseOutputBtn;
    QPushButton* embedBtn;

    QLabel*      statusLabel;

    void setupUI();
    void setupStyles();
};