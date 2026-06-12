#include "extractdialog.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <fstream>
#include <filesystem>
#include "vault.h"

ExtractDialog::ExtractDialog(MainWindow* parent)
    : QDialog(parent)
    , mainWindow(parent)
{
    setWindowTitle("Extract File");
    setMinimumWidth(480);
    setupUI();
}

void ExtractDialog::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    QLabel* title = new QLabel("Extract File from Image");
    title->setObjectName("dialogTitle");
    layout->addWidget(title);

    QLabel* stegoLabel = new QLabel("Stego Image");
    stegoLabel->setObjectName("fieldLabel");
    layout->addWidget(stegoLabel);

    QHBoxLayout* stegoRow = new QHBoxLayout();
    stegoEdit = new QLineEdit();
    stegoEdit->setPlaceholderText("Select a stego PNG image...");
    stegoEdit->setReadOnly(true);
    browseStegoBtn = new QPushButton("Browse");
    browseStegoBtn->setObjectName("browseButton");
    browseStegoBtn->setFixedWidth(80);
    stegoRow->addWidget(stegoEdit);
    stegoRow->addWidget(browseStegoBtn);
    layout->addLayout(stegoRow);

    QLabel* passLabel = new QLabel("Password");
    passLabel->setObjectName("fieldLabel");
    layout->addWidget(passLabel);

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter the password...");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QLabel* outputLabel = new QLabel("Output Folder");
    outputLabel->setObjectName("fieldLabel");
    layout->addWidget(outputLabel);

    QHBoxLayout* outputRow = new QHBoxLayout();
    outputEdit = new QLineEdit();
    outputEdit->setPlaceholderText("Select output folder... (default: output/)");
    outputEdit->setReadOnly(true);
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

    extractBtn = new QPushButton("Extract");
    extractBtn->setObjectName("actionButton");
    extractBtn->setFixedHeight(44);
    extractBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(extractBtn);

    connect(browseStegoBtn,  &QPushButton::clicked, this, &ExtractDialog::onBrowseStego);
    connect(browseOutputBtn, &QPushButton::clicked, this, &ExtractDialog::onBrowseOutput);
    connect(extractBtn,      &QPushButton::clicked, this, &ExtractDialog::onExtract);

    setupStyles();
}

void ExtractDialog::onBrowseStego()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select Stego Image",
        "",
        "PNG Images (*.png)"
    );

    if (path.isEmpty()) return;
    stegoEdit->setText(path);

    // Update the main window preview
    mainWindow->updateImagePreview(path);
}

void ExtractDialog::onBrowseOutput()
{
    QString path = QFileDialog::getExistingDirectory(
        this,
        "Select Output Folder",
        ""
    );

    if (path.isEmpty()) return;
    outputEdit->setText(path);
}

void ExtractDialog::onExtract()
{
    if (stegoEdit->text().isEmpty())
    {
        statusLabel->setStyleSheet("color: #ff6b6b;");
        statusLabel->setText("Please select a stego image.");
        return;
    }
    if (passwordEdit->text().isEmpty())
    {
        statusLabel->setStyleSheet("color: #ff6b6b;");
        statusLabel->setText("Please enter a password.");
        return;
    }

    extractBtn->setEnabled(false);
    statusLabel->setStyleSheet("color: #9a96a8;");
    statusLabel->setText("Extracting...");

    VaultResult result = extractVault(
        stegoEdit->text().toUtf8().constData(),
        passwordEdit->text().toStdString()
    );

    extractBtn->setEnabled(true);

    if (result.success)
    {
        QString outputFolder = outputEdit->text();
        if (outputFolder.isEmpty()) outputFolder = "output";

        std::filesystem::path outDir  = std::filesystem::u8path(outputFolder.toUtf8().constData());
        std::filesystem::create_directories(outDir);
        std::filesystem::path outPath = outDir / result.filename;
        std::ofstream f(outPath, std::ios::binary);
        f.write(reinterpret_cast<const char*>(result.data.data()), result.data.size());

        statusLabel->setStyleSheet("color: #5fd17a;");
        statusLabel->setText(
            QString("File extracted successfully!\nSaved to: %1/%2\nSize: %3 bytes")
                .arg(outputFolder)
                .arg(QString::fromStdString(result.filename))
                .arg(result.data.size())
        );
    }
    else
    {
        statusLabel->setStyleSheet("color: #ff6b6b;");
        statusLabel->setText(QString::fromStdString("Error: " + result.error));
    }
}

void ExtractDialog::setupStyles()
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