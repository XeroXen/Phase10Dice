/* Program name: Phase10DiceFINAL.cpp
*  Author: Jonathon Lewis
*  Date last updated: 12/11/2018
* Purpose: Final project. Implementation of Phase 10 Dice.
*/

#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>

using namespace std;
//We actually ran into some issues with this somehow in this project. Apparently cstdlib sometimes has issues with cout and cin.
//I had to manually access cout and cin with std::cout and std::cin respectively to fix it.

enum DiceColors { ORANGE, RED, BLUE, GREEN };
//DiceColors enum. Used in dice generation.

struct Face {
	int value;
	DiceColors color;
};
//Face struct. Holds the value and color for each die face.

/*I'll note here that I chose to generate dice dynamically at the start of each game, as opposed to randomly
every single time a roll was needed. This allows for consistency of the die for each phase of the game, and allows the user
to play more stratigically in phase 8 specifically, because on any 6 faced die that pulls from 4 potential colors, there will be die
that are advantageous to a particular color.

The generation of the die are still random, so it should fulfill the requirements of the project. More specifics are in the generateDice function.*/

struct Die {
	Face faces[6];
};
//Die struct. Each Die has exactly 6 faces, so there is an array of 6 faces to represent that.

struct Player {
	int turnOrder;
	int number;
	int currentPhase;
	int score;
	int currentTurn;
	int failedPhases[10];
	bool turnFiveBonus = true;
};
//Player struct. Holds onto pertinent information for playing the game and displaying score.

void rollDice(Die[], Face[], int);
void generateDice(Die[]);
void playPhase(Player&, Die[]);
int passedPhase(Player, Face[]);
string colorToText(DiceColors);
void displayDice(Face[]);
bool confirmDie(bool[]);
int highestValue(int[], int);
int checkForSet(int[], int);
int checkForSet(DiceColors[], int[], int size);
int checkForRun(int[], int);
void sortArray(int[], int);
void enterDieValues(string, int[], bool[], Face[], int);
void sortPlayersByTurnOrder(Player[], int);
void showFullDice(Die[], int);
string displayRules(int);
void displayRoundInfo(Player[], int, int);

//We use a lot of functions in this program. They're all described in their bodies.

