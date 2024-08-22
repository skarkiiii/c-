#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream> // For file handling

using namespace sf;
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_SIZE 20
#define HIGHEST_SCORE_FILE "highest_score.txt" // File to store the highest score
#define EAT_SOUND_FILE "eat.wav" // Sound file for eating fruit

enum Direction { UP, DOWN, LEFT, RIGHT };

class SnakeGame {
public:
    SnakeGame();
    void run();

private:
    void processInput();
    void update();
    void render();
    void setup();
    void moveSnake();
    void checkCollisions();
    void generateFruit();
    void displayGameOverScreen();
    void loadHighestScore();
    void saveHighestScore();
    void playEatSound(); // Function to play the eating sound

    RenderWindow window;
    RectangleShape snakeSegment;
    RectangleShape fruit;
    Font font;
    Text gameOverText;
    Text fruitEatenText;
    Text replayInstructionsText;
    Text highestScoreText;
    SoundBuffer eatSoundBuffer; // Buffer to hold the sound
    Sound eatSound; // Sound object to play the sound

    int width, height, x, y, fruitx, fruity, score, piece, fruitsEaten;
    int highestScore; // Make sure this is declared before gameend
    Direction currentDirection, nextDirection;
    bool gameend;
    vector<Vector2i> tail;
};

SnakeGame::SnakeGame()
    : window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game"),
      snakeSegment(Vector2f(GRID_SIZE, GRID_SIZE)),
      fruit(Vector2f(GRID_SIZE, GRID_SIZE)),
      width(WINDOW_WIDTH / GRID_SIZE), height(WINDOW_HEIGHT / GRID_SIZE),
      x(width / 2), y(height / 2), score(0), piece(0), fruitsEaten(0),
      highestScore(0), // Initialize highestScore here
      currentDirection(RIGHT), nextDirection(RIGHT), gameend(false) // Initialize gameend here
{
    snakeSegment.setFillColor(Color::Red);
    fruit.setFillColor(Color::Yellow);

    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Failed to load font" << endl;
    }

    if (!eatSoundBuffer.loadFromFile(EAT_SOUND_FILE)) {
        cerr << "Failed to load sound file" << endl;
    }
    eatSound.setBuffer(eatSoundBuffer);

    gameOverText.setFont(font);
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(Color::White);
    gameOverText.setString("Game Over!");

    fruitEatenText.setFont(font);
    fruitEatenText.setCharacterSize(30);
    fruitEatenText.setFillColor(Color::White);

    replayInstructionsText.setFont(font);
    replayInstructionsText.setCharacterSize(25);
    replayInstructionsText.setFillColor(Color::White);

    highestScoreText.setFont(font);
    highestScoreText.setCharacterSize(25);
    highestScoreText.setFillColor(Color::White);

    loadHighestScore(); // Load the highest score from file
    setup();
}

void SnakeGame::setup() {
    x = width / 2;
    y = height / 2;
    generateFruit();
    score = 0;
    piece = 0;
    fruitsEaten = 0;
    tail.clear();
    gameend = false;
    currentDirection = RIGHT;
    nextDirection = RIGHT;
}

void SnakeGame::processInput() {
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed) {
            window.close();
        }
        if (event.type == Event::KeyPressed) {
            if (gameend) {
                if (event.key.code == Keyboard::Y) {
                    setup();
                } else if (event.key.code == Keyboard::Q) {
                    saveHighestScore(); // Save the highest score before quitting
                    window.close();
                }
            } else {
                switch (event.key.code) {
                    case Keyboard::W: if (currentDirection != DOWN) nextDirection = UP; break;
                    case Keyboard::S: if (currentDirection != UP) nextDirection = DOWN; break;
                    case Keyboard::A: if (currentDirection != RIGHT) nextDirection = LEFT; break;
                    case Keyboard::D: if (currentDirection != LEFT) nextDirection = RIGHT; break;
                    case Keyboard::Q: gameend = true; break;
                    default: break;
                }
            }
        }
    }
}

void SnakeGame::update() {
    if (!gameend) {
        moveSnake();
        checkCollisions();
    }
}

