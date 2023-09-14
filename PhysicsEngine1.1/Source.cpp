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

	void Simulate(float _dt, int steps)
	{
		const float dt = _dt / steps;
		for (int i = 0; i < steps; i++)
		{
			Collision();
			Update(dt);
		}
	}

	void FT(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, olc::Pixel p = olc::WHITE)
	{
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) Draw(i, ny, p); };

		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;

		// Sort vertices
		if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
		if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3); }
		if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3); }


		t1x = t2x = x1; y = y1;   // Starting points
		dx1 = (int)(x2 - x1);
		if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y2 - y1);

		dx2 = (int)(x3 - x1);
		if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
		else signx2 = 1;
		dy2 = (int)(y3 - y1);

		if (dy1 > dx1) { std::swap(dx1, dy1); changed1 = true; }
		if (dy2 > dx2) { std::swap(dy2, dx2); changed2 = true; }

		e2 = (int)(dx2 >> 1);
		// Flat top, just process the second half
		if (y1 == y2) goto next;
		e1 = (int)(dx1 >> 1);

		for (int i = 0; i < dx1;) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) t1xp = signx1;//t1x += signx1;
					else          goto next1;
				}
				if (changed1) break;
				else t1x += signx1;
			}
			// Move line
		next1:
			// process second line until y value is about to change
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;//t2x += signx2;
					else          goto next2;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next2:
			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);    // Draw line from min to max points found on the y
			// Now increase y
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y == y2) break;
		}
	next:
		// Second half
		dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y3 - y2);
		t1x = x2;

		if (dy1 > dx1) {   // swap values
			std::swap(dy1, dx1);
			changed1 = true;
		}
		else changed1 = false;

		e1 = (int)(dx1 >> 1);

		for (int i = 0; i <= dx1; i++) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) { t1xp = signx1; break; }//t1x += signx1;
					else          goto next3;
				}
				if (changed1) break;
				else   	   	  t1x += signx1;
				if (i < dx1) i++;
			}
		next3:
			// process second line until y value is about to change
			while (t2x != x3) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;
					else          goto next4;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next4:

			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y > y3) return;
		}
	}

	void FT2(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, olc::Pixel p = olc::WHITE)
	{
		x1 = x1 / 100;
		x2 = x2 / 100;
		x3 = x3 / 100;
		y1 = y1 / 100;
		y2 = y2 / 100;
		y3 = y3 / 100;
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) DrawRect(i * 100, ny * 100, 100, 100, p); };

		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;
		// Sort vertices
		if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
		if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3); }
		if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3); }

		t1x = t2x = x1; y = y1;   // Starting points
		dx1 = (int)(x2 - x1);
		if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y2 - y1);

		dx2 = (int)(x3 - x1);
		if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
		else signx2 = 1;
		dy2 = (int)(y3 - y1);

		if (dy1 > dx1) { std::swap(dx1, dy1); changed1 = true; }
		if (dy2 > dx2) { std::swap(dy2, dx2); changed2 = true; }

		e2 = (int)(dx2 >> 1);
		// Flat top, just process the second half
		if (y1 == y2) goto next;
		e1 = (int)(dx1 >> 1);

		for (int i = 0; i < dx1;) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) t1xp = signx1;//t1x += signx1;
					else          goto next1;
				}
				if (changed1) break;
				else t1x += signx1;
			}
			// Move line
		next1:
			// process second line until y value is about to change
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;//t2x += signx2;
					else          goto next2;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next2:
			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);    // Draw line from min to max points found on the y
			// Now increase y
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y == y2) break;
		}
	next:
		// Second half
		dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y3 - y2);
		t1x = x2;

		if (dy1 > dx1) {   // swap values
			std::swap(dy1, dx1);
			changed1 = true;
		}
		else changed1 = false;

		e1 = (int)(dx1 >> 1);

		for (int i = 0; i <= dx1; i++) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) { t1xp = signx1; break; }//t1x += signx1;
					else          goto next3;
				}
				if (changed1) break;
				else   	   	  t1x += signx1;
				if (i < dx1) i++;
			}
		next3:
			// process second line until y value is about to change
			while (t2x != x3) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;
					else          goto next4;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next4:

			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y > y3) return;
		}
	}

	void DDA(float x1, float y1, float x2, float y2, int scale = 1,  olc::Pixel p = olc::WHITE)
	{
		x1 /= scale;
		y1 /= scale;
		x2 /= scale;
		y2 /= scale;

		float dx = x2 - x1;
		float dy = y2 - y1;
		float temp = (dy * dy) / (dx * dx);
		float projectedStepx = sqrt(1 + temp);
		float projectedStepy = sqrt(1 + 1.0f / temp);

		int currentTruncx = x1;
		int currentTruncy = y1;
		int truncStepx;
		int truncStepy;
		float totalProjectedx;
		float totalProjectedy;

		if (dx < 0) {
			truncStepx = -1;
			totalProjectedx = (x1 - currentTruncx) * projectedStepx;
		} else {
			truncStepx = 1;
			totalProjectedx = (currentTruncx + 1.0f - x1) * projectedStepx;
		}

		if (dy < 0) {
			truncStepy = -1;
			totalProjectedy = (y1 - currentTruncy) * projectedStepy;
		} else {
			truncStepy = 1;
			totalProjectedy = (currentTruncy + 1.0f - y1) * projectedStepy;
		}

		int steps = abs((int)x2 - currentTruncx) + abs((int)y2 - currentTruncy);
		DrawRect(currentTruncx* scale, currentTruncy* scale, scale, scale, p == olc::BLACK ? p : olc::RED);
		for (int i = steps; i--;)
		{
			if (totalProjectedx < totalProjectedy) {
				currentTruncx += truncStepx;
				totalProjectedx += projectedStepx;
				DrawRect(currentTruncx * scale, currentTruncy * scale, scale, scale, p == olc::BLACK ? p : olc::GREEN);
			} else {
				currentTruncy += truncStepy;
				totalProjectedy += projectedStepy;
				DrawRect(currentTruncx * scale, currentTruncy * scale, scale, scale, p == olc::BLACK ? p : olc::BLUE);
			}
		}

		/*
		int steps = abs((int)y2 - currentTruncy);
		//DrawRect(currentTruncx* scale, currentTruncy* scale, scale, scale, p);
		for (int i = steps; i--;)
		{
			while(true)
			{
				if (totalProjectedx < totalProjectedy) {
					currentTruncx += truncStepx;
					totalProjectedx += projectedStepx;
				} else {
					currentTruncy += truncStepy;
					totalProjectedy += projectedStepy;
					DrawRect(currentTruncx * scale, currentTruncy * scale, scale, scale, p);
					break;
				}
			}
		}
		*/
	}

	void FT2(float x1, float y1, float x2, float y2, float x3, float y3, int scale = 1, olc::Pixel p = olc::WHITE)
	{
		x1 = x1 / scale;
		x2 = x2 / scale;
		x3 = x3 / scale;
		y1 = y1 / scale;
		y2 = y2 / scale;
		y3 = y3 / scale;
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) DrawRect(i * scale, ny * scale, scale, scale, p); };

		float dx, dy, temp;

		if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
		if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3); }
		if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3); }

		dx = x3 - x1;
		dy = y3 - y1;
		temp = (dy * dy) / (dx * dx);
		float projectedStepx13 = sqrt(1 + temp);
		float projectedStepy13 = sqrt(1 + 1.0f / temp);

		int currentTruncx13 = x1;
		int currentTruncy13 = y1;
		int truncStepx13;
		int truncStepy13;
		float totalProjectedx13;
		float totalProjectedy13;

		if (dx < 0) {
			truncStepx13 = -1;
			totalProjectedx13 = (x1 - currentTruncx13) * projectedStepx13;
		}
		else {
			truncStepx13 = 1;
			totalProjectedx13 = (currentTruncx13 + 1.0f - x1) * projectedStepx13;
		}

		if (dy < 0) {
			truncStepy13 = -1;
			totalProjectedy13 = (y1 - currentTruncy13) * projectedStepy13;
		}
		else {
			truncStepy13 = 1;
			totalProjectedy13 = (currentTruncy13 + 1.0f - y1) * projectedStepy13;
		}

		int steps = abs((int)x2 - currentTruncx13) + abs((int)y2 - currentTruncy13);

		if (y1 == y2) goto next;

	next:
		if (true);
	}

	/*
	you want n by m units for the map
	you want n by m blocks for the collision grid
	you have a n and m scalar to scale from units to blocks
	*/

	int x1 = 170;
	int y1 = 340;
	int x2 = 200;
	int y2 = 200;
	int x3 = 150;
	int y3 = 200;

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
		/*FT(x1, y1, x2, y2, x3, y3, olc::BLACK);
		FT2(x1, y1, x2, y2, x3, y3, olc::BLACK);*/
		DDA(x1, y1, x2, y2, 1, olc::BLACK);
		DDA(x1, y1, x2, y2, 30, olc::BLACK);

		if (GetKey(olc::Key::K1).bPressed)
			x = &x1, y = &y1;
		if (GetKey(olc::Key::K2).bPressed)
			x = &x2, y = &y2;
		if (GetKey(olc::Key::K3).bPressed)
			x = &x3, y = &y3;

		// set x and y to mouse position
		if (GetMouse(0).bHeld)
		{
			*x = GetMouseX();
			*y = GetMouseY();
		}

		/*FT(x1, y1, x2, y2, x3, y3);
		FT2(x1, y1, x2, y2, x3, y3);*/
		DDA(x1, y1, x2, y2);
		DDA(x1, y1, x2, y2, 30);
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