# SISFA: Secure Image Steganography File Vault

Hides and encrypts private files inside PNG images.  
Two security layers: **AES-256 encryption** (password) + **LSB steganography** (invisible in the image).

## How it works

1. You pick a carrier PNG and a secret file
2. The file gets encrypted with your password (AES-256-CBC + PBKDF2)
3. The encrypted data gets embedded into the image's pixels, the image looks completely normal
4. To recover the file, you need both the stego image and the password

## Build

**Requirements:** 
- CMake 3.20+ 
- GCC/G++
- OpenSSL  
- Qt6 (Core, Widgets)
- On Windows (MSYS64): `pacman -S mingw-w64-ucrt-x86_64-openssl mingw-w64-ucrt-x86_64-qt6`

```bash
cmake -B ./build
cmake --build ./build
```
## Run
```bash
./build/SISFA.exe
```
* Run from the project root. 
* Options: embed file, extract file, check image capacity.

## Notes
- Carrier image must be large enough to hold the file (use option 3 to check capacity).
- Only PNG carrier images are supported.
- Output is saved to "output/" as default.
