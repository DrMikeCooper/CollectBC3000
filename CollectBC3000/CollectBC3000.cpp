// BC3000.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdlib.h"
#include "conio.h"
#include <string>

int getLevel();
int roundScore;

//////////// CONFIGURABLES ///////////////////////
const int numGames = 30000;
const int numRuns = 10;
const int seed = 10222; // random number seed - joggle as see fit

const int num1 = 19; // number of staples
const int num2 = 29; // total number of common/rare
const int num3 = 30; // total number of special/ultra

// in a random pick, this is the multiplier for items you dont have
const int weightNotOwnedBase = 2;
int weightNotOwned = weightNotOwnedBase;
int weightNotOwnedDelta = 2;

// cost of jars
const int stapleCost = 1000;
const int ultraCost = 3000;
const float refund = 0.25; // percentage you get back when you get a duplicate

const int coinBonusPerLevel = 150;

// increase this to make the player more likely to save up for rare jars as their staples get low
const float savingUpFactor = 0.2f;

//daily tasks
int gamesPerDay = 10;
int dailyTaskCoins = 125;

// player skill - 100% matches Ella's data
float playerSkill = 0.3f;
bool thirdItem = false;

// net coins earned per game
int coinsPerGame()
{
	// based on Ella's data. 
	int level = getLevel();
	if (level > 20)
		level = 20; // level cap

	float coinScore = playerSkill * (thirdItem ? 1.5f : 1.0f) * (30 + (level - 1) * 4);
	// store for later to print out
	roundScore = coinScore * 200;
	
	return coinScore;
}

//////////// END CONFIGURABLES ///////////////////////

// currencies
int coins = 0;
int xp = 0;

// inventory of items owned
bool staple[num1] = { false };
bool common[num2+num3] = { false };

int gamesPlayed = 0;

bool savingForUltra = false;

// XP earned per game
int xpPerGame()
{
	return 10 * coinsPerGame();
}

int getRandomIndex(bool* arr, int num, int weight)
{
	int total = 0;
	for (int i = 0; i < num; i++)
		total += arr[i] ? 1 : weight;

	int index = rand() % total;
	int choice = 0;
	total = 0;
	for (int i = 0; i < num; i++)
	{
		if (total <= index)
			choice = i;
		total += arr[i] ? 1 : weight;
	}
	return choice;
}

// gets player level based off XP
int getLevel()
{
	int xps[] = { 100,200,400,600,800,1000,1200,1400,1600,1800, 99999999999 };
	int xp0 = xp;
	int lev = 0;
	while (xp0 > 0 && lev <10)
	{
		xp0 -= xps[lev];
		lev++;
	}
	while (xp0 > 0 && lev <20)
	{
		xp0 -= 5000;
		lev++;
	}
	while (xp0 > 0)
	{
		xp0 -= 20000;
		lev++;
	}

	return lev;
}

// analytics for checking source of coins
int coinsGame, coinsLevelUp, coinsTasks, coinsRefund;

void Reset()
{
	coins = 0;
	xp = 0;
	for (int i = 0; i < num1; i++)
		staple[i] = false;
	for (int i = 0; i < num2+num3; i++)
		common[i] = false;
	coinsGame = coinsLevelUp = coinsTasks = coinsRefund = 0;
	weightNotOwned = weightNotOwnedBase;
}

