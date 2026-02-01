#include <iostream>
#include <iomanip>
#include <conio.h>
#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <cstring>
#include <string>

using namespace std;

// CONSTANTS 
const int MAX_SIZE = 10;
const int EASY_SIZE = 8;
const int HARD_SIZE = 10;
const int EASY_TIME = 60;
const int HARD_TIME = 40;
const int EASY_CANDY_COUNT = 5;
const int HARD_CANDY_COUNT = 7;
const char EASY_CANDIES[] = { '@','#','&','$','%' };
const char HARD_CANDIES[] = { '@','#','&','$','%','!','*' };
const int SCORE_MATCH_3 = 10;
const int SCORE_MATCH_4 = 15;
const int SCORE_MATCH_5 = 20;
const int SCORE_L_SHAPE = 25;

// FUNCTION PROTOTYPES
bool LoadGame(char board[MAX_SIZE][MAX_SIZE], int &score, int &remainingTime, int mode);
void SaveGame(const char board[MAX_SIZE][MAX_SIZE], int score, int remainingTime, int mode, int size);
void SaveScore(const char name[], int score, int mode);
void GenerateBoard(char board[MAX_SIZE][MAX_SIZE], int size, const char candies[], int candyCount);
void PrintBoard(const char board[MAX_SIZE][MAX_SIZE], int size);
int CandyColor(char candy);
void RemoveInitialMatches(char board[MAX_SIZE][MAX_SIZE], int size, const char candies[], int candyCount);
void SwapCandies(char board[MAX_SIZE][MAX_SIZE], int r1, int c1, int r2, int c2);
void ApplyGravity(char board[MAX_SIZE][MAX_SIZE], int size, const char candies[], int candyCount);
bool CheckMatches(char board[MAX_SIZE][MAX_SIZE], int size, int &score);
void PlayerMove(char board[MAX_SIZE][MAX_SIZE], int size, int &score, int &remainingTime, bool &pauseGame, int mode);
void PlayGame(int mode);
void LoadAndPlay(int mode);
void MainMenu();
void DisplayHighScores();
void DisplayWelcomeScreen();

// FILE OPERATIONS

//Loads a saved game from file based on mode
// Returns true if successful, false otherwise
bool LoadGame(char board[MAX_SIZE][MAX_SIZE], int &score, int &remainingTime, int mode) {
	string filename = (mode == 1) ? "SavedGameEasy.txt" : "SavedGameHard.txt";
	ifstream file(filename);

	if (!file.is_open()) {
		return false;
	}

	int savedMode, rows, cols;
	file >> savedMode >> rows >> cols >> score >> remainingTime;

	int expectedSize = (mode == 1) ? EASY_SIZE : HARD_SIZE;

	// Validate loaded data
	if (savedMode != mode || rows != expectedSize || cols != expectedSize || score < 0 || remainingTime <= 0) {
		file.close();
		return false;
	}

	file.ignore();

	// Read board state from file
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			file.get(board[r][c]);
		}
		file.ignore();
	}

	file.close();
	return true;
}

//Saves the current game state to file
void SaveGame(const char board[MAX_SIZE][MAX_SIZE], int score, int remainingTime, int mode, int size) {
	string filename = (mode == 1) ? "SavedGameEasy.txt" : "SavedGameHard.txt";
	ofstream file(filename, ios::trunc);

	if (!file.is_open()) {
		return;
	}

	file << mode << " " << size << " " << size << " " << score << " " << remainingTime << endl;

	// Write board state to file
	for (int r = 0; r < size; r++) {
		for (int c = 0; c < size; c++) {
			file << board[r][c];
		}
		file << endl;
	}

	file.close();
	cout << ((mode == 1) ? "Easy" : "Hard") << " game saved successfully!" << endl;
}

//Saves player score to the appropriate high score file

void SaveScore(const char name[], int score, int mode) {
	string filename = (mode == 1) ? "highestScoreEasy.txt" : "highestScoreHard.txt";
	ofstream file(filename, ios::app);

	if (file.is_open()) {
		// Remove spaces from name
		char cleanName[50];
		int writePos = 0;

		for (int i = 0; name[i] != '\0'; i++) {
			if (name[i] != ' ') {
				cleanName[writePos++] = name[i];
			}
		}
		cleanName[writePos] = '\0';

		file << cleanName << " " << score << endl;
		file.close();
	}
}

