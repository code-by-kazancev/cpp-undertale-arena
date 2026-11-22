#pragma execution_character_set("utf-8")
#include <iostream>
#include <locale>
#include <windows.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <ctime>

using namespace std;

const int dlina = 20, shirina = 20;

enum class BossState { WARNING, PROJECTILE, MSTITELNIYPROJECTILE, SHRAPNEL, WAVE, REST };

struct Particle {
	int x, y;
	int speedX, speedY;
	bool isActive;
};

Particle shrapnel[8];

void idiCursorVNachalo();
void handleInput(int& pointX, int& pointY);
void updateWarningAttack(int& pointX, int& pointY, int& stateTimer, int& warnX, int& warnY, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState);
void updateProjectileAttack(int& pointX, int& pointY, int& stateTimer, int& projX, int& projY, int& projSpeedX, int& projSpeedY, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState);
void updateMsitelniyStrelaAttack(int& pointX, int& pointY, int& stateTimer, int& projX, int& projY, int& projSpeedX, int& projSpeedY, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState);
void updateShrapnelPhase(int& pointX, int& pointY, int& stateTimer, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState);
void updateWaveAttack(int& pointX, int& pointY, int& stateTimer, int& waveY, int& waveGapX, int& waveSpeedCounter, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState);

int main() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	setlocale(LC_ALL, ".UTF-8");
	srand(static_cast<unsigned int>(time(nullptr)));

	int pointX = dlina / 2, pointY = shirina / 2;
	bool srajenie = true;

	BossState currentState = BossState::WARNING;
	int stateTimer = 0;

	int hitX = 10, hitY = 10;
	int warnX = 10, warnY = 10;
	int projX = 2, projY = 2;
	int projSpeedX = 1, projSpeedY = 1;

	int waveY = 0;
	int waveGapX = 10;
	int waveSpeedCounter = 0;

	int collisionTimer = 0;
	bool playerHit = false;

	for (int i = 0; i < 8; i++) shrapnel[i].isActive = false;

	while (srajenie) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) srajenie = false;
		handleInput(pointX, pointY);

		switch (currentState) {
			case BossState::WARNING:
				updateWarningAttack(pointX, pointY, stateTimer, warnX, warnY, hitX, hitY, collisionTimer, playerHit, currentState);
				break;
			case BossState::PROJECTILE:
				updateProjectileAttack(pointX, pointY, stateTimer, projX, projY, projSpeedX, projSpeedY, hitX, hitY, collisionTimer, playerHit, currentState);
				break;
			case BossState::MSTITELNIYPROJECTILE:
				updateMsitelniyStrelaAttack(pointX, pointY, stateTimer, projX, projY, projSpeedX, projSpeedY, hitX, hitY, collisionTimer, playerHit, currentState);
				break;
			case BossState::SHRAPNEL:
				updateShrapnelPhase(pointX, pointY, stateTimer, hitX, hitY, collisionTimer, playerHit, currentState);
				break;
			case BossState::WAVE:
				if (stateTimer == 0) {
					waveY = shirina + 1;
					waveGapX = 1 + rand() % dlina;
					waveSpeedCounter = 0;
				}
				updateWaveAttack(pointX, pointY, stateTimer, waveY, waveGapX, waveSpeedCounter, hitX, hitY, collisionTimer, playerHit, currentState);
				break;
			case BossState::REST:
				if (collisionTimer == 0) {
					currentState = BossState::WARNING;
					stateTimer = 0;
				}
				break;
		}

		if (collisionTimer > 0) collisionTimer--;

		string frame;
		for (int row = 0; row <= dlina + 1; row++) {
			for (int col = 0; col <= shirina + 1; col++) {
				bool drawn = false;

				if (row == 0 || row == dlina + 1 || col == 0 || col == shirina + 1) {
					frame += "█"; drawn = true;
				}
				else if (collisionTimer > 0 && playerHit && row == hitY && col == hitX) {
					frame += "*"; drawn = true;
				}
				else if (collisionTimer > 0 && !playerHit && row == hitY && col == hitX) {
					frame += "O"; drawn = true;
				}
				else if (currentState == BossState::PROJECTILE && row == projY && col == projX) {
					frame += "►"; drawn = true;
				}
				else if (currentState == BossState::MSTITELNIYPROJECTILE && row == projY && col == projX) {
					frame += "◄"; drawn = true;
				}
				else if (currentState == BossState::SHRAPNEL) {
					for (int i = 0; i < 8; i++)
						if (shrapnel[i].isActive && shrapnel[i].y == row && shrapnel[i].x == col) {
							frame += "<"; drawn = true; break;
						}
				}

				if (!drawn && row == pointY && col == pointX) {
					frame += "☻"; drawn = true;
				}
				else if (!drawn && currentState == BossState::WAVE && row == waveY) {
					if (col == waveGapX) frame += " ";
					else frame += "=";
					drawn = true;
				}

				if (!drawn) {
					if (currentState == BossState::WARNING && row == warnY && col == warnX) frame += "!";
					else frame += " ";
				}
			}
			if (row < dlina + 1) frame += '\n';
		}

		frame += "\n========================\n";
		frame += "Состояние: ";
		switch (currentState) {
			case BossState::WARNING: frame += "WARNING"; break;
			case BossState::PROJECTILE: frame += "PROJECTILE"; break;
			case BossState::MSTITELNIYPROJECTILE: frame += "МСТИТЕЛЬНЫЙ СНАРЯД"; break;
			case BossState::SHRAPNEL: frame += "SHRAPNEL (Осколки)"; break;
			case BossState::WAVE: frame += "WAVE"; break;
			case BossState::REST: frame += "REST"; break;
		}

		frame += " | Таймер: " + to_string(stateTimer);
		if (collisionTimer > 0) frame += " | Удар: " + to_string(collisionTimer);
		frame += "\n========================\n";

		idiCursorVNachalo();
		cout << frame;
		Sleep(30);
	}
	return 0;
}

