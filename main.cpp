#pragma execution_character_set("utf-8")
#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <memory>

using namespace std;

const int dlina = 20, shirina = 20;

// === БАЗОВЫЙ КЛАСС ===
class Attack {
public:
	bool isActive;
	int stateTimer;
	int hitX, hitY;
	int collisionTimer;
	bool playerHit;
	bool isResolved;

	Attack() : isActive(false), stateTimer(0), hitX(0), hitY(0), collisionTimer(0), playerHit(false), isResolved(false) {}
	virtual ~Attack() {}

	virtual unique_ptr<Attack> update(int px, int py) = 0;
	virtual void render(string& frame, int row, int col, int px, int py, bool& drawn) = 0;

	// Метод говорит менеджеру, нужно ли ещё держать эту атаку в памяти
	virtual bool isAlive() const {
		return isActive || collisionTimer > 0;
	}

	virtual void reset() {
		isActive = false;
		stateTimer = 0;
		hitX = 0; hitY = 0;
		collisionTimer = 0;
		playerHit = false;
		isResolved = false;
	}
};

// === ПРЕДУПРЕЖДЕНИЕ ===
class WarningAttack : public Attack {
public:
	int warnX, warnY;
	bool initialized;

	WarningAttack() : warnX(0), warnY(0), initialized(false) {}

	unique_ptr<Attack> update(int px, int py) override {
		if (collisionTimer > 0) collisionTimer--;

		if (!initialized) {
			warnX = 1 + rand() % dlina;
			warnY = 1 + rand() % shirina;
			initialized = true;
		}

		stateTimer++;
		if (stateTimer >= 30 && !isResolved) {
			if (px == warnX && py == warnY) {
				hitX = warnX; hitY = warnY;
				playerHit = true;
				collisionTimer = 15;
				cout << "\a";
			}
			else {
				hitX = warnX; hitY = warnY;
				playerHit = false;
				collisionTimer = 15;
			}
			isResolved = true;
			isActive = false;
		}
		return nullptr;
	}

	void render(string& frame, int row, int col, int px, int py, bool& drawn) override {
		if (collisionTimer > 0 && playerHit && row == hitY && col == hitX) {
			frame += "*"; drawn = true;
		}
		else if (collisionTimer > 0 && !playerHit && row == hitY && col == hitX) {
			frame += "O"; drawn = true;
		}
		else if (initialized && row == warnY && col == warnX) {
			frame += "!"; drawn = true;
		}
	}

	void reset() override {
		Attack::reset();
		initialized = false;
		warnX = 0; warnY = 0;
	}
};

// === ПРОМЕЖУТОЧНЫЙ КЛАСС: ВСЁ, ЧТО ВЗРЫВАЕТСЯ ===
class ExplosiveAttack : public Attack {
protected:
	struct ShrapnelParticle {
		int x, y;
		int speedX, speedY;
		bool isActive;
	};

	ShrapnelParticle shrapnel[8];
	int shrapnelTimer;
	bool hasExploded; // Флаг: снаряд уже взорвался, рисуем только осколки