// BOARD GENERATION
//Generates random candies on the board
void GenerateBoard(char board[MAX_SIZE][MAX_SIZE], int size, const char candies[], int candyCount) {
	for (int r = 0; r < size; r++) {
		for (int c = 0; c < size; c++) {
			int candyIndex = rand() % candyCount;
			board[r][c] = candies[candyIndex];
		}
	}
}

//Returns the color code for console display based on candy type
int CandyColor(char candy) {
	switch (candy) {
	case '@': return 12; // Red
	case '#': return 9;  // Blue
	case '&': return 10; // Green
	case '$': return 14; // Yellow
	case '%': return 13; // Magenta
	case '!': return 11; // Cyan
	case '*': return 15; // White
	default:  return 7;  // Gray
	}
}

//Displays the board with colored candies
void PrintBoard(const char board[MAX_SIZE][MAX_SIZE], int size) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	cout << "   ";

	// Print column numbers
	for (int c = 0; c < size; c++) {
		cout << c << "  ";
	}
	cout << endl;

	// Print each row with row number
	for (int r = 0; r < size; r++) {
		cout << r << " ";
		for (int c = 0; c < size; c++) {
			int color = CandyColor(board[r][c]);
			SetConsoleTextAttribute(h, color);
			cout << "[" << board[r][c] << "]";
			SetConsoleTextAttribute(h, 7); // Reset to default color
		}
		cout << endl;
	}
}

// INITIAL MATCH REMOVAL

//Ensures no initial 3+ matches exist on board at game start
void RemoveInitialMatches(char board[MAX_SIZE][MAX_SIZE], int size, const char candies[], int candyCount) {
	for (int r = 0; r < size; r++) {
		for (int c = 0; c < size; c++) {
			char newCandy;
			bool hasMatch;

			// Keep trying new candies until no match is found
			do {
				newCandy = candies[rand() % candyCount];
				board[r][c] = newCandy;

				// Check for horizontal match (3 in a row)
				bool horizontalMatch = (c >= 2 && board[r][c] == board[r][c - 1] && board[r][c] == board[r][c - 2]);

				// Check for vertical match (3 in a column)
				bool verticalMatch = (r >= 2 && board[r][c] == board[r - 1][c] && board[r][c] == board[r - 2][c]);

				hasMatch = horizontalMatch || verticalMatch;

			} while (hasMatch);
		}
	}
}

// CANDY SWAPPING

//Swaps two candies on the board
void SwapCandies(char board[MAX_SIZE][MAX_SIZE], int r1, int c1, int r2, int c2) {
	char temp = board[r1][c1];
	board[r1][c1] = board[r2][c2];
	board[r2][c2] = temp;
}

// GRAVITY SYSTEM 
//Applies gravity to board: moves candies down and spawns new ones at top
void ApplyGravity(char board[MAX_SIZE][MAX_SIZE], int size, const char candies[], int candyCount) {
	// Process each column from bottom to top
	for (int c = 0; c < size; c++) {
		for (int r = size - 1; r >= 0; r--) {
			if (board[r][c] == ' ') { // Empty cell found
									  // Find the next candy above this empty cell
				int r2 = r - 1;
				while (r2 >= 0 && board[r2][c] == ' ') {
					r2--;
				}

				if (r2 >= 0) {
					// Move existing candy down
					board[r][c] = board[r2][c];
					board[r2][c] = ' ';
				}
				else {
					// Spawn new candy at top (avoiding immediate 3-match)
					char newCandy;
					bool valid;

					do {
						newCandy = candies[rand() % candyCount];
						valid = true;

						// Check if new candy creates vertical match below
						if (r >= 2 && board[r - 1][c] == newCandy && board[r - 2][c] == newCandy) {
							valid = false;
						}

						// Check if new candy creates horizontal match
						if (c >= 2 && board[r][c - 1] == newCandy && board[r][c - 2] == newCandy) {
							valid = false;
						}

						// Check downward vertical match
						if (r <= size - 3 && board[r + 1][c] == newCandy && board[r + 2][c] == newCandy) {
							valid = false;
						}

					} while (!valid);

					board[r][c] = newCandy;
				}
			}
		}
	}
}

// MATCH DETECTION 

