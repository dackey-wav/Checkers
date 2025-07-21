# Checkers Game

This project is a Checkers game implemented with a graphical user interface (GUI) using the **SFML 3.0** library. It offers both **Player vs. Player** and **Player vs. AI** modes, providing a classic game experience with an intelligent opponent.

---

## Features

* **SFML 3.0 Graphics:** A modern and responsive graphical interface powered by the SFML 3.0 library, ensuring smooth animations and a pleasant visual experience.
* **Player vs. Player Mode:** Enjoy a traditional game of checkers against a friend on the same computer.
* **Player vs. AI Mode:** Challenge an artificial intelligence opponent.
* **Minimax AI with Alpha-Beta Pruning:** The AI opponent is powered by the **Minimax algorithm with Alpha-Beta Pruning**, a classic search algorithm for decision-making in turn-based games. This allows the AI to anticipate moves and make strategic decisions.
* **Two Difficulty Levels:**
    * **Easy:** The AI uses a shallower search depth, making it more approachable for casual play.
    * **Hard:** The AI delves deeper into possible moves, providing a more challenging and strategic opponent.

---

## Project Structure

* `include/`: Contains all header files (`.hpp`) for class declarations.
* `src/`: Contains all source files (`.cpp`) for class implementations.
* `viz/assets/`: Stores game resources such as images for pieces, board textures, etc.

---

## How to Play

* **Player vs. Player:** Use your mouse to click on the piece you want to move, then click on the target square.
* **Player vs. AI:** Choose your desired difficulty level (Easy/Hard) at the start of the game. The AI will automatically make its moves.

---
