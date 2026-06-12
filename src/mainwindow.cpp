#include "mainwindow.h"
#include "embeddialog.h"
#include "extractdialog.h"
#include <QApplication>
#include <QImageReader>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("SISFA");
    setMinimumSize(700, 500);
    setupUI();
}

void MainWindow::setupUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // --- LEFT PANEL ---
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(260);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(24, 32, 24, 24);
    leftLayout->setSpacing(12);

    QLabel* titleLabel = new QLabel("SISFA");
    titleLabel->setObjectName("titleLabel");
    leftLayout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel("Secure Image Steganography\nFile Vault");
    subtitleLabel->setObjectName("subtitleLabel");
    leftLayout->addWidget(subtitleLabel);

    leftLayout->addSpacing(24);

    embedButton = new QPushButton("Embed File");
    embedButton->setObjectName("primaryButton");
    embedButton->setFixedHeight(48);
    embedButton->setCursor(Qt::PointingHandCursor);
    leftLayout->addWidget(embedButton);

    QLabel* embedDesc = new QLabel("Hide a file inside an image");
    embedDesc->setObjectName("descLabel");
    leftLayout->addWidget(embedDesc);

    leftLayout->addSpacing(12);

    extractButton = new QPushButton("Extract File");
    extractButton->setObjectName("primaryButton");
    extractButton->setFixedHeight(48);
    extractButton->setCursor(Qt::PointingHandCursor);
    leftLayout->addWidget(extractButton);

    QLabel* extractDesc = new QLabel("Recover a hidden file from an image");
    extractDesc->setObjectName("descLabel");
    leftLayout->addWidget(extractDesc);

    leftLayout->addStretch();

    imageInfoLabel = new QLabel("");
    imageInfoLabel->setObjectName("infoLabel");
    imageInfoLabel->setWordWrap(true);
    leftLayout->addWidget(imageInfoLabel);

    // --- RIGHT PANEL ---
    QWidget* rightPanel = new QWidget();
    rightPanel->setObjectName("rightPanel");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    previewLabel = new QLabel("SISFA");
    previewLabel->setObjectName("previewLabel");
    previewLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(previewLabel);

    // --- CONNECT ---
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);

    connect(embedButton,   &QPushButton::clicked, this, &MainWindow::onEmbedClicked);
    connect(extractButton, &QPushButton::clicked, this, &MainWindow::onExtractClicked);

    setupStyles();
}

void MainWindow::updateImagePreview(const QString& imagePath)
{
    QPixmap pixmap(imagePath);
    if (pixmap.isNull())
    {
        clearImagePreview();
        return;
    }

    // Scale image to fit the preview panel while keeping aspect ratio
    pixmap = pixmap.scaled(
        previewLabel->width(),
        previewLabel->height(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    previewLabel->setPixmap(pixmap);
    previewLabel->setText("");

    // Show image info in bottom left
    QImageReader reader(imagePath);
    QSize size = reader.size();
    size_t pixelBytes = static_cast<size_t>(size.width() * size.height() * 3);
    size_t capacity   = (pixelBytes - 32) / 8;

    imageInfoLabel->setText(
        QString("%1 x %2 px\nCapacity: %3 KB")
            .arg(size.width())
            .arg(size.height())
            .arg(capacity / 1024)
    );
}

void MainWindow::clearImagePreview()
{
    previewLabel->clear();
    previewLabel->setText("SISFA");
}

void MainWindow::onEmbedClicked()
{
    EmbedDialog dialog(this);
    dialog.exec();
}

void MainWindow::onExtractClicked()
{
    ExtractDialog dialog(this);
    dialog.exec();
}

void MainWindow::setupStyles()
{
    qApp->setStyleSheet(R"(
        QWidget {
            background-color: #eff1f5;
            color: #4c4f69;
            font-family: Segoe UI;
            font-size: 13px;
        }

        QWidget#rightPanel {
            background-color: #e6e9ef;
            border-left: 1px solid #ccd0da;
        }

        QPushButton#primaryButton {
            background-color: #ccd0da;
            color: #4c4f69;
            border: 1px solid #bcc0cc;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            padding: 0 16px;
        }

        QPushButton#primaryButton:hover {
            background-color: #bcc0cc;
            border-color: #ea76cb;
        }

        QPushButton#primaryButton:pressed {
            background-color: #ea76cb;
            color: #eff1f5;
        }

        QLabel#titleLabel {
            font-size: 28px;
            font-weight: bold;
            color: #ea76cb;
        }

        QLabel#subtitleLabel {
            font-size: 12px;
            color: #6c6f85;
        }

        QLabel#descLabel {
            font-size: 11px;
            color: #6c6f85;
            padding-left: 2px;
        }

        QLabel#infoLabel {
            font-size: 11px;
            color: #6c6f85;
            border-top: 1px solid #ccd0da;
            padding-top: 12px;
        }

        QLabel#previewLabel {
            font-size: 48px;
            font-weight: bold;
            color: #ccd0da;
        }
    )");
}