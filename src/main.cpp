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
#define WALL_WIDTH 1 // Width of the wall sections
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
    void generateFruit();
    void displayGameOverScreen();
    void loadHighestScore();
    void saveHighestScore();
    void playEatSound(); // Function to play the eating sound

    RenderWindow window;
    RectangleShape snakeSegment;
    RectangleShape fruit;
    RectangleShape wallSegment; // For drawing wall segments
    Font font;
    Text gameOverText;
    Text fruitEatenText;
    Text replayInstructionsText;
    Text highestScoreText;
    Text scoreText; // Text object to display the score
    SoundBuffer eatSoundBuffer; // Buffer to hold the sound
    Sound eatSound; // Sound object to play the sound

    int width, height, x, y, fruitx, fruity, score, piece, fruitsEaten;
    int highestScore; // Highest score
    Direction currentDirection, nextDirection;
    bool gameend;
    vector<Vector2i> tail;
    vector<RectangleShape> walls; // List of wall segments

    void setupWalls(); // Function to set up the walls
    void initializeSnake(); // Function to initialize the snake

    // Added for speed control
    float snakeSpeed; // The speed of the snake in seconds
    Clock clock; // SFML clock for timing
    Time timeSinceLastMove; // Time elapsed since the last move
};

SnakeGame::SnakeGame()
    : window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game"),
      snakeSegment(Vector2f(GRID_SIZE, GRID_SIZE)),
      fruit(Vector2f(GRID_SIZE, GRID_SIZE)),
      wallSegment(Vector2f(WALL_WIDTH, WALL_WIDTH)),
      width(WINDOW_WIDTH / GRID_SIZE), height(WINDOW_HEIGHT / GRID_SIZE),
      x(width / 2), y(height / 2), score(0), piece(0), fruitsEaten(0),
      highestScore(0), // Initialize highestScore here
      currentDirection(RIGHT), nextDirection(RIGHT), gameend(false), 
      snakeSpeed(0.2f) // Set the snake speed (adjust as needed)
{
    snakeSegment.setFillColor(Color::Green);
    fruit.setFillColor(Color::Red);
    wallSegment.setFillColor(Color::Yellow); // Set wall color

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

    scoreText.setFont(font); // Initialize the scoreText object
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(Color::White);

    loadHighestScore(); // Load the highest score from file
    setup();
}

void SnakeGame::setup() {
    initializeSnake(); // Initialize the snake to avoid self-collision
    generateFruit();
    score = 0;
    piece = 0;
    fruitsEaten = 0;
    tail.clear();
    gameend = false;
    currentDirection = RIGHT;
    nextDirection = RIGHT;
    setupWalls(); // Initialize wall positions

    clock.restart(); // Reset the clock at the start
    timeSinceLastMove = Time::Zero; // Reset the time counter
}

void SnakeGame::initializeSnake() {
    // Ensure the snake starts in a valid position
    x = width / 2;
    y = height / 2;
    tail.clear();
    tail.push_back(Vector2i(x, y));
    // Add a second segment to start with, ensuring it's not on top of the head
    tail.push_back(Vector2i(x - 1, y));
}

void SnakeGame::setupWalls() {
    walls.clear();
    
    // Create horizontal walls
    for (int i = 0; i < WINDOW_WIDTH / WALL_WIDTH; i++) {
        // Top wall
        wallSegment.setPosition(i * WALL_WIDTH, 0);
        walls.push_back(wallSegment);
        // Bottom wall
        wallSegment.setPosition(i * WALL_WIDTH, WINDOW_HEIGHT - WALL_WIDTH);
        walls.push_back(wallSegment);
    }

    // Create vertical walls
    for (int i = 0; i < WINDOW_HEIGHT / WALL_WIDTH; i++) {
        // Left wall
        wallSegment.setPosition(0, i * WALL_WIDTH);
        walls.push_back(wallSegment);
        // Right wall
        wallSegment.setPosition(WINDOW_WIDTH - WALL_WIDTH, i * WALL_WIDTH);
        walls.push_back(wallSegment);
    }
}

