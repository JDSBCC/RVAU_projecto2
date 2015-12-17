#pragma once

#define WINNER 1
#define LOSER 0

class Table
{
	enum card {
		AS, DOIS, TRES, QUATRO, CINCO, SEIS, SETE, OITO, NOVE, DEZ, DAMA, VALETE, REI
	};

	enum naipe {
		ESPADAS, COPAS, PAUS, OUROS
	};

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