	void triggerExplosion(int ex, int ey) {
		// Сдвигаем точку взрыва на 1 клетку от стен
		int safeX = ex;
		int safeY = ey;

		if (safeX <= 1) safeX = 2;
		if (safeX >= dlina) safeX = dlina - 1;
		if (safeY <= 1) safeY = 2;
		if (safeY >= shirina) safeY = shirina - 1;

		int dirsX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
		int dirsY[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
		for (int i = 0; i < 8; i++) {
			shrapnel[i] = { safeX, safeY, dirsX[i], dirsY[i], true };
		}
		shrapnelTimer = 15;
		hasExploded = true;
		isActive = false;
		isResolved = true;
	}

	void updateShrapnelLogic(int px, int py) {
		for (int i = 0; i < 8; i++) {
			if (!shrapnel[i].isActive) continue;

			shrapnel[i].x += shrapnel[i].speedX;
			shrapnel[i].y += shrapnel[i].speedY;

			if (shrapnel[i].x == px && shrapnel[i].y == py && collisionTimer == 0) {
				hitX = px; hitY = py;
				playerHit = true;
				collisionTimer = 15;
				cout << "\a";
			}

			if (shrapnel[i].x < 1 || shrapnel[i].x > dlina || shrapnel[i].y < 1 || shrapnel[i].y > shirina) {
				shrapnel[i].isActive = false;
			}
		}

		if (shrapnelTimer > 0) shrapnelTimer--;
		else for (int i = 0; i < 8; i++) shrapnel[i].isActive = false;
	}

	void renderShrapnel(string& frame, int row, int col, bool& drawn) const {
		if (shrapnelTimer > 0) {
			for (int i = 0; i < 8; i++) {
				if (shrapnel[i].isActive &&
					shrapnel[i].x > 1 && shrapnel[i].x < dlina &&
					shrapnel[i].y > 1 && shrapnel[i].y < shirina &&
					row == shrapnel[i].y && col == shrapnel[i].x) {
					frame += "<"; drawn = true;
				}
			}
		}
	}

public:
	ExplosiveAttack() : shrapnelTimer(0), hasExploded(false) {
		for (int i = 0; i < 8; i++) shrapnel[i].isActive = false;
	}

	// ПЕРЕОПРЕДЕЛЯЕМ: Держим атаку в памяти, пока тикают осколки или удар
	bool isAlive() const override {
		return isActive || collisionTimer > 0 || shrapnelTimer > 0;
	}

	void reset() override {
		Attack::reset();
		shrapnelTimer = 0;
		hasExploded = false;
		for (int i = 0; i < 8; i++) shrapnel[i].isActive = false;
	}
};

// === КОНКРЕТНАЯ АТАКА: МСТИТЕЛЬНЫЙ СНАРЯД ===
class MstitelnyyAttack : public ExplosiveAttack {
private:
	int projX, projY, speedX, speedY;

public:
	MstitelnyyAttack(int startX, int startY) : projX(startX), projY(startY), speedX(0), speedY(0) {}

	unique_ptr<Attack> update(int px, int py) override {
		if (collisionTimer > 0) {
			collisionTimer--;
		}
		else if (collisionTimer == 0 && (hitX != 0 || hitY != 0)) {
			hitX = 0; hitY = 0;
		}

		// Двигаем снаряд ТОЛЬКО если он ещё не взорвался
		if (!hasExploded) {
			if (stateTimer == 0 || stateTimer % 5 == 0) {
				int dx = px - projX;
				int dy = py - projY;
				int absDx = abs(dx);
				int absDy = abs(dy);

				speedX = 0; speedY = 0;
				if (absDx > absDy) speedX = (dx > 0) ? 1 : -1;
				else if (absDy > absDx) speedY = (dy > 0) ? 1 : -1;
				else {
					speedX = (dx > 0) ? 1 : -1;
					speedY = (dy > 0) ? 1 : -1;
				}
				if (speedX == 0 && speedY == 0) { speedX = -1; speedY = -1; }
			}

			stateTimer++;
			projX += speedX;
			projY += speedY;

			int distX = px - projX; if (distX < 0) distX = -distX;
			int distY = py - projY; if (distY < 0) distY = -distY;

			if (distX <= 1 && distY <= 1 && !isResolved) {
				hitX = px; hitY = py;
				playerHit = true;
				collisionTimer = 15;
				cout << "\a";
				hasExploded = true;
				isResolved = true;
				isActive = false;
			}
			else if (stateTimer > 35 && !isResolved) {
				triggerExplosion(projX, projY);
			}
			else if ((projX < 1 || projX > dlina || projY < 1 || projY > shirina) && !isResolved) {
				triggerExplosion(projX, projY);
			}
		}

		// Осколки обновляются всегда, пока живы
		updateShrapnelLogic(px, py);
		return nullptr;
	}

	void render(string& frame, int row, int col, int px, int py, bool& drawn) override {
		if (collisionTimer > 0 && playerHit && row == hitY && col == hitX) {
			frame += "*"; drawn = true;
		}
		else if (collisionTimer > 0 && !playerHit && row == hitY && col == hitX) {
			frame += "O"; drawn = true;
		}
		// Рисуем снаряд ТОЛЬКО если он ещё не взорвался
		else if (!hasExploded && row == projY && col == projX) {
			frame += "◄"; drawn = true;
		}

		renderShrapnel(frame, row, col, drawn);
	}
};

// === ПУЛЯ ===
class ProjectileAttack : public Attack {
public:
	int projX, projY, speedX, speedY;

	ProjectileAttack() : projX(0), projY(0), speedX(0), speedY(0) {}