int main()
{
	srand(time(NULL)); //seed the random number generator.
	Die allDice[10]; //make Dice Array.

	while (true) { //loop forever (until we break it later)

		int playerCount;
		std::cout << "Enter the number of players for this game (1-4): ";
		while (!(std::cin >> playerCount) || playerCount < 1 || playerCount > 4) {
			std::cout << "Invalid input. Try again";
			if (!cin) {
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			std::cout << "\nEnter the number of players for this game (1-4): ";
		}

		//above code prompts the user for a number of players between 1 and 4 (yes, we do have a working solitaire mode!)

		Player players[4];
		for (int i = 0; i < playerCount; i++) {
			players[i].number = i + 1;
			players[i].currentPhase = 1;
			players[i].score = 0;
			players[i].currentTurn = 0;
			for (int x = 0; x < 10; x++) {
				players[i].failedPhases[x] = 0;
			}
		}//initialize all players that are playing.
		if (playerCount > 1) {
			bool hasBeenRolled[6] = { false };

			cout << "\nAll players will roll a high dice to determine turn order";

			int randomTurn;
			bool looped = false;

			int turnOrder[4] = { 0 }; //declare an int array that will determine turn orders for players.
			for (int i = 0; i < playerCount; i++) {
				do {
					if (!looped) cout << "\nPlayer " << i + 1 << " must press enter to roll a die: ";
					if (looped) cout << "\nThat value has already been rolled. Player " << i + 1 << " must roll again.";

					char tmp;
					cin.get(tmp);

					randomTurn = (rand() % 6) + 5;
					cout << "Player " << i + 1 << " has rolled a " << randomTurn;
					//generate a random number between 5 and 10 to simulate a high dice.

					if (hasBeenRolled[randomTurn - 5]) looped = true;
					else looped = false;

				} while (hasBeenRolled[randomTurn - 5]);

				players[i].turnOrder = randomTurn;
				hasBeenRolled[randomTurn - 5] = true;
			}

		}
		else {
			players[0].turnOrder = 1;
		}
		sortPlayersByTurnOrder(players, playerCount);
		//this will sort the players array by turn order so we can simplify the execution of the game.

		int playerTurn = 0; //this is the starting index of the players array. this keeps track of which player's turn it is.

		std::cout << "\n\nTurn order for players: ";
		for (int i = 0; i < playerCount; i++) {
			std::cout << "\nPlayer " << players[i].number;
		}
		std::cout << endl << endl;
		//display turn order for all players.
		//then press a key to generate dice, and show the dice.

		std::cout << "Dice will be randomly generated. Press enter to generate them: ";
		char tmp;
		tmp = std::cin.get();

		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		//input cleanup.

		generateDice(allDice);
		std::cout << endl;
		showFullDice(allDice, 10);
		//generate random dice and display the specifics to the game. This is mostly useful for phase 8 where certain dice are more likely to get specific colors.

		bool gameRunning = true;
		int finalTurn = -1;
		int firstFinish = 0;
		int round = -1;
		//declare and initialize some variables we need. finalTurn and firstFinish are for the extra credit
		//of allowing users to take a final turn after a player has completed phase 10, and the first to finish phase 10 bonus, respectively.

		//round is there to help us with displaying the scores between each... round.

		while (gameRunning) {
			//as long is the game is running, we'll keep taking turns.

			if (playerTurn == finalTurn) gameRunning = false;
			//we set finalTurn equal to the player who first completes phase 10 down below. When it reaches their turn
			//AGAIN, it tells the game to stop.

			else {
				if (finalTurn != -1) cout << "\nThis is Player " << players[playerTurn].number << "'s final turn!\n";
				//display a prompt if this is the player's final allowed turn.

				if (players[playerTurn].currentTurn > round) {
					round++;
					displayRoundInfo(players, round, playerCount);
				}//this if statement is round maintinence, basically. It checks to see if we've encountered a new round, and increments and displays accordingly.

				playPhase(players[playerTurn], allDice);
				//this function actually plays the phase. More information in the function itself.

				if (players[playerTurn].currentPhase == 6) { //if they completed phase 5.
					if (players[playerTurn].score >= 220 && players[playerTurn].turnFiveBonus == true) { //and if their score is high enough (and this check hasn't been done before)
						players[playerTurn].score += 40; //give them bonus score, prompt them, and make it so they can't get the bonus again.
						cout << "Player " << players[playerTurn].number << " has earned 40 points for completing phase 5 with over 220 score!\n";
						players[playerTurn].turnFiveBonus = false;
					}
					else players[playerTurn].turnFiveBonus = false; //eliminate the possibility of them getting the bonus later
				}
				//The above if block all serves as the finishing phase 5 extra credit.


				if (players[playerTurn].currentPhase > 10) { //if they completed the game
					if (finalTurn == -1) { //and nobody else has.
						finalTurn = playerTurn; //set finalTurn equal to the current playerTurn (so we can end the game when we get back to them)
						if (playerCount > 1) { //and in a multiplayer game...
							firstFinish = players[playerTurn].currentTurn; //we set the firstFinish "turn" equal to the player's current turn. So other players who finish on the same turn can get the bonus.
							std::cout << "First Finish Bonus! Player " << players[playerTurn].number << " gets 40 bonus points!\n";
							players[playerTurn].score += 40;
						}//then we simply tell the user they got bonus points and increment accordingly.
					}
					else {
						if (players[playerTurn].currentTurn <= firstFinish) { //if another player finishes the game on the same turn as the first finish user...
							std::cout << "Matched the First Finish Bonus! Player " << players[playerTurn].number << " also gets 40 bonus points!\n";
							players[playerTurn].score += 40;
						}//they will also get bonus points! This works because a player who had gone before the first finish player will have a higher currentTurn than the firstFinish variable
						 //by the time their next turn comes around, making them ineligible for the bonus.
					}
				}

				playerTurn++; //finally, we increment the current player's turn.
			}
			if (playerTurn > playerCount - 1) playerTurn = 0;
			//and make sure we don't go outside the bounds of the players we are playing with.
		}
		//And that's the main game loop.

		//After game display/cleanup.

		if (playerCount > 1) {
			int highestIndex = 0;
			for (int i = 1; i < playerCount; i++) {
				if (players[i].score > players[highestIndex].score) highestIndex = i;
			}

			std::cout << "Player " << players[highestIndex].number << " wins!";
			std::cout << "\nTotal Scores:\n";
			for (int i = 0; i < playerCount; i++) {
				std::cout << "Player " << players[i].number << " score: " << players[i].score << endl;
			}
		}//The above block determines who won the game, and tells the user(s) who won in a multiplayer game.

		else { //This else covers the end of a solitaire game.
			int phasesFailed = 0;

			for (int i = 0; i < 10; i++) {
				phasesFailed += players[0].failedPhases[i];
			}//this for loop will find every phase the player had failed and store it in a counter variable.

			if (players[0].failedPhases[9] == 0) {
				players[0].score += 40;
				cout << "Player 1 earned 40 bonus points for completing phase 10 in one try!\n\n";
			}//we also check to see if they had ever failed phase 10. In order to give them bonus score for finishing it in one try.

			players[0].score -= phasesFailed * 5;
			cout << "Player 1 has completed solitaire mode with a score of " << players[0].score << " and failed " << phasesFailed << " phases";
			//we decrement the score by the amount of phases failed multiplied by 5, and tell the user how they did.
		}

		cin.clear();
		cin.ignore(INT_MAX, '\n');
		//input cleanup. Otherwise it had some issues.

		std::cout << "\nWould you like to play again? (Y/N) ";
		while (true) {
			tmp = std::cin.get();
			if (tmp == 'Y' || tmp == 'y') break;
			else if (tmp == 'N' || tmp == 'n') return 0;
			else {
				std::cout << "Invalid Input. Try again.";
				cin.clear();
				cin.ignore(INT_MAX, '\n');
				std::cout << "\nWould you like to play again ? (Y/N) ";
			}
		}//prompt the user if they would like to play again, and if not end the game.
	}

	return 0;
}

void rollDice(Die diceArray[], Face faceArray[], int index) {
	//this will roll one of the dies in the dice array.
	//and it stores the information of the rolled die in a faceArray.

	//This is where the random factor of each die comes into play. When die are used,
	//we actually roll from their 6 potential faces and use that information.
	int randomFace = rand() % 6;

	faceArray[index].value = diceArray[index].faces[randomFace].value;
	faceArray[index].color = diceArray[index].faces[randomFace].color;

	return;
}

void generateDice(Die dieArray[]) {
	//this function assumes the rules of phase 10. therefore knowing what values to assign to each face.

	int availableColors[4] = { 5,5,5,5 };
	//This array keeps track of which "extra" colors have and haven't been used. This way we keep an equal color distribution
	//across all die.

	int d;
	for (d = 0; d < 4; d++) { //generate low dice

		int position = rand() % GREEN;
		for (int i = 0; i < 4; i++) {
			dieArray[d].faces[i].color = static_cast<DiceColors>(position);
			if (position == GREEN) position = ORANGE;
			else position++;
		}//generates a random "position" to start the first 4 face colors. and then goes through each one, assigning it
		 //to each face.

		int colorIndex = rand() % 4;
		while (availableColors[colorIndex] != highestValue(availableColors, 4)) {
			if (colorIndex == 3) colorIndex = 0;
			else colorIndex++;
		}//we determine a new random color for face 5. we also make sure it's
		 //pulled from one of the colors that have been used least from the "bonus" colors.
		 //this is to ensure equal color distribution and no repeats.

		dieArray[d].faces[4].color = static_cast<DiceColors>(colorIndex);
		availableColors[colorIndex]--;
		//Assigns the color to the face, and decrements the part of availableColors that corresponds to that color by 1.

		//This color generation is done several times so I won't comment it every time.

		colorIndex = rand() % 4;
		while (availableColors[colorIndex] != highestValue(availableColors, 4)) {
			if (colorIndex == 3) colorIndex = 0;
			else colorIndex++;
		}

		dieArray[d].faces[5].color = static_cast<DiceColors>(colorIndex);
		availableColors[colorIndex]--;

		for (int i = 0; i < 6; i++) { //loop through face of each dice.

			if (i > 3) {
				dieArray[d].faces[i].value = 0; //set to be a WILD face.
			}
			else {
				dieArray[d].faces[i].value = i + 1;
				//set the face value of each low dice.
			}
		}//this for loop sets the values of each of the low faces on each low dice.
		 //and ensures we get our wild values generated in the right amount.
	}
	for (d; d < 10; d++) { //generate high dice

		int position = rand() % GREEN;
		for (int i = 0; i < 4; i++) {
			dieArray[d].faces[i].color = static_cast<DiceColors>(position);
			if (position == GREEN) position = ORANGE;
			else position++;
		}

		int colorIndex = rand() % 4;
		while (availableColors[colorIndex] != highestValue(availableColors, 4)) {
			if (colorIndex == 3) colorIndex = 0;
			else colorIndex++;
		}

		dieArray[d].faces[4].color = static_cast<DiceColors>(colorIndex);
		availableColors[colorIndex]--;

		colorIndex = rand() % 4;
		while (availableColors[colorIndex] != highestValue(availableColors, 4)) {
			if (colorIndex == 3) {
				colorIndex = 0;
			}
			else {
				colorIndex++;
			}
		}

		dieArray[d].faces[5].color = static_cast<DiceColors>(colorIndex);
		availableColors[colorIndex]--;

		//All of the above code generates colors identically to how they were done on the low dice. Except for the high dice.
		//the colors are generated randomly.

		for (int i = 0; i < 6; i++) {
			dieArray[d].faces[i].value = i + 5;
		}//this for loop sets the actual value of each face of the high dice.
	}

	return;
}

void playPhase(Player& currentPlayer, Die dieArray[]) {
	//This is called from the main function to actually play a phase. We are sent the current player by reference
	//so we can make modifications to their turn, phase, and score.

	//We also are sent the entire die array so we can use it to determine rolls.

	std::cout << "Player " << currentPlayer.number << "'s turn" << " Phase " << currentPlayer.currentPhase << ":" << endl << endl;
	std::cout << "Player " << currentPlayer.number << " " << displayRules(currentPlayer.currentPhase) << endl << endl;
	//Tell the user whose turn and phase it is, as well as display the rules of the phase.

	std::cout << "Press Enter to roll dice: ";
	char tmp;
	tmp = std::cin.get();

	std::cin.clear();
	std::cin.ignore(INT_MAX, '\n');
	//input cleanup.

	//The above code prompts the user to hit enter to roll the dice.

	Face rolledFaces[10];
	for (int i = 0; i < 10; i++) {
		rollDice(dieArray, rolledFaces, i);
	}
	//We declare an array of 10 faces, which will correspond to each die being rolled to exactly one face.
	//Then we loop through and roll each die to a face randomly.

	displayDice(rolledFaces); //Shows the rolled die for the user.

	for (int i = 0; i < 2; i++) {
		//This is a for loop that will ask the user to re-roll up to 2 times.

		std::cout << "\nEnter values equal to the Die you would like to reroll. Enter 0 to stop: ";

		bool rerollArray[10] = { false };
		//We use a boolean array to keep track of which die we will be rerolling.

		while (!confirmDie(rerollArray)) {
			std::cout << "Invalid input. Try again" << endl;
			std::cout << "Enter values equal to the Die you would like to reroll. Enter to stop: ";
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
		}
		//Input validation. The function called in the while condition also keeps track of which die are being rerolled.

		bool testForBreak = true;
		//determines whether there are die to be rerolled. We assume there are not.

		for (int i = 0; i < 10; i++) {
			if (rerollArray[i] == true) {
				rollDice(dieArray, rolledFaces, i);
				testForBreak = false;
				//If there are die to be rerolled, we reroll them and set the testForBreak flag to false.
			}
		}

		if (testForBreak) break; //if no die were found to be rerolled, we end this for loop prematurely (so it doesn't prompt them to reroll again)

		std::cout << "\nRerolled Die" << endl;
		displayDice(rolledFaces);
		//then we display the die as they are rerolled.
	}

	int phaseScore = passedPhase(currentPlayer, rolledFaces);
	//we call the passedPhase function to determine whether the user passes the phase or not.
	//it returns a score that we can simultaneously use to determine whether the phase was passed,
	//and if it was it also is an accurate score to increment.

	if (phaseScore == -1) { //If the phase returns a score of negative one, that is our fail state for the phase.
		std::cout << "Player " << currentPlayer.number << " did not pass this phase.\n\n";
		currentPlayer.currentTurn++;
		currentPlayer.failedPhases[currentPlayer.currentPhase - 1]++;
		return;
		//Tell the user that they did not pass, increment their turn, and increment that they have failed the phase.
	}
	else {//If the phase was passed...
		std::cout << "Player " << currentPlayer.number << " earned " << phaseScore << " score for this phase!\n\n";
		currentPlayer.score += phaseScore;
		currentPlayer.currentPhase++;
		currentPlayer.currentTurn++;
		return;
		//Tell the user what they scored for the phase, and increment their score, phase, and turn.
	}
}

string colorToText(DiceColors color) {
	if (color == ORANGE) return "Orange";
	else if (color == RED) return "Red";
	else if (color == BLUE) return "Blue";
	else return "Green";

	//This function just returns a textual representation of the colors enum.
}

void displayDice(Face rolledFaces[]) {
	for (int i = 0; i < 10; i++) {
		int value = rolledFaces[i].value;
		if (value == 0) std::cout << "Die " << i + 1 << " value: " << "W" << "    Color: " << colorToText(rolledFaces[i].color) << endl;
		else std::cout << "Die " << i + 1 << " value: " << value << "    Color: " << colorToText(rolledFaces[i].color) << endl;
	}

	return;

	//This displays a textual representation of rolled die. It also shows the value for wild faces as "W" instead of 0.
}

bool confirmDie(bool boolArray[]) {
	//Sets the boolean array to true when the input is valid.
	//This is used to keep track of which die are being re-rolled in the playPhase function.

	//Important note: this function returns false if the input is wrong.
	//Otherwise it will continue to get information about which die to reroll until the user enters 0.
	int currentInput;

	while (std::cin >> currentInput) {
		if (currentInput > 10) return false;
		else if (currentInput == 0) return true;
		else if(currentInput > 0 && currentInput <= 10) boolArray[currentInput - 1] = true;
		else if (currentInput < 0) return false;
	}

	return false;
}

int highestValue(int intArray[], int size) {
	int highValue = intArray[0];
	for (int i = 1; i < size; i++) {
		if (highValue < intArray[i])  highValue = intArray[i];
	}

	return highValue;

	//this searches an array of ints of size... size and returns the highest value it finds.
}

int passedPhase(Player currentPlayer, Face rolledArray[]) {

	//This is where the main logic of each phase is held. Most of these are very similar, so I won't be commenting
	//too heavily on repeated portions of code.

	if (currentPlayer.currentPhase == 1) { //If the phase is phase 1...

		int chosenDiceOne[3];
		bool indexChosen[10] = { false };
		//We make two arrays, an int array of the chosen dice for the first set of phase 1
		//and a boolean array of which indexes have been chosen (so they can't be chosen more than once).

		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set 1", chosenDiceOne, indexChosen, rolledArray, 3);
		//This function will determine which die the player will be using for this set.

		int setOne = checkForSet(chosenDiceOne, 3);
		//We store the result of this set in the setOne variable, calling our checkForSet function to determine whether it's a set or not.

		if (setOne == -1) return -1;
		//if it's not a set, we return a "score" of -1. Implicating that the phase was failed.

		int chosenDiceTwo[3];
		//We have another array of size 3 for the second set of phase 1.

		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set 2", chosenDiceTwo, indexChosen, rolledArray, 3);
		//And we get those values from the enterDieValues function (this was originally a block of code but I used it so much it became a function).

		int setTwo = checkForSet(chosenDiceTwo, 3);
		//we check whether setTwo is indeed a set.

		if (setTwo == -1) return -1;

		return setOne + setTwo; //and provided both of these sets were valid, we return the combined score of them.
	}
	//END OF PHASE 1 LOGIC.

	/*Most of these phases are pretty identical. I'll go into new details for each phase.*/

	if (currentPlayer.currentPhase == 2) {
		int chosenDice[3];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the set", chosenDice, indexChosen, rolledArray, 3);

		int set = checkForSet(chosenDice, 3);
		if (set == -1) return -1; //return a value indicating failure.
								  //end of checking for a set.

		int chosenDiceRun[4]; //Here we use an array of size 4 to represent the values we'll be using for our run.
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the run", chosenDiceRun, indexChosen, rolledArray, 4);

		int run = checkForRun(chosenDiceRun, 4); //Check for run works similarly to checkForSet. Except it was harder to code and debug.
		if (run == -1) return -1;

		return set + run;
	}
	//END OF PHASE 2 LOGIC.

	if (currentPlayer.currentPhase == 3) {
		int chosenDiceSet[4];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the set", chosenDiceSet, indexChosen, rolledArray, 4);

		int set = checkForSet(chosenDiceSet, 4);
		if (set == -1) return -1;

		int chosenDiceRun[4];
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the run", chosenDiceRun, indexChosen, rolledArray, 4);

		int run = checkForRun(chosenDiceRun, 4);
		if (run == -1) return -1;

		return set + run;
	}
	//END OF PHASE 3 LOGIC.

	if (currentPlayer.currentPhase == 4) {
		int chosenDice[7];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the run", chosenDice, indexChosen, rolledArray, 7);

		return checkForRun(chosenDice, 7);
	}
	//END OF PHASE 4 LOGIC.

	if (currentPlayer.currentPhase == 5) {
		int chosenDice[8];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the run", chosenDice, indexChosen, rolledArray, 8);

		return checkForRun(chosenDice, 8);
	}
	//END OF PHASE 5 LOGIC.

	if (currentPlayer.currentPhase == 6) {
		int chosenDice[9];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the run", chosenDice, indexChosen, rolledArray, 9);

		return checkForRun(chosenDice, 9);
	}
	//END OF PHASE 6 LOGIC.

	if (currentPlayer.currentPhase == 7) {
		int chosenDiceOne[4];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set 1", chosenDiceOne, indexChosen, rolledArray, 4);

		int setOne = checkForSet(chosenDiceOne, 4);
		if (setOne == -1) return -1;

		int chosenDiceTwo[4];
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set 2", chosenDiceTwo, indexChosen, rolledArray, 4);

		int setTwo = checkForSet(chosenDiceTwo, 4);
		if (setTwo == -1) return -1;

		return setOne + setTwo;
	}
	//END OF PHASE 7 LOGIC

	if (currentPlayer.currentPhase == 8) {
		int chosenDice[7];
		bool indexChosen[10] = { false };
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for the set", chosenDice, indexChosen, rolledArray, 7);
		DiceColors chosenColors[7]; //we need to see which colors we have.
		int count = 0;
		for (int i = 0; count < 7; i++) { //unconventional for loop but it does exactly what we need
			if (indexChosen[i]) chosenColors[count++] = rolledArray[i].color;
			//will get the same color of the indexes chosen. order doesn't matter because they all need to be the same anyway so it doesn't need to be perfectly parallel
			//with the chosen colors array.
		}

		return checkForSet(chosenColors, chosenDice, 7); //uses the overloaded function for colors.
	}
	//END OF PHASE 8 LOGIC	

	if (currentPlayer.currentPhase == 9) {
		int chosenDiceOne[5];
		bool indexChosen[10];
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set one", chosenDiceOne, indexChosen, rolledArray, 5);

		int setOne = checkForSet(chosenDiceOne, 5);
		if (setOne == -1) return -1;

		int chosenDiceTwo[2];
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set two", chosenDiceTwo, indexChosen, rolledArray, 2);

		int setTwo = checkForSet(chosenDiceTwo, 2);
		if (setTwo == -1) return -1;

		return setOne + setTwo;
	}
	//END OF PHASE 9 LOGIC

	if (currentPlayer.currentPhase == 10) {
		int chosenDiceOne[5];
		bool indexChosen[10];
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set one", chosenDiceOne, indexChosen, rolledArray, 5);

		int setOne = checkForSet(chosenDiceOne, 5);
		if (setOne == -1) return -1;

		int chosenDiceTwo[3];
		enterDieValues("\nEnter the Die Index of the Die you would like to submit for set two", chosenDiceTwo, indexChosen, rolledArray, 3);

		int setTwo = checkForSet(chosenDiceTwo, 3);
		if (setTwo == -1) return -1;

		return setOne + setTwo;
	}
	//END OF PHASE 10 LOGIC

	return -1; //return the fail state if you get some weird phase somehow.
}

int checkForSet(int setArray[], int size) {
	//This function takes an array of ints of size... size and determines
	//whether or not they make up a set of numbers. (Factoring in wildcards.)

	int setValue = 0;
	int total = 0;
	//initializes some variables we need.

	for (int i = 0; i < size; i++) {
		if (setArray[i]) {
			setValue = setArray[i];
			break;
		}
	}//this for loop loops through the array and determines what the set
	 //will actually be comprised of.

	for (int i = 0; i < size; i++) {
		if (setArray[i] != 0 && setArray[i] != setValue) {
			return -1;
		}
		else total += setArray[i];
	}//If this loop finds a number that isn't a wildcard or the same number as the set value, it returns the fail state.
	 //Otherwise it increments the total.

	return total;
	//and we return the total down here if everything went well!
}

int checkForSet(DiceColors setArray[], int valueArray[], int size) {
	//This is an overloaded version of checkForSet that checks for colors.
	//We don't worry about wild faces here because they don't affect colors,
	//but we do need to know the value of each face to increment the score.

	int colorValue = setArray[0];
	int total = valueArray[0];
	for (int i = 1; i < size; i++) {
		if (setArray[i] == colorValue) total += valueArray[i];
		else return -1;
	}//checks if all colors are the same, and if so it'll increment the total accordingly.

	return total;
	//returns the total found.
}

int checkForRun(int runArray[], int size) {
	//This function checks an int array of size... size for a run of numbers.
	//Was definitely more complicated to code than checkForSet.

	int setStart = 0;
	int total = 0;

	sortArray(runArray, size);
	//we sort the array so we can calculate a run from the largest number.

	setStart = runArray[size - 1];
	//The largest number is now in a predictable spot. So we assign the setStart to that number.

	int count = 1;
	total += setStart;
	//We use a count variable to determine whether a set has been found later. And we increment the total
	//By the first value found.

	int totalWildCards = 0;
	for (int i = 0; i < size; i++) {
		if (runArray[i] == 0) totalWildCards++;
	}
	//We store the total number of wild cards available to us so our logic can work correctly later on.

	int wildCardsUsed = 0;
	int runMin = (setStart - size) + 1;
	//We'll also need to know how many wild cards have been used, as well as the smallest possible
	//value our run could have (there were bugs otherwise).

	bool valuesUsed[11] = { false };
	//Because we can't have repeats, we store a boolean array of
	//which values are used.

	valuesUsed[setStart] = true;

	for (int i = size - 2; i >= 0 + wildCardsUsed; i--) {
		//We start looping from the second highest index, and loop downward.
		//We don't end up looping through wild faces we use.

		if (runArray[i] != 0 && runArray[i] < runMin) {
			total = -1;
			break;
		}//if we find a smaller number than the minimum and it's not a wild face, we return the fail state.

		if (runArray[i] == setStart - (count + wildCardsUsed)) {
			if (runArray[i] != 0) {
				if (!valuesUsed[runArray[i]]) {
					count++;
					total += runArray[i];
					valuesUsed[runArray[i]] = true;
				}
				else {
					total = -1;
					break;
				}
			}//determines whether a nonzero value has been used before. If not, we allow this to happen
			 //and set the check to true that it has been used before.

			 //If it has been used before, we return the fail state. This was a hard bug to track down.

			else {
				count++;
				total += runArray[i];
			}
		}//If we find the next value that we need for the run, we increment the count 
		 //and the total.

		 //This works because it's looking for setStart - (count + wildCardsUsed). Every time we find a new number
		 //it will look for a lower number. And if we used a wild face to "be" another number, it accounts for that.

		else if (totalWildCards > 0) {
			//If we don't quite find the number we are looking for, but have wildcards to spare...
			totalWildCards--;
			wildCardsUsed++;
			count++;
			total += runArray[i];
		}//we pretend like we found a decent number, and increment (and decrement) necessary variables.
		 //this is structured in a way that is preserves the main logic.

		else {
			total = -1;
			break;
		} //If we find a number that isn't what we're looking for and no more wildcards exist, return the fail state.
	}

	return total;
}

void sortArray(int intArray[], int size) {
	//these are relatively small so we'll just do a selection sort.
	for (int i = 0; i < size - 1; i++) {
		int smallestIndex = i;
		for (int x = i + 1; x < size; x++) {
			if (intArray[x] < intArray[smallestIndex]) smallestIndex = x;
		}//find the smallest value and store the index of it.

		int tmp = intArray[i];
		intArray[i] = intArray[smallestIndex];
		intArray[smallestIndex] = tmp;
	}//does the swap for us.
}

void enterDieValues(string prompt, int storedArray[], bool chosenIndexes[], Face dieArray[], int size) {
	//I ended up doing this a whole lot. So I made it into a function so it wouldn't clog the passedPhase function (it's so much more readible now.)


	std::cout << prompt << " (" << size << " dice): ";
	//Prompts the user for what we need. Depends on the function call.

	for (int i = 0; i < size; i++) {
		int selectedIndex;
		while (!(std::cin >> selectedIndex) || selectedIndex > 10 || selectedIndex < 1 || chosenIndexes[selectedIndex - 1] == true) {
			std::cout << "Invalid input. Try again" << endl;
			std::cout << prompt << " (" << size - i << " dice): ";
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
		}
		storedArray[i] = dieArray[selectedIndex - 1].value;
		chosenIndexes[selectedIndex - 1] = true;
	}
	//And stores the integers we need from the user, provided they get passed the input validation.

}

void sortPlayersByTurnOrder(Player players[], int numberOfPlayers) {
	//This is the same as the selection sort array but by a different name.
	//I thought of overloading the other function but thought this would be more clear to myself.
	for (int i = 0; i < numberOfPlayers; i++) {
		int highestIndex = i;
		for (int x = i + 1; x < numberOfPlayers; x++) {
			if (players[x].turnOrder > players[highestIndex].turnOrder) highestIndex = x;
		}

		Player tmp = players[i];
		players[i] = players[highestIndex];
		players[highestIndex] = tmp;
	}

	//This function changes the order of the players array based on their turn order.
}

void showFullDice(Die diceArray[], int size) {
	for (int i = 0; i < size; i++) {
		std::cout << "Die " << i + 1 << endl;
		for (int x = 0; x < 6; x++) {
			if (diceArray[i].faces[x].value == 0) std::cout << "Face " << x + 1 << " Value: " << "W" << " Color: " << colorToText(diceArray[i].faces[x].color) << endl;
			else std::cout << "Face " << x + 1 << " Value: " << diceArray[i].faces[x].value << " Color: " << colorToText(diceArray[i].faces[x].color) << endl;
		}
		std::cout << endl;
	}
	//Displays the full information when dice are generated. So you can know which values correspond to which colors, and which colors are more prevalent on certain dice.
}

string displayRules(int phase) {
	if (phase == 1) return "must roll 2 sets of 3";
	if (phase == 2) return "must roll 1 set of 3 and 1 run of 4";
	if (phase == 3) return "must roll 1 set of 4 and 1 run of 4";
	if (phase == 4) return "must roll 1 run of 7";
	if (phase == 5) return "must roll 1 run of 8";
	if (phase == 6) return "must roll 1 run of 9";
	if (phase == 7) return "must roll 2 sets of 4";
	if (phase == 8) return "must roll 7 of all one color";
	if (phase == 9) return "must roll 1 set of 5 and 1 set of 2";
	if (phase == 10) return "1 set of 5 and 1 set of 3";

	return "error";

	//Returns the rules as a string for each phase.
}

void displayRoundInfo(Player players[], int round, int numberOfPlayers) {
	std::cout << "----------------------------------------------";
	std::cout << "\nRound " << round + 1 << " begins! Current Player Scores and Phases: " << endl;
	for (int i = 0; i < numberOfPlayers; i++) {
		std::cout << "Player " << players[i].number << " Current Phase: " << players[i].currentPhase << " Score: " << players[i].score << endl;
	}
	std::cout << "----------------------------------------------";
	std::cout << endl;

	//Displays player scores, the current round, and the phase of each player every round.
}
