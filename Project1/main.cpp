#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

//Declare colours to be used in the game, 'retro' feeling means only two are needed 
Color backgroundColour = { 68, 141, 206, 255 };
Color accentColour = { 57, 2, 52, 255 };

//Declaring the size of the grid squares and the number of squares in each row.
float cellSize = 30;
int cellCount = 25;
// Offset for the border around the play area.
int offset = 75;

double lastUpdateTime = 0;
//a function to determine the speed at which the snake should update, using an interval provided.
bool eventTriggered(double interval)
{
	double currentTime = GetTime();
	
	if (currentTime - lastUpdateTime >= interval)
	{
		lastUpdateTime = currentTime;
		return true;
	}

	return false;
}
//Function for checking if an element would exist within a deque, allowing us to check if the food would spawn within the snake.
bool elementInDeque(Vector2 element, deque<Vector2> deque)
{
	for (unsigned int i = 0; i < deque.size(); i++)
	{
		if (Vector2Equals(deque[i], element))
		{
			return true;
		}
	}
	return false;
}

//Creating the class for the food that the snake will eat.
class SnakeFood 
{
public:
	//Declaring variables for grid position and the texture that will be used for the image representing the food, to differentiate it from the squares of the snake.
	Vector2 position;
	Texture2D foodTexture;

	//Constructor
	SnakeFood(deque<Vector2> snakeBody) 
	{
		//Load the image file, convert it to a texture which can be rendered by raylib, and unload to save space.
		Image foodImage = LoadImage("Graphics/food.png");
		foodTexture = LoadTextureFromImage(foodImage);
		UnloadImage(foodImage);
		position = GenerateRandomPos(snakeBody);
	}

	//Destructor
	~SnakeFood()
	{
		UnloadTexture(foodTexture);
	}

	//Method to draw the food object.
	void Draw() 
	{
		DrawTexture(foodTexture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
	}
	//Method to generate a random cell on the grid.
	Vector2 GenerateRandomCell()
	{
		float x = GetRandomValue(0, cellCount - 1);
		float y = GetRandomValue(0, cellCount - 1);

		return Vector2{ x, y };
	}
	//Method to generate a random position for the food object, such that it doesn't generate in the same spot each time.
	//Also checks if the food would spawn inside the snake, generating a new position if it would.
	Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
	{
		
		Vector2 position = GenerateRandomCell();

		while (elementInDeque(position, snakeBody))
		{
			position = GenerateRandomCell();
		}

		return position;
	}
};
//Creating the class for the snake itself.
class Snake
{
public:
	//Declaring the snake's body, direction and whether we should add a segment.
	deque<Vector2> snakeBody = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
	Vector2 direction = { 1,0 };
	Vector2 directionBeforeUpdate = { 1,0 };
	bool addSegment = false;

	//Creating a variable to be able to set the snake's speed, and a variable to keep track of the last score value the speed was updated.
	double snakeSpeedInterval = 0.2;
	int previousScoreUpdated = 0;

	//method to draw the snake, using a for loop to draw each element wihtin the deque that defines it's body
	void Draw()
	{
		for (unsigned int i = 0; i < snakeBody.size(); i++)
		{
			float x = snakeBody[i].x;
			float y = snakeBody[i].y;
			Rectangle segment = Rectangle{ offset + x * cellSize, offset + y * cellSize, cellSize, cellSize };
			DrawRectangleRounded(segment, 0.5, 6, accentColour);
		}
	}
	//Method to update the snake's position, using the pop_back and push_front for the deque to remove the back segment and add a new front, giving the illusion of movement.
	void Update()
	{
		snakeBody.push_front(Vector2Add(snakeBody[0], direction));
		if (addSegment == true)
		{
			addSegment = false;
		}
		else
		{
			direction = directionBeforeUpdate;
			snakeBody.pop_back();
		}

	}
	//Method to reset the position of the snake, as well as the speed and speed update scores.
	void Reset()
	{
		snakeBody = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
		direction = { 1, 0 };
		snakeSpeedInterval = 0.2;
		previousScoreUpdated = 0;
	}

};
//Creating a Game class, to hold all of the objects within the game and the logic for those objects.
class Game
{
public:
	//initialising the game objects and setting score and running values
	Snake snake = Snake();
	SnakeFood food = SnakeFood(snake.snakeBody);
	bool running = true;
	int score = 0;

	//initialising the sounds and music
	Sound eatSound;
	Sound hitSound;
	Music backgroundMusic;
	//constructor
	Game()
	{
		InitAudioDevice();
		//Recorded clips of my own voice for the sfx.
		eatSound = LoadSound("Sounds/eat.mp3");
		hitSound = LoadSound("Sounds/hit.mp3");
		//"Klockworx" by Kevin MacLeod (incompetech.com)
		backgroundMusic = LoadMusicStream("Sounds/Klockworx.mp3");
		SetMusicVolume(backgroundMusic, 0.2f);
	}
	//destructor
	~Game()
	{
		UnloadSound(eatSound);
		UnloadSound(hitSound);
		UnloadMusicStream(backgroundMusic);
		CloseAudioDevice();
	}

