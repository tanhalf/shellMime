# shellMime – Quick Start

## Requirements

* A Unix-like environment (Linux / Ubuntu / macOS)
* `gcc` compiler installed

## Install gcc (if needed)

### Ubuntu / Debian

```id="c1"
sudo apt update
sudo apt install build-essential
```

### macOS

```id="c2"
xcode-select --install
```

## Compile

From the project root directory:

```id="c3"
gcc -o bin/shellMime src/bcshell.c
```

## Run

```id="c4"
./bin/shellMime
```

## Features

### Parallel Execution (`&`)

* Use `&` to run multiple commands in parallel
* The shell executes commands at the same time instead of waiting for one to finish
* Example:

```id="c5"
sleep 5 & ls
```

### Input Redirection (`<`)

* Redirect input from a file into a command
* Example:

```id="c6"
wc -l < file.txt
```

### Output Redirection (`>`)

* Redirect output of a command into a file (overwrites file)
* Example:

```id="c7"
ls > output.txt
```

## Notes

* If the `bin/` folder doesn’t exist:

```id="c8"
mkdir -p bin
```

* Run all commands from the project root directory