//Checks for matches on board and updates score
//Returns true if any matches were found
bool CheckMatches(char board[MAX_SIZE][MAX_SIZE], int size, int &score) {
	bool foundMatch = false;

	// Check horizontal matches (left to right)
	for (int r = 0; r < size; r++) {
		for (int c = 0; c < size - 2; c++) {
			if (board[r][c] != ' ' && board[r][c] == board[r][c + 1] && board[r][c] == board[r][c + 2]) {
				foundMatch = true;
				char candy = board[r][c];
				int count = 3;

				// Check for 4-match
				if (c + 3 < size && board[r][c + 3] == candy) {
					count++;
				}

				// Check for 5-match
				if (c + 4 < size && board[r][c + 4] == candy) {
					count++;
				}

				// Add score based on match length
				if (count == 3) {
					score += SCORE_MATCH_3;
				}
				else if (count == 4) {
					score += SCORE_MATCH_4;
				}
				else if (count == 5) {
					score += SCORE_MATCH_5;
				}

				// Remove matched candies
				for (int i = 0; i < count; i++) {
					board[r][c + i] = ' ';
				}
			}
		}
	}

	// Check vertical matches (top to bottom)
	for (int c = 0; c < size; c++) {
		for (int r = 0; r < size - 2; r++) {
			if (board[r][c] != ' ' && board[r][c] == board[r + 1][c] && board[r][c] == board[r + 2][c]) {
				foundMatch = true;
				char candy = board[r][c];
				int count = 3;

				// Check for 4-match
				if (r + 3 < size && board[r + 3][c] == candy) {
					count++;
				}

				// Check for 5-match
				if (r + 4 < size && board[r + 4][c] == candy) {
					count++;
				}

				// Add score based on match length
				if (count == 3) {
					score += SCORE_MATCH_3;
				}
				else if (count == 4) {
					score += SCORE_MATCH_4;
				}
				else if (count == 5) {
					score += SCORE_MATCH_5;
				}

				// Remove matched candies
				for (int i = 0; i < count; i++) {
					board[r + i][c] = ' ';
				}
			}
		}
	}

	// Check L-shape matches (all 4 orientations)
	for (int r = 0; r < size - 2; r++) {
		for (int c = 0; c < size - 2; c++) {
			char candy = board[r][c];

			// L-shape Type 1: vertical down, horizontal right
			if (candy != ' ' && board[r + 1][c] == candy && board[r + 2][c] == candy &&
				board[r + 2][c + 1] == candy && board[r + 2][c + 2] == candy) {
				foundMatch = true;
				score += SCORE_L_SHAPE;
				board[r][c] = board[r + 1][c] = board[r + 2][c] = board[r + 2][c + 1] = board[r + 2][c + 2] = ' ';
			}

			// L-shape Type 2: vertical down, horizontal left (mirrored)
			if (candy != ' ' && board[r + 1][c + 2] == candy && board[r + 2][c + 2] == candy &&
				board[r + 2][c] == candy && board[r + 2][c + 1] == candy) {
				foundMatch = true;
				score += SCORE_L_SHAPE;
				board[r][c + 2] = board[r + 1][c + 2] = board[r + 2][c + 2] = board[r + 2][c] = board[r + 2][c + 1] = ' ';
			}

			// L-shape Type 3: horizontal right, vertical down
			if (candy != ' ' && board[r][c + 1] == candy && board[r][c + 2] == candy &&
				board[r + 1][c] == candy && board[r + 2][c] == candy) {
				foundMatch = true;
				score += SCORE_L_SHAPE;
				board[r][c] = board[r][c + 1] = board[r][c + 2] = board[r + 1][c] = board[r + 2][c] = ' ';
			}

			// L-shape Type 4: horizontal right, vertical down (corner at top-right)
			if (candy != ' ' && board[r][c + 2] == candy && board[r + 1][c + 2] == candy &&
				board[r + 2][c + 2] == candy && board[r][c] == candy && board[r][c + 1] == candy) {
				foundMatch = true;
				score += SCORE_L_SHAPE;
				board[r][c + 2] = board[r + 1][c + 2] = board[r + 2][c + 2] = board[r][c] = board[r][c + 1] = ' ';
			}
		}
	}

	return foundMatch;
}

// PLAYER MOVES 

