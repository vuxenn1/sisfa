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
        statusLabel->setStyleSheet("color: #5fd17a;");
        statusLabel->setText("File embedded successfully!\nSaved to: " + output);
    }
    else
    {
        statusLabel->setStyleSheet("color: #ff6b6b;");
        statusLabel->setText("Embedding failed. Check image capacity.");
    }
}

void EmbedDialog::setupStyles()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #16161e;
        }

        QLabel#dialogTitle {
            font-size: 18px;
            font-weight: bold;
            color: #ff4f9e;
        }

        QLabel#fieldLabel {
            font-size: 12px;
            color: #9a96a8;
        }

        QLineEdit {
            background-color: #1f1f29;
            color: #e8e6f0;
            border: 1px solid #34344a;
            border-radius: 4px;
            padding: 6px 10px;
            font-size: 13px;
        }

        QLineEdit:focus {
            border-color: #4ea8ff;
        }

        QLineEdit:read-only {
            background-color: #1a1a22;
            color: #6f6c7a;
        }

        QPushButton#browseButton {
            background-color: #1f1f29;
            color: #e8e6f0;
            border: 1px solid #34344a;
            border-radius: 4px;
            padding: 6px;
            font-size: 12px;
        }

        QPushButton#browseButton:hover {
            background-color: #29293a;
            border-color: #4ea8ff;
            color: #6fbcff;
        }

        QPushButton#actionButton {
            background-color: #ff4f9e;
            color: #16161e;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
        }

        QPushButton#actionButton:hover {
            background-color: #ff77b5;
        }

        QPushButton#actionButton:pressed {
            background-color: #d63d82;
        }

        QPushButton#actionButton:disabled {
            background-color: #29293a;
            color: #5a5868;
        }

        QLabel#statusLabel {
            font-size: 12px;
            color: #9a96a8;
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