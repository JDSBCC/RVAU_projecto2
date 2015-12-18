#pragma once

#define WINNER 1
#define LOSER 0

class Table
{
private:
	int cards[4][3];
public:
	Table(int cards[]);
	~Table();

	void setCards(int cards[]);
	int getFirstToPlay();
	void processTable();
	int getResult(int index);
};