void idiCursorVNachalo() {
	COORD coordinate{ 0, 0 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordinate);
}

void handleInput(int& pointX, int& pointY) {
	if ((GetAsyncKeyState('W') & 0x8000) && pointY > 1) pointY--;
	if ((GetAsyncKeyState('S') & 0x8000) && pointY < dlina) pointY++;
	if ((GetAsyncKeyState('A') & 0x8000) && pointX > 1) pointX--;
	if ((GetAsyncKeyState('D') & 0x8000) && pointX < shirina) pointX++;
}

void updateWarningAttack(int& pointX, int& pointY, int& stateTimer, int& warnX, int& warnY, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState) {
	if (stateTimer == 0) {
		warnX = 1 + rand() % dlina;
		warnY = 1 + rand() % shirina;
	}
	stateTimer++;
	if (stateTimer >= 30) {
		if (pointX == warnX && pointY == warnY) {
			hitX = warnX;
			hitY = warnY;
			playerHit = true;
			collisionTimer = 15;
			cout << "\a";
		}
		else {
			hitX = warnX;
			hitY = warnY;
			playerHit = false;
			collisionTimer = 15;
		}
		currentState = BossState::PROJECTILE;
		stateTimer = 0;
	}
}

void updateProjectileAttack(int& pointX, int& pointY, int& stateTimer, int& projX, int& projY, int& projSpeedX, int& projSpeedY, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState) {
	if (stateTimer == 0) {
		projX = 2;
		projY = 2;
		projSpeedX = 1;
		projSpeedY = 1;
		int dx_spawn = pointX - projX; if (dx_spawn < 0) dx_spawn = -dx_spawn;
		int dy_spawn = pointY - projY; if (dy_spawn < 0) dy_spawn = -dy_spawn;
		if (dx_spawn <= 1 && dy_spawn <= 1) {
			hitX = pointX;
			hitY = pointY;
			playerHit = true;
			collisionTimer = 15;
			cout << "\a";
			currentState = BossState::WAVE;
			stateTimer = 0;
			return;
		}
	}

	stateTimer++;
	projX += projSpeedX;
	projY += projSpeedY;

	int dx = pointX - projX; if (dx < 0) dx = -dx;
	int dy = pointY - projY; if (dy < 0) dy = -dy;

	if (dx <= 1 && dy <= 1) {
		hitX = pointX;
		hitY = pointY;
		playerHit = true;
		collisionTimer = 15;
		cout << "\a";
		currentState = BossState::WAVE;
		stateTimer = 0;
		return;
	}
	else if (projX <= 1 || projX > dlina || projY <= 1 || projY > shirina) {
		projX -= projSpeedX;
		projY -= projSpeedY;
		hitX = pointX;
		hitY = pointY;
		playerHit = false;
		collisionTimer = 15;
		currentState = BossState::MSTITELNIYPROJECTILE;
		stateTimer = 0;
	}
}

