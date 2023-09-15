#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct Ball
{
	olc::vf2d pos;
	olc::vf2d vel;
	olc::vf2d acc;
	float radius;
	float density;
	float invMass;
	float invElasticity;
	olc::Pixel color;

	Ball(olc::vf2d _pos, olc::vf2d _vel, float _radius, float _density = 1.0f, float _elasticity = 1.0f, olc::Pixel _color = olc::WHITE)
		: pos(_pos), vel(_vel), acc(olc::vf2d(0, 0)), radius(_radius), density(_density), invMass(1.0f / (_density * 3.14159f * _radius * _radius)), invElasticity(1.0f / _elasticity), color(_color)
	{}
};

struct Wall
{
	olc::vf2d refPos1;
	olc::vf2d refPos2;
	olc::vf2d center;
	float angle;
	float angleVel;
	float length;
	olc::vf2d normal;
	olc::vf2d pos1;
	olc::vf2d pos2;
	olc::Pixel color;

	Wall(olc::vf2d _pos1, olc::vf2d _pos2, olc::Pixel _color = olc::WHITE)
		: angle(0), angleVel(0), color(_color)
	{
		const olc::vf2d delta = _pos2 - _pos1;
		center = _pos1 + delta * 0.5f;
		refPos1 = _pos1 - center;
		refPos2 = _pos2 - center;
		length = delta.mag();
		normal = delta / length;
	}
};

float FloatRand(float min, float max)
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

class Example : public olc::PixelGameEngine
{
public:
	std::vector<Ball> balls;
	std::vector<Wall> walls;

	bool OnUserCreate() override
	{
		for (int i = 0; i < 0; i++)
			balls.push_back(Ball(
				olc::vf2d(FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight())),
				olc::vf2d(FloatRand(-100, 100), FloatRand(-100, 100)),
				FloatRand(2, 16),
				1.0f,
				0.5f
			));

