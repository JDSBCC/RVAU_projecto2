#include "stdafx.h"
#include "Table.h"
#include <stdlib.h>
#include <math.h>
#include <time.h> 


Table::Table(int cards[]){
	setCards(cards);
}


Table::~Table()
{
}

void Table::setCards(int cards[]) {//0-naipe  1-numero
	for (int i = 0; i < 4; i++) {
		this->cards[i][0] = cards[i] % 4;//0=espadas 1=copas 2=paus 3=ouros
		int value = (int)round((double)cards[i] / 4.0);
		this->cards[i][1] = value==1?14:value;
		this->cards[i][2] = LOSER;
		printf("table = %d \n", this->cards[i][1]);
	}
}

int Table::getFirstToPlay() {
	srand(time(NULL));
	return rand() % 4;
}

void Table::processTable() {
	int first = getFirstToPlay();
	printf("first = %d", cards[first][1]);
	int naipe = cards[first][0];
	int num = cards[first][1];
	int bestCardPos = -1;

	for (int i = 0; i < 4; i++) {
		if (naipe == cards[i][0]) {
			if (num <= cards[i][1]) {
				bestCardPos = i;
				num = cards[i][1];
			}
		}
	}
	cards[bestCardPos][2] = WINNER;
}

int Table::getResult(int index) {
	return cards[index][2];
}
