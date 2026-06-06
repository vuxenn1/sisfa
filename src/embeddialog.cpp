#include "embeddialog.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include "vault.h"

EmbedDialog::EmbedDialog(MainWindow* parent)
    : QDialog(parent)
    , mainWindow(parent)
{
    setWindowTitle("Embed File");
    setMinimumWidth(480);
    setupUI();
}

void EmbedDialog::onBrowseCarrier()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select Carrier Image",
        "",
        "PNG Images (*.png)"
    );

    if (path.isEmpty()) return;

    carrierEdit->setText(path);

    // Update the main window preview
    mainWindow->updateImagePreview(path);
}

void EmbedDialog::onBrowseSecret()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select Secret File",
        "",
        "All Files (*.*)"
    );

    if (path.isEmpty()) return;

    secretEdit->setText(path);
}

void EmbedDialog::onBrowseOutput()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        "Save Stego Image As",
        "output/stego.png",
        "PNG Images (*.png)"
    );

    if (path.isEmpty()) return;

    outputEdit->setText(path);
}

void EmbedDialog::onEmbed()
{
    // Validate inputs
    if (carrierEdit->text().isEmpty())
    {
        statusLabel->setText("Please select a carrier image.");
        return;
    }
    if (secretEdit->text().isEmpty())
    {
        statusLabel->setText("Please select a secret file.");
        return;
    }
    if (passwordEdit->text().isEmpty())
    {
        statusLabel->setText("Please enter a password.");
        return;
    }

    QString carrier  = carrierEdit->text();
    QString secret   = secretEdit->text();
    QString password = passwordEdit->text();
    QString output   = outputEdit->text();
    if (output.isEmpty()) output = "output/stego.png";

    // Disable button while working
    embedBtn->setEnabled(false);
    statusLabel->setText("Embedding...");

    bool ok = embedVault(
        carrier.toUtf8().constData(),
        secret.toUtf8().constData(),
        password.toStdString(),
        output.toUtf8().constData()
    );

    embedBtn->setEnabled(true);

    if (ok)
    {
        statusLabel->setStyleSheet("color: #a6e3a1;");
        statusLabel->setText("File embedded successfully!\nSaved to: " + output);
    }
    else
    {
        statusLabel->setStyleSheet("color: #f38ba8;");
        statusLabel->setText("Embedding failed. Check image capacity.");
    }
}

void EmbedDialog::setupStyles()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #1e1e2e;
        }

        QLabel#dialogTitle {
            font-size: 18px;
            font-weight: bold;
            color: #89b4fa;
        }

        QLabel#fieldLabel {
            font-size: 12px;
            color: #6c7086;
        }

        QLineEdit {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 4px;
            padding: 6px 10px;
            font-size: 13px;
        }

        QLineEdit:focus {
            border-color: #89b4fa;
        }

        QLineEdit:read-only {
            background-color: #181825;
            color: #6c7086;
        }

        QPushButton#browseButton {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 4px;
            padding: 6px;
            font-size: 12px;
        }

        QPushButton#browseButton:hover {
            background-color: #45475a;
            border-color: #89b4fa;
        }

        QPushButton#actionButton {
            background-color: #89b4fa;
            color: #1e1e2e;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }

        QPushButton#actionButton:hover {
            background-color: #b4d0ff;
        }

        QPushButton#actionButton:disabled {
            background-color: #45475a;
            color: #6c7086;
        }

        QLabel#statusLabel {
            font-size: 12px;
            color: #6c7086;
        }
    )");
}

void EmbedDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    QLabel* title = new QLabel("Embed File into Image");
    title->setObjectName("dialogTitle");
    layout->addWidget(title);

    QLabel* carrierLabel = new QLabel("Carrier Image");
    carrierLabel->setObjectName("fieldLabel");
    layout->addWidget(carrierLabel);

    QHBoxLayout* carrierRow = new QHBoxLayout();
    carrierEdit = new QLineEdit();
    carrierEdit->setPlaceholderText("Select a PNG image...");
    carrierEdit->setReadOnly(true);
    browseCarrierBtn = new QPushButton("Browse");
    browseCarrierBtn->setObjectName("browseButton");
    browseCarrierBtn->setFixedWidth(80);
    carrierRow->addWidget(carrierEdit);
    carrierRow->addWidget(browseCarrierBtn);
    layout->addLayout(carrierRow);

    QLabel* secretLabel = new QLabel("Secret File");
    secretLabel->setObjectName("fieldLabel");
    layout->addWidget(secretLabel);

    QHBoxLayout* secretRow = new QHBoxLayout();
    secretEdit = new QLineEdit();
    secretEdit->setPlaceholderText("Select a file to hide...");
    secretEdit->setReadOnly(true);
    browseSecretBtn = new QPushButton("Browse");
    browseSecretBtn->setObjectName("browseButton");
    browseSecretBtn->setFixedWidth(80);
    secretRow->addWidget(secretEdit);
    secretRow->addWidget(browseSecretBtn);
    layout->addLayout(secretRow);

    QLabel* passLabel = new QLabel("Password");
    passLabel->setObjectName("fieldLabel");
    layout->addWidget(passLabel);

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter a strong password...");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QLabel* outputLabel = new QLabel("Output Image");
    outputLabel->setObjectName("fieldLabel");
    layout->addWidget(outputLabel);

    QHBoxLayout* outputRow = new QHBoxLayout();
    outputEdit = new QLineEdit();
    outputEdit->setPlaceholderText("output/stego.png (default)");
    browseOutputBtn = new QPushButton("Browse");
    browseOutputBtn->setObjectName("browseButton");
    browseOutputBtn->setFixedWidth(80);
    outputRow->addWidget(outputEdit);
    outputRow->addWidget(browseOutputBtn);
    layout->addLayout(outputRow);

    statusLabel = new QLabel("");
    statusLabel->setObjectName("statusLabel");
    statusLabel->setWordWrap(true);
    layout->addWidget(statusLabel);

    layout->addStretch();

    embedBtn = new QPushButton("Embed");
    embedBtn->setObjectName("actionButton");
    embedBtn->setFixedHeight(44);
    embedBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(embedBtn);

    connect(browseCarrierBtn, &QPushButton::clicked, this, &EmbedDialog::onBrowseCarrier);
    connect(browseSecretBtn,  &QPushButton::clicked, this, &EmbedDialog::onBrowseSecret);
    connect(browseOutputBtn,  &QPushButton::clicked, this, &EmbedDialog::onBrowseOutput);
    connect(embedBtn,         &QPushButton::clicked, this, &EmbedDialog::onEmbed);

    setupStyles();
}