void updateMsitelniyStrelaAttack(int& pointX, int& pointY, int& stateTimer, int& projX, int& projY, int& projSpeedX, int& projSpeedY, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState) {
	if (stateTimer == 0 || stateTimer % 5 == 0) {
		int dx = pointX - projX;
		int dy = pointY - projY;
		int absDx = abs(dx);
		int absDy = abs(dy);

		projSpeedX = 0;
		projSpeedY = 0;

		if (absDx > absDy) projSpeedX = (dx > 0) ? 1 : -1;
		else if (absDy > absDx) projSpeedY = (dy > 0) ? 1 : -1;
		else {
			projSpeedX = (dx > 0) ? 1 : -1;
			projSpeedY = (dy > 0) ? 1 : -1;
		}

		if (projSpeedX == 0 && projSpeedY == 0) {
			projSpeedX = -1;
			projSpeedY = -1;
		}
	}

	stateTimer++;
	projX += projSpeedX;
	projY += projSpeedY;

	int distX = pointX - projX; if (distX < 0) distX = -distX;
	int distY = pointY - projY; if (distY < 0) distY = -distY;

	if (distX <= 1 && distY <= 1) {
		hitX = pointX;
		hitY = pointY;
		playerHit = true;
		collisionTimer = 15;
		cout << "\a";
		currentState = BossState::WAVE;
		stateTimer = 0;
		return;
	}
	else if (stateTimer > 35) {
		cout << "\a";
		int dirsX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
		int dirsY[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
		for (int i = 0; i < 8; i++) {
			shrapnel[i].x = projX;
			shrapnel[i].y = projY;
			shrapnel[i].speedX = dirsX[i];
			shrapnel[i].speedY = dirsY[i];
			shrapnel[i].isActive = true;
		}
		currentState = BossState::SHRAPNEL;
		stateTimer = 0;
		return;
	}
	else if (projX < 1 || projX > dlina || projY < 1 || projY > shirina) {
		cout << "\a";
		int explosionX = projX;
		int explosionY = projY;
		if (explosionX < 1) explosionX = 1;
		if (explosionX > dlina) explosionX = dlina;
		if (explosionY < 1) explosionY = 1;
		if (explosionY > shirina) explosionY = shirina;

		int dirsX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
		int dirsY[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
		for (int i = 0; i < 8; i++) {
			shrapnel[i].x = explosionX;
			shrapnel[i].y = explosionY;
			shrapnel[i].speedX = dirsX[i];
			shrapnel[i].speedY = dirsY[i];
			shrapnel[i].isActive = true;
		}
		currentState = BossState::SHRAPNEL;
		stateTimer = 0;
	}
}

void updateShrapnelPhase(int& pointX, int& pointY, int& stateTimer, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState) {
	stateTimer++;
	bool allDead = true;

	for (int i = 0; i < 8; i++) {
		if (!shrapnel[i].isActive) continue;

		shrapnel[i].x += shrapnel[i].speedX;
		shrapnel[i].y += shrapnel[i].speedY;

		if (shrapnel[i].x == pointX && shrapnel[i].y == pointY) {
			hitX = pointX;
			hitY = pointY;
			playerHit = true;
			collisionTimer = 15;
			cout << "\a";
			shrapnel[i].isActive = false;
			currentState = BossState::WAVE;
			stateTimer = 0;
		}
		else if (shrapnel[i].x < 1 || shrapnel[i].x > dlina || shrapnel[i].y < 1 || shrapnel[i].y > shirina) {
			shrapnel[i].isActive = false;
		}

		if (shrapnel[i].isActive) allDead = false;
	}

	if (allDead || stateTimer > 40) {
		currentState = BossState::WAVE;
		stateTimer = 0;
	}
}

void updateWaveAttack(int& pointX, int& pointY, int& stateTimer, int& waveY, int& waveGapX, int& waveSpeedCounter, int& hitX, int& hitY, int& collisionTimer, bool& playerHit, BossState& currentState) {
	stateTimer++;
	waveSpeedCounter++;

	if (waveSpeedCounter >= 3) {
		waveY--;
		waveSpeedCounter = 0;
	}

	if (pointY == waveY && pointX != waveGapX) {
		hitX = pointX;
		hitY = pointY;
		playerHit = true;
		collisionTimer = 15;
		cout << "\a";
		currentState = BossState::REST;
		stateTimer = 0;
		return;
	}
	else if (waveY < 1) {
		currentState = BossState::REST;
		stateTimer = 0;
	}
}