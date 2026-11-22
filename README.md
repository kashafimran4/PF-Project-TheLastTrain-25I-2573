# The Last Train - Programming Fundamentals Project (Procedural / No Structs)

CS-1002: Programming Fundamentals (AI & DS)

## Architecture Overview

This project strictly follows primitive procedural programming concepts:

- **No Structs / No OOP:** All variables, game statuses, inventory, and building rooms are stored in primitive types (`int`, `char`, `bool`) and multi-dimensional parallel arrays.
- `include/`: Modular C++ header definitions (`config.h`, `state.h`, `level.h`, `simulator.h`, `physics.h`, `switches.h`, `logger.h`, `util.h`, `viewer_iface.h`).
- `src/core/`: Modular implementation files.
- `data/`: Building map (`building.txt`) and configuration (`initial-state.txt`).
- `out/`: Runtime save states (`savegame.txt`) and session summary reports (`game_summary.txt`).
- `docs/`: Project diagrams (`Project_Diagram.jpg`, `Flowchart.png`) and console screenshot (`ConsoleUI.png`).

## Build Instructions (g++ C++17)

Compile the project using:

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train
```

Run on Windows PowerShell:

```powershell
.\last_train.exe
```

## Game Overview

The Last Train is a text-based C++ game in which the player explores a multi-floor building, collects required items, solves challenges, manages resources, and completes the mission to board the train.

## How to Play

The game accepts text-based commands through the console.

- `move floor room` – Move to a connected room.
- `inspect` – Inspect the current room.
- `collect symbol` – Collect an available item.
- `use symbol` – Use an item from inventory.
- `unlock 427` – Unlock the cabinet using the code.
- `map` – Display the building map.
- `status` – Display player status.
- `objectives` – Display mission objectives.
- `save` – Save the current game.
- `load` – Load a saved game.
- `help` – Display available commands.
- `quit` – Exit the game.

## Save and Load System

- `out/savegame.txt` stores the saved game state.
- `out/game_summary.txt` stores the game session summary.

## Technologies Used

- C++
- C++17
- GCC (g++)
- Visual Studio Code