// do a battlecomputer run
void simulateGame()
{
	char fname[256];
	// make up a file name to represent the parameters we used
	// eg log_saving60_skill75_thirdItem.csv means saving tendencies of 60%, player skill of 75% and third item was active
	sprintf_s(fname, "log_saving%i_skill%i%s.csv", (int)(100 * savingUpFactor), (int)(100 * playerSkill), (thirdItem ? "_thirdItem" : "" ));
	FILE* f1;
	fopen_s(&f1, fname, "w");
	fprintf(f1, "Action, games, score, coin, xp, level, staples, commons, specials\n");
	int gamesPlayedTotal = 0;

	for (int k = 0; k < numRuns; k++)
	{
		Reset();
		for (gamesPlayed = 0; gamesPlayed < numGames; gamesPlayed++)
		{
			std::string action;

			coins += coinsPerGame();
			coinsGame += coinsPerGame();

			// level up gives coins
			int lvl = getLevel();
			xp += xpPerGame();
			int lvl2 = getLevel();
			coins += (lvl2 - lvl)*coinBonusPerLevel;
			coinsLevelUp += (lvl2 - lvl)*coinBonusPerLevel;

			// daily task
			if ((gamesPlayed % gamesPerDay) == 0)
			{
				coins += dailyTaskCoins;
				coinsTasks += dailyTaskCoins;
			}

			// persistent tasks


			int numStaples = 0;
			for (int i = 0; i < num1; i++)
				if (staple[i]) numStaples++;
			int numCommon = 0;
			for (int i = 0; i < num2; i++)
				if (common[i]) numCommon++;
			int numSpecial = 0;
			for (int i = num2; i < num2 + num3; i++)
				if (common[i]) numSpecial++;

			int numStaplesLeft = num1 - numStaples;
			int numRareLeft = num2 + num3 - numCommon - numSpecial;

			// decision to buy a staple jar or an ultra jar depends on
			// how many we have left of each, and the relative cost.
			float pStaple = (float)(numStaplesLeft) / (float)num1;
			if (coins < stapleCost)
				pStaple = 0;
			if (savingForUltra) // if we've decided to save,stick to this decision
				pStaple = 0;
			float pUltra = (float)(numRareLeft) / (float)(num2 + num3);
			// if we dont have enough for a ultra jar, dont dismiss immediately. We may be saving up,
			// which gets more likely the fewer remaining staples we need
			if (coins < ultraCost)
				pUltra *= savingUpFactor * (1 - pStaple);

			if (coins > stapleCost)
			{
				// roll a dice between stable and ultra probabilities
				int diceSize = 10000;
				float dice = (pStaple + pUltra) * (rand() % diceSize);
				if (dice < pStaple*diceSize)
				{
					coins -= stapleCost;
					int index = getRandomIndex(staple, num1, weightNotOwned);
					if (staple[index])
					{
						coins += stapleCost*refund;
						coinsRefund += stapleCost*refund;
						action = "Dupe staple";
						weightNotOwned += weightNotOwnedDelta;
					}
					else
					{
						staple[index] = true;
						action = "Bought staple";
						numStaples++;
						weightNotOwned = weightNotOwnedBase;
					}
				}
				else if (coins > ultraCost)
				{
					savingForUltra = false;
					coins -= ultraCost;
					int index = getRandomIndex(common, num2 + num3, weightNotOwned);
					{
						if (common[index])
						{
							coins += ultraCost*refund; // return
							coinsRefund += ultraCost*refund;
							action = index >= num2 ? "Dupe special" : "Dupe common";
							weightNotOwned += weightNotOwnedDelta;
						}
						else
						{
							common[index] = true;
							weightNotOwned = weightNotOwnedBase;
							if (index >= num2)
							{
								action = "Bought special";
								numSpecial++;
							}
							else
							{
								action = "Bought common";
								numCommon++;
							}
						}
					}
				}
				else
					savingForUltra = true;
			}

			if (action != "")

				fprintf(f1, "%s, %i, %i, %i, %i, %i, %i, %i, %i\n", action.c_str(), gamesPlayed, roundScore, coins, xp, getLevel(), numStaples, numCommon, numSpecial);

			// early exit once we caught em all
			if (numStaples == num1 && numCommon == num2 && numSpecial == num3)
			{
				printf("%i (%i, %i, %i, %i)\n", gamesPlayed, coinsGame, coinsLevelUp, coinsTasks, coinsRefund);
				gamesPlayedTotal += gamesPlayed;
				gamesPlayed = 10000000;
			}

		}
		fprintf(f1, "\n");
	}
	printf("avg = %i\n", gamesPlayedTotal/numRuns);
	fclose(f1);

}


void randomPickTest(int weight)
{
	const int num = 78;
	bool bin[num] = { false };
	int collected = 0;
	int count = 0;

	while (collected < num)
	{
		int index = getRandomIndex(bin, num, weight);//rand() % num;
		bin[index] = true;
		int coll = 0;
		for (int i = 0; i < num; i++)
			if (bin[i])
				coll++;

		count++;
		if (coll > collected)
		{
			collected = coll;
			//printf("%i: %i\n", collected, count);
		}
	}
	printf("%i: %i\n", weight, count);
}

int main()
{
	srand(seed);
	/* unit test
	bool test[4] = { false };
	test[3] = true;
	int counts[4] = { 0 };
	for (int i = 0; i < 1000; i++)
	{
		int index = getRandomIndex(test, 4, 2);
		counts[index]++;
	}*/
	//randomPickTest(4);
	randomPickTest(1);
	randomPickTest(2);
	randomPickTest(3);
	randomPickTest(5);
	randomPickTest(10);
	randomPickTest(20);

	simulateGame();
	
	_getch();

	return 0;
}