//Handles a single player move
//Allows pausing by entering 'P'
void PlayerMove(char board[MAX_SIZE][MAX_SIZE], int size, int &score, int &remainingTime, bool &pauseGame, int mode) {
	int r = -1, c = -1;
	char dir;
	char inputChar;

	// Get row input
	while (true) {
		cout << "\nEnter the row (0-" << size - 1 << ") or P to pause: ";
		cin >> inputChar;

		if (inputChar == 'P' || inputChar == 'p') {
			SaveGame(board, score, remainingTime, mode, size);
			pauseGame = true;
			return;
		}

		r = inputChar - '0';
		if (r >= 0 && r < size) {
			c = -1;

			// Get column input
			while (true) {
				cout << "\nEnter the column (0-" << size - 1 << ") or P to pause: ";
				cin >> inputChar;

				if (inputChar == 'P' || inputChar == 'p') {
					SaveGame(board, score, remainingTime, mode, size);
					pauseGame = true;
					return;
				}

				c = inputChar - '0';
				if (c >= 0 && c < size) {
					dir = ' ';

					// Get direction input
					while (true) {
						cout << "\nEnter direction to swap (W/A/S/D) or P to pause: ";
						cin >> dir;

						if (dir == 'P' || dir == 'p') {
							SaveGame(board, score, remainingTime, mode, size);
							pauseGame = true;
							return;
						}

						if (dir == 'W' || dir == 'w' || dir == 'A' || dir == 'a' ||
							dir == 'S' || dir == 's' || dir == 'D' || dir == 'd') {

							// Calculate target position
							int r2 = r, c2 = c;
							if (dir == 'W' || dir == 'w') r2 = r - 1;
							else if (dir == 'S' || dir == 's') r2 = r + 1;
							else if (dir == 'A' || dir == 'a') c2 = c - 1;
							else if (dir == 'D' || dir == 'd') c2 = c + 1;

							// Validate target position
							if (r2 < 0 || r2 >= size || c2 < 0 || c2 >= size) {
								cout << "Cannot move outside the grid.\n";
								return;
							}

							// Swap candies
							SwapCandies(board, r, c, r2, c2);

							// Get candy type info for gravity
							const char* candies = (mode == 1) ? EASY_CANDIES : HARD_CANDIES;
							int candyCount = (mode == 1) ? EASY_CANDY_COUNT : HARD_CANDY_COUNT;

							// Process matches and gravity
							while (CheckMatches(board, size, score)) {
								ApplyGravity(board, size, candies, candyCount);
							}

							// Display updated board
							PrintBoard(board, size);
							cout << "Score: " << score << endl;
							return;
						}
						cout << "Invalid direction. Try again.\n";
					}
				}
				cout << "Invalid column. Try again.\n";
			}
		}
		cout << "Invalid row. Try again.\n";
	}
}

// GAME MODES 

// Main game loop for both Easy and Hard modes
void PlayGame(int mode) {
	cout << "You chose " << ((mode == 1) ? "Easy" : "Hard") << " Mode!" << endl;

	int size = (mode == 1) ? EASY_SIZE : HARD_SIZE;
	int gameTime = (mode == 1) ? EASY_TIME : HARD_TIME;
	const char* candies = (mode == 1) ? EASY_CANDIES : HARD_CANDIES;
	int candyCount = (mode == 1) ? EASY_CANDY_COUNT : HARD_CANDY_COUNT;

	char board[MAX_SIZE][MAX_SIZE];
	int score = 0;
	bool pauseGame = false;

	// Generate and initialize board
	GenerateBoard(board, size, candies, candyCount);
	RemoveInitialMatches(board, size, candies, candyCount);
	PrintBoard(board, size);

	auto start = chrono::high_resolution_clock::now();

	// Game loop
	while (true) {
		// Calculate remaining time
		auto now = chrono::high_resolution_clock::now();
		int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
		int remaining = gameTime - elapsed;

		// Check if time is up
		if (remaining <= 0) {
			cout << endl << "Time is up!" << endl;
			cout << "Final Score: " << score << endl;
			char name[50];
			cin.ignore();
			cout << "Enter your name to save score: ";
			cin.getline(name, 50);
			SaveScore(name, score, mode);
			return;
		}

		// Display time and get player move
		cout << "Time remaining: " << remaining << "s" << endl;
		PlayerMove(board, size, score, remaining, pauseGame, mode);

		// Check if game was paused
		if (pauseGame) {
			return;
		}
	}
}

//Loads and resumes a saved game
void LoadAndPlay(int mode) {
	char board[MAX_SIZE][MAX_SIZE];
	int score = 0, remainingTime = 0;
	int size = (mode == 1) ? EASY_SIZE : HARD_SIZE;
	const char* candies = (mode == 1) ? EASY_CANDIES : HARD_CANDIES;
	int candyCount = (mode == 1) ? EASY_CANDY_COUNT : HARD_CANDY_COUNT;

	if (LoadGame(board, score, remainingTime, mode)) {
		cout << ((mode == 1) ? "Easy" : "Hard") << " game loaded successfully!\n";
		cout << "Score: " << score << " | Time Remaining: " << remainingTime << "s\n";
		PrintBoard(board, size);

		bool pauseGame = false;
		auto start = chrono::high_resolution_clock::now();

		// Resume game loop
		while (true) {
			auto now = chrono::high_resolution_clock::now();
			int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
			int remaining = remainingTime - elapsed;

			if (remaining <= 0) {
				cout << "\nTime is up!" << endl;
				cout << "Final Score: " << score << endl;
				char name[50];
				cin.ignore();
				cout << "Enter your name to save score: ";
				cin.getline(name, 50);
				SaveScore(name, score, mode);
				return;
			}

			cout << "Time remaining: " << remaining << "s" << endl;
			PlayerMove(board, size, score, remaining, pauseGame, mode);

			if (pauseGame) {
				return;
			}
		}
	}
	else {
		cout << "No saved " << ((mode == 1) ? "Easy" : "Hard") << " game found.\n";
	}
}