void SnakeGame::processInput() {
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed) {
            saveHighestScore(); // Save the highest score before quitting
            window.close();
        }
        if (event.type == Event::KeyPressed) {
            if (gameend) {
                if (event.key.code == Keyboard::Y) {
                    setup(); // Restart the game
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
    Time deltaTime = clock.restart(); // Get the time since the last frame
    timeSinceLastMove += deltaTime;

    if (timeSinceLastMove.asSeconds() >= snakeSpeed) {
        if (!gameend) {
            moveSnake();
        }
        timeSinceLastMove = Time::Zero; // Reset the time since last move
    }
}

void SnakeGame::render() {
    window.clear();

    if (gameend) {
        displayGameOverScreen();
    } else {
        // Draw walls
        for (const auto &wall : walls) {
            window.draw(wall);
        }

        // Draw the snake
        for (const auto &segment : tail) {
            snakeSegment.setPosition(segment.x * GRID_SIZE, segment.y * GRID_SIZE);
            window.draw(snakeSegment);
        }
        
        // Draw the fruit
        fruit.setPosition(fruitx * GRID_SIZE, fruity * GRID_SIZE);
        window.draw(fruit);

        // Update and draw the score
        string scoreTextString = "Score: " + to_string(score);
        scoreText.setString(scoreTextString);
        scoreText.setPosition(10, 10); // Position at the top-left corner with some padding
        window.draw(scoreText); // Draw the score text

        // Update and draw the fruit eaten text
        string fruitEatenTextString = "Fruit Eaten: " + to_string(fruitsEaten);
        fruitEatenText.setString(fruitEatenTextString);
        fruitEatenText.setPosition(10, 50); // Position at the top-left corner below the score
        window.draw(fruitEatenText); // Draw the fruit eaten text
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
    
    // Check wall collision
    for (const auto &wall : walls) {
        IntRect wallBounds(wall.getPosition().x, wall.getPosition().y, WALL_WIDTH, WALL_WIDTH);
        if (wallBounds.contains(x * GRID_SIZE, y * GRID_SIZE)) {
            gameend = true;
            // Check if the current score is higher than the highest score
            if (score > highestScore) {
                highestScore = score; // Update highest score
                saveHighestScore(); // Save the new highest score
            }
            break;
        }
    }

    // Check boundary collision
    if (x < 0 || x >= width || y < 0 || y >= height) {
        gameend = true;
        // Check if the current score is higher than the highest score
        if (score > highestScore) {
            highestScore = score; // Update highest score
            saveHighestScore(); // Save the new highest score
        }
    }

    // Check self-collision
    for (size_t i = 1; i < tail.size(); ++i) { // Start from 1 to skip the head itself
        if (tail[i].x == x && tail[i].y == y) {
            gameend = true;
            // Check if the current score is higher than the highest score
            if (score > highestScore) {
                highestScore = score; // Update highest score
                saveHighestScore(); // Save the new highest score
            }
            break;
        }
    }

    // Check if snake has eaten the fruit
    if (x == fruitx && y == fruity) {
        score += 10;
        piece++;
        fruitsEaten++;
        generateFruit();
        playEatSound(); // Play sound when eating fruit
    }
}

void SnakeGame::generateFruit() {
    fruitx = rand() % width;
    fruity = rand() % height;
}

void SnakeGame::displayGameOverScreen() {
    // Display game over text
    gameOverText.setPosition(WINDOW_WIDTH / 2 - gameOverText.getLocalBounds().width / 2, WINDOW_HEIGHT / 2 - gameOverText.getLocalBounds().height / 2);
    window.draw(gameOverText);

    // Display fruit eaten text
    string fruitEatenTextString = "Fruit Eaten: " + to_string(fruitsEaten);
    fruitEatenText.setString(fruitEatenTextString);
    fruitEatenText.setPosition(10, 50); // Position at the top-left corner below the score
    window.draw(fruitEatenText);

    // Display highest score text
    string highestScoreTextString = "Highest Score: " + to_string(highestScore);
    highestScoreText.setString(highestScoreTextString);
    highestScoreText.setPosition(WINDOW_WIDTH / 2 - highestScoreText.getLocalBounds().width / 2, WINDOW_HEIGHT / 2 + 100);
    window.draw(highestScoreText);

    // Display replay instructions text
    replayInstructionsText.setString("Press Y to play again or Q to quit");
    replayInstructionsText.setPosition(WINDOW_WIDTH / 2 - replayInstructionsText.getLocalBounds().width / 2, WINDOW_HEIGHT / 2 + 150);
    window.draw(replayInstructionsText);
}

void SnakeGame::loadHighestScore() {
    ifstream file(HIGHEST_SCORE_FILE);
    if (file.is_open()) {
        file >> highestScore;
        if (file.fail()) {
            highestScore = 0;
        }
        file.close();
        cout << "Loaded highest score: " << highestScore << endl; // Debug statement
    } else {
        highestScore = 0;
        cout << "File not found. Initialized highest score to 0." << endl; // Debug statement
    }
}

void SnakeGame::saveHighestScore() {
    ofstream file(HIGHEST_SCORE_FILE);
    if (file.is_open()) {
        file << highestScore;
        file.close();
        cout << "Saved highest score: " << highestScore << endl; // Debug statement
    } else {
        cerr << "Failed to save highest score" << endl;
    }
}

void SnakeGame::playEatSound() {
    eatSound.play(); // Play the eating sound
}


void SnakeGame::run() {
    while (window.isOpen()) {
        processInput();
        if (gameend) {
            render(); // Render the game over screen
        } else {
            update();
            render();
            sleep(milliseconds(1)); // Adjust game speed here
        }
    }
}

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
