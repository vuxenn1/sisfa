#include "stb_image.h"
#include "steganography.h"
#include "vault.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

// helpers

static std::string prompt(const std::string& msg)
{
    std::cout << msg;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

static void checkCapacity()
{
    std::string path = prompt("Image path: ");

    int width, height, channels;
    uint8_t* raw = stbi_load(path.c_str(), &width, &height, &channels, 3);
    if (!raw)
    {
        std::cout << "ERROR: Could not load image: " << path << std::endl;
        return;
    }
    stbi_image_free(raw);

    size_t pixelBytes = static_cast<size_t>(width * height * 3);
    size_t capacity   = maxEmbeddableBytes(pixelBytes);

    std::cout << "Image     : " << width << " x " << height << " (" << channels << " ch)\n"
              << "Capacity  : " << capacity << " bytes  ("
              << capacity / 1024 << " KB)" << std::endl;
}

int main()
{
    while (true)
    {
        std::cout << "[1] Embed file into image\n"
                  << "[2] Extract file from image\n"
                  << "[3] Check image capacity\n"
                  << "[4] Quit\n"
                  << "> ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1")
        {
            std::string carrier = prompt("Carrier image path: ");
            std::string secret  = prompt("Secret file path: ");
            std::string pass    = prompt("Password: ");
            std::string output  = prompt("Output path [output/stego.png as default]: ");
            if (output.empty()) output = "output/stego.png";

            std::cout << "\n";
            embedVault(carrier, secret, pass, output);
        }
        else if (choice == "2")
        {
            std::string stego = prompt("Stego image path : ");
            std::string pass  = prompt("Password         : ");

            std::cout << "\n";
            VaultResult r = extractVault(stego, pass);

            if (!r.success)
                std::cout << "ERROR: " << r.error << std::endl;
            else
            {
                std::filesystem::create_directories("output");
                std::string outPath = "output/" + r.filename;
                std::ofstream f(outPath, std::ios::binary);
                if (f)
                {
                    f.write(reinterpret_cast<const char*>(r.data.data()), r.data.size());
                    std::cout << "File extracted successfully.\n"
                              << "  Saved to: " << outPath << "\n"
                              << "  Size: " << r.data.size() << " bytes" << std::endl;
                }
                else
                    std::cout << "ERROR: Could not write to: " << outPath << std::endl;
            }
        }
        else if (choice == "3")
            checkCapacity();
        else if (choice == "4")
            break;
        else
            std::cout << "Invalid option.\n";
        std::cout << "\n";
    }
    return 0;
}
