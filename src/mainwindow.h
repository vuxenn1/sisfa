#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPixmap>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    // Called by embed dialog when user picks a carrier image
    void updateImagePreview(const QString& imagePath);
    void clearImagePreview();

private slots:
    void onEmbedClicked();
    void onExtractClicked();

private:
    // Left panel
    QPushButton* embedButton;
    QPushButton* extractButton;
    QLabel*      imageInfoLabel;

    // Right panel
    QLabel*      previewLabel;

    // Helpers
    void setupUI();
    void setupStyles();
};

