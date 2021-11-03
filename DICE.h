#include <stdio.h>     
#include <stdlib.h>     
#include <time.h>
#pragma once
class DICE
{
private:
	int dice1;
	int dice2;

public:

	DICE()
	{
		srand((unsigned int)time(NULL));
		dice1 = -1;
		dice2 = -1;
	}

	// Accessor Functions
	int getDice1()
	{
		return dice1;
	}

	int getDice2()
	{
		return dice2;
	}

	// Mutator Functions

	void setDice1(int newDice1)
	{
		dice1 = newDice1;
	}

	void setDice2(int newDice2)
	{
		dice2 = newDice2;
	}


	// Other Functions
	int rollDice()
	{
		dice1 = rand() % 6 + 1; // Range [1,6]
		dice2 = rand() % 6 + 1; // Range [1,6]

		return dice1 + dice2;
	}

	int getOldRoll()
	{
		return dice1 + dice2;
	}
	
};