	unique_ptr<Attack> update(int px, int py) override {
		if (collisionTimer > 0) collisionTimer--;

		if (stateTimer == 0) {
			projX = 2; projY = 2;
			speedX = 1; speedY = 1;
		}
		stateTimer++;
		projX += speedX;
		projY += speedY;

		int dx = px - projX; if (dx < 0) dx = -dx;
		int dy = py - projY; if (dy < 0) dy = -dy;

		if (dx <= 1 && dy <= 1 && !isResolved) {
			hitX = px; hitY = py;
			playerHit = true;
			collisionTimer = 15;
			cout << "\a";
			isResolved = true;
			isActive = false;
		}
		else if ((projX <= 1 || projX > dlina || projY <= 1 || projY > shirina) && !isResolved) {
			projX -= speedX;
			projY -= speedY;

			hitX = projX; hitY = projY;
			playerHit = false;
			collisionTimer = 15;
			isResolved = true;
			isActive = false;

			return make_unique<MstitelnyyAttack>(projX, projY);
		}
		return nullptr;
	}

	void render(string& frame, int row, int col, int px, int py, bool& drawn) override {
		if (collisionTimer > 0 && playerHit && row == hitY && col == hitX) {
			frame += "*"; drawn = true;
		}
		else if (collisionTimer > 0 && !playerHit && row == hitY && col == hitX) {
			frame += "O"; drawn = true;
		}
		else if (row == projY && col == projX) {
			frame += "►"; drawn = true;
		}
	}
};

// === ВОЛНА (горизонтальная) ===
class WaveAttack : public Attack {
public:
	int wavePos, waveGap, waveSpeedCounter;
	bool hasHitPlayer;

	WaveAttack() : wavePos(0), waveGap(0), waveSpeedCounter(0), hasHitPlayer(false) {}

	unique_ptr<Attack> update(int px, int py) override {
		if (collisionTimer > 0) collisionTimer--;

		if (stateTimer == 0) {
			wavePos = shirina;
			waveGap = 1 + rand() % dlina;
			waveSpeedCounter = 0;
		}

		// ЗАМОРОЗКА НА 0.5 СЕКУНДЫ (15 кадров * 30мс = 450мс)
		if (stateTimer < 15) {
			stateTimer++;
			return nullptr; // Просто ждем, волна уже нарисована на стартовой позиции
		}

		stateTimer++;
		waveSpeedCounter++;
		if (waveSpeedCounter >= 3) {
			wavePos--;
			waveSpeedCounter = 0;
		}

		bool isInWave = (py == wavePos && px != waveGap);

		if (isInWave && !hasHitPlayer && !isResolved) {
			hitX = px; hitY = py;
			playerHit = true;
			collisionTimer = 15;
			cout << "\a";
			hasHitPlayer = true;
			isResolved = true;
		}

		if (!isInWave) {
			hasHitPlayer = false;
			isResolved = false;
		}

		if (wavePos < 1) isActive = false;
		return nullptr;
	}

	void render(string& frame, int row, int col, int px, int py, bool& drawn) override {
		if (collisionTimer > 0 && playerHit && row == hitY && col == hitX) {
			frame += "*"; drawn = true;
		}
		else if (collisionTimer > 0 && !playerHit && row == hitY && col == hitX) {
			frame += "O"; drawn = true;
		}
		else if (row == wavePos && col != waveGap) {
			frame += "="; drawn = true;
		}
	}

	void reset() override {
		Attack::reset();
		hasHitPlayer = false;
	}
};

// === ВЕРТИКАЛЬНАЯ ВОЛНА ===
class VerticalWaveAttack : public Attack {
public:
	int wavePos, waveGap, waveSpeedCounter;
	bool hasHitPlayer;

	VerticalWaveAttack() : wavePos(0), waveGap(0), waveSpeedCounter(0), hasHitPlayer(false) {}

	unique_ptr<Attack> update(int px, int py) override {
		if (collisionTimer > 0) collisionTimer--;

		if (stateTimer == 0) {
			wavePos = 1;
			waveGap = 1 + rand() % shirina;
			waveSpeedCounter = 0;
		}

		// ЗАМОРОЗКА НА 0.5 СЕКУНДЫ (15 кадров * 30мс = 450мс)
		if (stateTimer < 15) {
			stateTimer++;
			return nullptr; // Просто ждем, волна уже нарисована на стартовой позиции
		}

		stateTimer++;
		waveSpeedCounter++;
		if (waveSpeedCounter >= 3) {
			wavePos++;
			waveSpeedCounter = 0;
		}

		bool isInWave = (px == wavePos && py != waveGap);

		if (isInWave && !hasHitPlayer && !isResolved) {
			hitX = px; hitY = py;
			playerHit = true;
			collisionTimer = 15;
			cout << "\a";
			hasHitPlayer = true;
			isResolved = true;
		}

		if (!isInWave) {
			hasHitPlayer = false;
			isResolved = false;
		}

		if (wavePos > shirina + 1) isActive = false;
		return nullptr;
	}

