# shellMime – Quick Start

## Requirements

* A Unix-like environment (Linux / Ubuntu / macOS)
* `gcc` compiler installed

## Install gcc (if needed)

### Ubuntu / Debian

sudo apt update
sudo apt install build-essential


### macOS

Install Xcode Command Line Tools:

xcode-select --install

## Compile

From the project root directory:

gcc -o bin/shellMime src/bcshell.c

## Run

./bin/shellMime

## Notes

* gcc is most commonly used on Linux, but it also works on macOS and Windows (via tools like WSL or MinGW).
* If the bin/ folder doesn’t exist:

mkdir -p bin

* Make sure you run commands from the project root directory.