void SnakeGame::render() {
    window.clear();
    
    if (gameend) {
        displayGameOverScreen();
    } else {
        for (const auto &segment : tail) {
            snakeSegment.setPosition(segment.x * GRID_SIZE, segment.y * GRID_SIZE);
            window.draw(snakeSegment);
        }
        
        fruit.setPosition(fruitx * GRID_SIZE, fruity * GRID_SIZE);
        window.draw(fruit);
    }
    
    window.display();
}

void SnakeGame::moveSnake() {
    currentDirection = nextDirection;
    tail.insert(tail.begin(), Vector2i(x, y));
    if (tail.size() > static_cast<size_t>(piece + 1)) {
        tail.pop_back();
    }

    switch (currentDirection) {
        case UP: y--; break;
        case DOWN: y++; break;
        case LEFT: x--; break;
        case RIGHT: x++; break;
    }
    
    if (x < 0 || x >= width || y < 0 || y >= height) {
        gameend = true;
    }

    if (x == fruitx && y == fruity) {
        score += 10;
        piece++;
        fruitsEaten++;
        generateFruit();
        playEatSound(); // Play sound when eating fruit
    }
}

void SnakeGame::checkCollisions() {
    for (const auto &segment : tail) {
        if (segment.x == x && segment.y == y) {
            gameend = true;
            if (fruitsEaten > highestScore) {
                highestScore = fruitsEaten; // Update highest score
            }
            break;
        }
    }
}

void SnakeGame::generateFruit() {
    bool onSnake;
    do {
        onSnake = false;
        fruitx = rand() % width;
        fruity = rand() % height;

        for (const auto &segment : tail) {
            if (segment.x == fruitx && segment.y == fruity) {
                onSnake = true;
                break;
            }
        }
    } while (onSnake);
}

void SnakeGame::displayGameOverScreen() {
    // Position the Game Over text at the center of the window
    gameOverText.setPosition(WINDOW_WIDTH / 2 - gameOverText.getLocalBounds().width / 2,
                             WINDOW_HEIGHT / 2 - gameOverText.getLocalBounds().height / 2 - 50);

    // Display the number of fruits eaten at the top-left corner
    string fruitsText = "Fruits Eaten: " + to_string(fruitsEaten);
    fruitEatenText.setString(fruitsText);
    fruitEatenText.setPosition(10, 10); // Position at the top-left corner with some padding

    // Display highest score
    string highestScoreTextString = "Highest Score: " + to_string(highestScore);
    highestScoreText.setString(highestScoreTextString);
    highestScoreText.setPosition(WINDOW_WIDTH / 2 - highestScoreText.getLocalBounds().width / 2,
                                 WINDOW_HEIGHT / 2 + 10); // Position below the game over text

    // Display replay and quit instructions
    replayInstructionsText.setString("Press Y to Replay or Q to Quit");
    replayInstructionsText.setPosition(WINDOW_WIDTH / 2 - replayInstructionsText.getLocalBounds().width / 2,
                                       WINDOW_HEIGHT / 2 + 50);

    window.draw(gameOverText);
    window.draw(fruitEatenText); // Draw the fruits eaten text
    window.draw(highestScoreText); // Draw the highest score text
    window.draw(replayInstructionsText); // Draw the replay and quit instructions
}

void SnakeGame::loadHighestScore() {
    ifstream file(HIGHEST_SCORE_FILE);
    if (file.is_open()) {
        file >> highestScore;
        file.close();
    }
    // If file does not exist or is empty, highestScore remains 0
}

void SnakeGame::saveHighestScore() {
    ofstream file(HIGHEST_SCORE_FILE);
    if (file.is_open()) {
        file << highestScore;
        file.close();
    }
}

void SnakeGame::playEatSound() {
    eatSound.play(); // Play the sound effect
}

void SnakeGame::run() {
    Clock clock;
    Time elapsed;

    while (window.isOpen()) {
        processInput();
        if (gameend) {
            render(); // Render the game over screen
        } else {
            elapsed = clock.restart();
            update();
            render();
            sleep(milliseconds(100)); // Adjust game speed here
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Ensure proper seed for random
    SnakeGame game;
    game.run();
    return 0;
}