	//method to draw all game objects
	void Draw()
	{
		if (running)
		{
			food.Draw();
		}
		snake.Draw();
	}
	//method to update all game objects, and call collision checks
	void Update()
	{
		if (running)
		{
			snake.Update();
			FoodCollisionCheck();
			EdgeCollisionCheck();
			SelfCollisionCheck();
		}
		
	}
	//Method to check collision between snake and food, and to both play the eating sound and generate a new food in a different spot when it is eaten.
	void FoodCollisionCheck()
	{
		if (Vector2Equals(snake.snakeBody[0], food.position))
		{
			cout << "Eating food..." << endl;
			food.position = food.GenerateRandomPos(snake.snakeBody);
			snake.addSegment = true;
			score++;
			PlaySound(eatSound);
		}
	}
	//Method to check for the snake colliding with the walls of the play area, calling the Game Over method if it does.
	void EdgeCollisionCheck()
	{
		if (snake.snakeBody[0].x == cellCount || snake.snakeBody[0].x == -1)
		{
			GameOver();
		}
		if (snake.snakeBody[0].y == cellCount || snake.snakeBody[0].y == -1)
		{
			GameOver();
		}
	}
	//Method to check for the snake colliding with itself, calling the Game Over method if it does.
	void SelfCollisionCheck()
	{
		deque<Vector2> headlessSnakeBody = snake.snakeBody;
		headlessSnakeBody.pop_front();
		if (elementInDeque(snake.snakeBody[0], headlessSnakeBody))
		{
			GameOver();
		}
	}
	//Method to bring the game into a game over state, resetting the position of snake and food, setting the game to pause, setting the score to zero, playing the hit sound
	//and also setting the volume of the music to lower.
	void GameOver()
	{
		cout << "Game Over!" << endl;
		snake.Reset();
		food.position = food.GenerateRandomPos(snake.snakeBody);
		running = false;
		score = 0;
		PlaySound(hitSound);
		SetMusicVolume(backgroundMusic, 0.05);
	}
};

int main() 
{

	//Print to console to signal game has started correctly, then initialise the window using raylib.
	cout << "Starting the game!" << endl;
	InitWindow((2 * offset) + (cellSize * cellCount), (2 * offset) + (cellSize * cellCount), "SneakySnekGame");
	SetTargetFPS(60);

	//Creating a Game object, to handle the game logic.
	Game game = Game();
	//Start playing game music.
	PlayMusicStream(game.backgroundMusic);
	

	while (WindowShouldClose() == false) 
	{
		BeginDrawing();
		UpdateMusicStream(game.backgroundMusic);

		//Use the eventTriggered function to slow down the snake's movement.
		if (eventTriggered(game.snake.snakeSpeedInterval))
		{
			game.Update();
		}
		//Use some logic to increase the speed of the snake slightly, if the score is a multiple of ten.
		if (game.score % 10 == 0 && game.score != 0 && game.score != game.snake.previousScoreUpdated)
		{
			game.snake.snakeSpeedInterval = game.snake.snakeSpeedInterval - 0.02;
			game.snake.previousScoreUpdated = game.score;
		}
		//Detecting whether the movement keys are pressed and changing the direction of the snake, with the exception of if the snake is already moving in the opposite direction.
		//Also resumes the game after a game over, and raises the volume of the background music.
		if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) && game.snake.direction.y != 1)
		{
			game.snake.directionBeforeUpdate = { 0, -1 };
			game.running = true;
			SetMusicVolume(game.backgroundMusic, 0.2);
		}
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) && game.snake.direction.y != -1)
		{
			game.snake.directionBeforeUpdate = { 0, 1 };
			game.running = true;
			SetMusicVolume(game.backgroundMusic, 0.2);
		}
		if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) && game.snake.direction.x != 1)
		{
			game.snake.directionBeforeUpdate = { -1, 0 };
			game.running = true;
			SetMusicVolume(game.backgroundMusic, 0.2);
		}
		if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) && game.snake.direction.x != -1)
		{
			game.snake.directionBeforeUpdate = { 1, 0 };
			game.running = true;
			SetMusicVolume(game.backgroundMusic, 0.2);
		}

		//Drawing steps
		ClearBackground(backgroundColour);
		DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, accentColour);
		DrawText("Sneaky Snake Game", offset - 5, 20, 40, accentColour);
		DrawText("Score: ", offset - 5, offset + cellSize * cellCount + 10, 40, accentColour);
		DrawText(TextFormat("%i", game.score), offset - 5 + 135, offset + cellSize * cellCount + 10, 40, accentColour);
		if (game.running == false)
		{
			DrawText("Game Over!", (((cellCount * cellSize)) / 2) - 150, ((offset) + (cellCount * cellSize)) / 2, 80, accentColour);
			DrawText("Press movement keys to restart", (((cellCount * cellSize)) / 2) - 260, (((offset)+(cellCount * cellSize)) / 2) + 85, 40, accentColour);
		}
		
		game.Draw();
		EndDrawing();
	}


	//close the raylib window, and terminate the program
	CloseWindow();
	return 0;
}