		for (int i = 0; i < 1; i++)
			walls.push_back(Wall(
				olc::vf2d(FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight())),
				olc::vf2d(FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight()))
			));

		return true;
	}

	void Unrender()
	{
		for (auto& ball : balls)
		{
			DrawCircle(ball.pos, ball.radius, olc::BLACK);

			DrawLine(ball.pos, ball.pos + ball.vel * 0.1f, olc::BLACK);
			DrawLine(ball.pos, ball.pos + ball.acc, olc::BLACK);
		}

		for (auto& wall : walls)
			DrawLine(wall.pos1, wall.pos2, olc::BLACK);
	}

	void Controls()
	{
		/*olc::vf2d acc = { 0, 0 };
		if (GetKey(olc::Key::W).bHeld)
			acc.y -= 1;
		if (GetKey(olc::Key::S).bHeld)
			acc.y += 1;
		if (GetKey(olc::Key::A).bHeld)
			acc.x -= 1;
		if (GetKey(olc::Key::D).bHeld)
			acc.x += 1;
		float mag2 = acc.mag2();
		if (mag2 > 0)
			balls.front().acc = acc * 0.002f / sqrt(mag2);*/
		float angleVel = 0;
		if (GetKey(olc::Key::A).bHeld)
			angleVel -= 1;
		if (GetKey(olc::Key::D).bHeld)
			angleVel += 1;
		walls.front().angleVel += angleVel * 0.01f;
	}

	void Collision()
	{
		for (auto& ball : balls)
		{
			for (auto& wall : walls)
			{
				const float ballToWall = (ball.pos - wall.pos1).dot(wall.normal);
				const olc::vf2d closestPoint = wall.pos1 + wall.normal * std::max(0.0f, std::min(wall.length, ballToWall));

				float dist = (ball.pos - closestPoint).mag();
				if (dist < ball.radius && dist > 0.01f)
				{
					const olc::vf2d ballNormal = (closestPoint - ball.pos) / dist;
					const float mass = 2.0f;
					const float elasticity = 2.0f / (ball.invElasticity + 1.0f);
					const float dot = ball.vel.dot(ballNormal) * elasticity * mass;
					if (dot > 0)
						ball.vel -= ballNormal * dot;
				}
			}
		}

		for (int i = 0; i < int(balls.size()) - 1; i++)
		{
			for (int j = i + 1; j < balls.size(); j++)
			{
				const olc::vf2d delta = balls[j].pos - balls[i].pos;
				const float dist = delta.mag();
				if (dist < balls[j].radius + balls[i].radius && dist > 0.01f)
				{
					const olc::vf2d normal = delta / dist;
					const float mass = 2.0f / (balls[i].invMass + balls[j].invMass);
					const float elasticity = 2.0f / (balls[i].invElasticity + balls[j].invElasticity);
					const float dot = (balls[i].vel - balls[j].vel).dot(normal) * mass * elasticity;
					if (dot > 0)
					{
						balls[i].vel -= normal * dot * balls[i].invMass;
						balls[j].vel += normal * dot * balls[j].invMass;
					}
				}
			}
		}

		for (auto& ball : balls)
		{
			if (ball.pos.x < ball.radius)
			{
				ball.pos.x = ball.radius;
				ball.vel.x *= -1;
			}
			if (ball.pos.x > ScreenWidth() - ball.radius)
			{
				ball.pos.x = ScreenWidth() - ball.radius;
				ball.vel.x *= -1;
			}
			if (ball.pos.y < ball.radius)
			{
				ball.pos.y = ball.radius;
				ball.vel.y *= -1;
			}
			if (ball.pos.y > ScreenHeight() - ball.radius)
			{
				ball.pos.y = ScreenHeight() - ball.radius;
				ball.vel.y *= -1;
			}
		}
	}

	void Update(float dt)
	{
		for (auto& ball : balls)
		{
			ball.vel += ball.acc * dt;
			ball.pos += ball.vel * dt;

			// this overides control ball acc display, fix
			ball.acc = { 0, 10 };
			//ball.vel *= 0.999f;
		}

		for (auto& wall : walls)
		{
			wall.angle += wall.angleVel * dt;
			float c = cos(wall.angle);
			float s = sin(wall.angle);
			olc::vf2d rot0 = { c, s };
			olc::vf2d rot1 = { -s, c };
			wall.pos1 = wall.center + olc::vf2d(rot0.dot(wall.refPos1), rot1.dot(wall.refPos1));
			wall.pos2 = wall.center + olc::vf2d(rot0.dot(wall.refPos2), rot1.dot(wall.refPos2));
		}
	}

	void Simulate(float _dt, int ySteps)
	{
		const float dt = _dt / ySteps;
		for (int i = 0; i < ySteps; i++)
		{
			Collision();
			Update(dt);
		}
	}

	void FillTriangle2(float x1, float y1, float x2, float y2, float x3, float y3, olc::Pixel p = olc::WHITE)
	{
		int scale = 30;
		x1 = x1 / scale;
		x2 = x2 / scale;
		x3 = x3 / scale;
		y1 = y1 / scale;
		y2 = y2 / scale;
		y3 = y3 / scale;
		auto drawline = [&](int sx, int ex, int ny, int size) { for (int i = sx; i <= ex; i++) DrawRect(i * scale, ny * scale, size, size, p); };
		//auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) Draw(i, ny, p); };

		if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
		if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3); }
		if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3); }

		int currentTruncY = y1, currentTruncX13 = x1, currentTruncX12 = x2, maxX, minX;
		float dy = y3 - y1;
		if (dy > 0)
		{
			float dx = x3 - x1;
			float temp = (dy * dy) / (dx * dx);
			float projectedStepx13 = sqrt(1 + temp);
			float projectedStepy13 = sqrt(1 + 1.0f / temp);

			int truncStepx13;
			float totalProjectedx13;
			if (dx < 0) {
				truncStepx13 = -1;
				totalProjectedx13 = (x1 - currentTruncX13) * projectedStepx13;
			}
			else {
				truncStepx13 = 1;
				totalProjectedx13 = (currentTruncX13 + 1.0f - x1) * projectedStepx13;
			}
			float totalProjectedy13 = (currentTruncY + 1.0f - y1) * projectedStepy13;
			
			int savedTruncX13, savedTruncX12;

			float ySteps = (int)y2 - currentTruncY;
			if (ySteps > 0)
			{
				currentTruncX12 = x1;

				dx = x2 - x1;
				dy = y2 - y1;
				temp = (dy * dy) / (dx * dx);
				float projectedStepx12 = sqrt(1 + temp);
				float projectedStepy12 = sqrt(1 + 1.0f / temp);

				int truncStepx12;
				float totalProjectedx12;
				if (dx < 0) {
					truncStepx12 = -1;
					totalProjectedx12 = (x1 - currentTruncX12) * projectedStepx12;
				}
				else {
					truncStepx12 = 1;
					totalProjectedx12 = (currentTruncX12 + 1.0f - x1) * projectedStepx12;
				}
				float totalProjectedy12 = (currentTruncY + 1.0f - y1) * projectedStepy12;

				for (int i = ySteps; i--;)
				{
					savedTruncX12 = currentTruncX12;
					savedTruncX13 = currentTruncX13;

					while (totalProjectedx12 < totalProjectedy12) {
						currentTruncX12 += truncStepx12;
						totalProjectedx12 += projectedStepx12;
					}

					while (totalProjectedx13 < totalProjectedy13) {
						currentTruncX13 += truncStepx13;
						totalProjectedx13 += projectedStepx13;
					}

					maxX = std::max(currentTruncX12, std::max(currentTruncX13, std::max(savedTruncX12, savedTruncX13)));
					minX = std::min(currentTruncX12, std::min(currentTruncX13, std::min(savedTruncX12, savedTruncX13)));
					drawline(minX, maxX, currentTruncY, 30);

					currentTruncY++;
					totalProjectedy12 += projectedStepy12;
					totalProjectedy13 += projectedStepy13;
				}
			}

			currentTruncX12 = x2;
			ySteps = (int)y3 - currentTruncY;
			if (ySteps > 0)
			{
				dx = x3 - x2;
				dy = y3 - y2;
				temp = (dy * dy) / (dx * dx);
				float projectedStepx23 = sqrt(1 + temp);
				float projectedStepy23 = sqrt(1 + 1.0f / temp);

				int truncStepx23;
				float totalProjectedx23;
				if (dx < 0) {
					truncStepx23 = -1;
					totalProjectedx23 = (x2 - currentTruncX12) * projectedStepx23;
				}
				else {
					truncStepx23 = 1;
					totalProjectedx23 = (currentTruncX12 + 1.0f - x2) * projectedStepx23;
				}
				float totalProjectedy23 = (currentTruncY + 1.0f - y2) * projectedStepy23;

				for (int i = ySteps; i--;)
				{
					savedTruncX13 = currentTruncX13;
					savedTruncX12 = currentTruncX12;

					while (totalProjectedx13 < totalProjectedy13) {
						currentTruncX13 += truncStepx13;
						totalProjectedx13 += projectedStepx13;
					}

					while (totalProjectedx23 < totalProjectedy23) {
						currentTruncX12 += truncStepx23;
						totalProjectedx23 += projectedStepx23;
					}

					maxX = std::max(currentTruncX13, std::max(currentTruncX12, std::max(savedTruncX13, savedTruncX12)));
					minX = std::min(currentTruncX13, std::min(currentTruncX12, std::min(savedTruncX13, savedTruncX12)));

					drawline(minX, maxX, currentTruncY, 20);
					currentTruncY++;
					totalProjectedy13 += projectedStepy13;
					totalProjectedy23 += projectedStepy23;
				}
			}
		}

		maxX = std::max(currentTruncX13, std::max(currentTruncX12, (int)x3));
		minX = std::min(currentTruncX13, std::min(currentTruncX12, (int)x3));
		drawline(minX, maxX, currentTruncY, 10);
	}

	void FillCircle2(float x, float y, float radius, olc::Pixel p = olc::WHITE)
	{
		int scale = 30;
		x = x / scale;
		y = y / scale;
		radius = radius / scale;
		auto drawline = [&](int sx, int ex, int ny, int size = 30) { for (int i = sx; i <= ex; i++) DrawRect(i * scale, ny * scale, size, size, p); };
		auto notInCircle = [&](float xx, float yy, float rr) { return xx * xx + yy * yy > rr * rr; };

		float yy = 0.5 - radius;
		for (int i = ceil(2 * radius); i--;)
		{
			float xx = 0.5f - radius;
			while (notInCircle(int(xx + 0.5), int(yy + 0.5f), radius)) xx++;
			int sx = xx;
			while (!notInCircle(int(xx - 0.5), int(yy + 0.5f), radius)) xx++;
			int ex = xx;
			drawline(sx + x, ex + x, yy + y);
			yy++;
		}
	}

	/*
	you want n by m units for the map
	you want n by m blocks for the collision grid
	you have a n and m scalar to scale from units to blocks
	*/

	int x1 = 500;
	int y1 = 100;
	int x2 = 1000;
	int y2 = 400;
	int x3 = 200;
	int y3 = 700;

	int* x = &x1;
	int* y = &y1;

	void Render()
	{
		/*for (auto& ball : balls)
		{
			DrawCircle(ball.pos, ball.radius, ball.color);

			DrawLine(ball.pos, ball.pos + ball.vel * 0.1f, olc::RED);
			DrawLine(ball.pos, ball.pos + ball.acc, olc::GREEN);
		}

		for (auto& wall : walls)
			DrawLine(wall.pos1, wall.pos2, wall.color);*/

		/*FT(x1, y1, x2, y2, x3, y3, olc::BLACK);*/
		//for (int i = 100; i--;)
			//FillTriangle2(x1, y1, x2, y2, x3, y3, olc::BLACK);
			//FillTriangle(x1, y1, x2, y2, x3, y3, olc::BLACK);
		//DrawTriangle(x1, y1, x2, y2, x3, y3, olc::BLACK);

		FillCircle2(x1, y1, 127, olc::BLACK);
		DrawCircle(x1, y1, 127, olc::BLACK);

		if (GetKey(olc::Key::K1).bPressed)
			x = &x1, y = &y1;
		if (GetKey(olc::Key::K2).bPressed)
			x = &x2, y = &y2;
		if (GetKey(olc::Key::K3).bPressed)
			x = &x3, y = &y3;

		if (GetMouse(0).bHeld)
		{
			*x = GetMouseX();
			*y = GetMouseY();
		}

		//for (int i = 100; i--;)
			//FillTriangle2(x1, y1, x2, y2, x3, y3);
			//FillTriangle(x1, y1, x2, y2, x3, y3);
		//DrawTriangle(x1, y1, x2, y2, x3, y3, olc::RED);

		FillCircle2(x1, y1, 127);
		DrawCircle(x1, y1, 127, olc::RED);
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		/*Unrender();
		Controls();
		Simulate(fElapsedTime, 10);*/
		Render();

		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(1400, 800, 1, 1))
		demo.Start();
	return 0;
}