// MENU AND DISPLAY

//Displays top 10 high scores for selected mode
void DisplayHighScores() {
	int mode;
	cout << "Which mode high scores?\n1. Easy Mode\n2. Hard Mode\nEnter: ";
	cin >> mode;

	string filename = (mode == 1) ? "highestScoreEasy.txt" : "highestScoreHard.txt";
	string modeText = (mode == 1) ? "EASY" : "HARD";

	ifstream file(filename);

	if (!file.is_open()) {
		cout << "No " << modeText << " high scores file found!" << endl;
		return;
	}

	char names[100][50];
	int scores[100];
	int count = 0;

	// Read all scores from file
	while (count < 100 && file >> names[count] >> scores[count]) {
		count++;
	}
	file.close();

	if (count == 0) {
		cout << "No " << modeText << " high scores yet!" << endl;
		return;
	}

	// Bubble sort scores in descending order
	for (int i = 0; i < count - 1; i++) {
		for (int j = 0; j < count - i - 1; j++) {
			if (scores[j] < scores[j + 1]) {
				// Swap scores
				int tempScore = scores[j];
				scores[j] = scores[j + 1];
				scores[j + 1] = tempScore;

				// Swap names
				char tempName[50];
				strcpy_s(tempName, names[j]);
				strcpy_s(names[j], names[j + 1]);
				strcpy_s(names[j + 1], tempName);
			}
		}
	}

	// Display scores
	if (count < 10) {
		cout << "\n=== " << modeText << " MODE HIGH SCORES (Only " << count << " saved) ===\n";
	}
	else {
		cout << "\n=== TOP 10 " << modeText << " MODE HIGH SCORES ===\n";
	}

	int limit = (count < 10) ? count : 10;
	for (int i = 0; i < limit; i++) {
		cout << i + 1 << ". " << names[i] << " - " << scores[i] << endl;
	}
}

//Displays welcome screen
void DisplayWelcomeScreen() {
	system("Color D7");
	cout << setfill('*') << setw(44) << " " << endl;
	cout << "*                                         *" << endl;
	cout << "*          WELCOME TO CANDY CRUSH         *" << endl;
	cout << "*                                         *" << endl;
	cout << setfill('*') << setw(44) << " " << endl;
	cout << "\nPress any key to continue...";
	_getch();
	system("cls");
}

//Displays main menu and handles player choices
void MainMenu() {
	int choice;

	do {
		cout << "Choose from one of the options below:" << endl;
		cout << "1. Easy Mode" << endl;
		cout << "2. Hard Mode" << endl;
		cout << "3. High Score" << endl;
		cout << "4. Load Game" << endl;
		cout << "5. Exit" << endl;
		cin >> choice;

		if (choice == 1) {
			PlayGame(1); // Easy Mode
		}
		else if (choice == 2) {
			PlayGame(2); // Hard Mode
		}
		else if (choice == 3) {
			cout << "High scores loading..." << endl;
			DisplayHighScores();
		}
		else if (choice == 4) {
			int mode;
			cout << "1. Load Easy Game\n2. Load Hard Game\nEnter: ";
			cin >> mode;

			if (mode == 1 || mode == 2) {
				LoadAndPlay(mode);
			}
			else {
				cout << "Invalid choice.\n";
			}
		}
		else if (choice == 5) {
			cout << "Exiting game..." << endl;
		}
		else {
			cout << "Invalid choice!" << endl;
		}

		// Wait for key press before returning to menu
		if (choice != 5) {
			cout << "\nPress any key to return to menu...";
			_getch();
			system("cls");
		}

	} while (choice != 5);
}

// MAIN FUNCTION 

//Main function - entry point of program
int main() {
	srand(time(0)); // Initialize random seed
	DisplayWelcomeScreen();
	MainMenu();
	system("pause");
	return 0;
}