	void render(string& frame, int row, int col, int px, int py, bool& drawn) override {
		if (collisionTimer > 0 && playerHit && row == hitY && col == hitX) {
			frame += "*"; drawn = true;
		}
		else if (collisionTimer > 0 && !playerHit && row == hitY && col == hitX) {
			frame += "O"; drawn = true;
		}
		else if (col == wavePos && row != waveGap) {
			frame += "║"; drawn = true;
		}
	}

	void reset() override {
		Attack::reset();
		hasHitPlayer = false;
	}
};

// === МЕНЕДЖЕР ===
class AttackManager {
	vector<unique_ptr<Attack>> attacks;

public:
	void spawn(unique_ptr<Attack> attack) {
		attack->isActive = true;
		attacks.push_back(move(attack));
	}

	void updateAll(int px, int py) {
		for (auto& a : attacks) {
			// ИСПОЛЬЗУЕМ isAlive() ВМЕСТО ПРЯМОЙ ПРОВЕРКИ ФЛАГОВ
			if (a && a->isAlive()) {
				unique_ptr<Attack> spawned = a->update(px, py);
				if (spawned) {
					spawned->isActive = true;
					attacks.push_back(move(spawned));
				}
			}
		}

		for (auto& a : attacks) {
			if (a && !a->isAlive()) {
				a->reset();
			}
		}
	}

	void render(string& frame, int row, int col, int px, int py, bool& drawn) {
		for (auto& a : attacks) {
			if (a && a->isAlive()) {
				a->render(frame, row, col, px, py, drawn);
				if (drawn) break;
			}
		}
	}

	int activeCount() {
		int c = 0;
		for (auto& a : attacks) if (a && a->isActive) c++;
		return c;
	}
};

// === ФУНКЦИИ ===
void idiCursorVNachalo() {
	COORD coordinate{ 0, 0 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordinate);
}

void handleInput(int& px, int& py) {
	if ((GetAsyncKeyState('W') & 0x8000) && py > 1) py--;
	if ((GetAsyncKeyState('S') & 0x8000) && py < dlina) py++;
	if ((GetAsyncKeyState('A') & 0x8000) && px > 1) px--;
	if ((GetAsyncKeyState('D') & 0x8000) && px < shirina) px++;
}

// === ГЛАВНЫЙ ЦИКЛ ===
int main() {
	SetConsoleOutputCP(CP_UTF8);
	srand(static_cast<unsigned int>(time(nullptr)));

	int px = dlina / 2, py = shirina / 2;
	bool running = true;
	AttackManager manager;

	int step = 0;
	int timer = 0;
	int pattern[] = { 0, 1, 2, 0, 0, 1, 2, 3 };
	int patternLen = 8;

	while (running) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) running = false;
		handleInput(px, py);

		if (step == 0) {
			manager.spawn(make_unique<WarningAttack>());
			step++;
			timer = 0;
		}
		else if (timer >= 60 && step < patternLen) {
			switch (pattern[step]) {
			case 0: manager.spawn(make_unique<WarningAttack>()); break;
			case 1: manager.spawn(make_unique<ProjectileAttack>()); break;
			case 2: manager.spawn(make_unique<WaveAttack>()); break;
			case 3: manager.spawn(make_unique<VerticalWaveAttack>()); break;
			}
			step++;
			timer = 0;
		}
		else {
			timer++;
		}

		manager.updateAll(px, py);

		string frame;
		for (int row = 0; row <= dlina + 1; row++) {
			for (int col = 0; col <= shirina + 1; col++) {
				bool drawn = false;
				if (row == 0 || row == dlina + 1 || col == 0 || col == shirina + 1) {
					frame += "█"; drawn = true;
				}
				if (!drawn) manager.render(frame, row, col, px, py, drawn);
				if (!drawn && row == py && col == px) {
					frame += "☻"; drawn = true;
				}
				if (!drawn) frame += " ";
			}
			if (row < dlina + 1) frame += '\n';
		}

		frame += "\n========================\n";
		frame += "Активных атак: " + to_string(manager.activeCount()) + "\n";
		frame += "========================\n";

		idiCursorVNachalo();
		cout << frame;
		Sleep(30);
	}
	return 0;
}