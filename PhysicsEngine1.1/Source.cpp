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
	olc::vf2d pos1;
	olc::vf2d pos2;
	float length;
	olc::vf2d normal;
	olc::Pixel color;

	Wall(olc::vf2d _pos1, olc::vf2d _pos2, olc::Pixel _color = olc::WHITE)
		: pos1(_pos1), pos2(_pos2), color(_color)
	{
		const olc::vf2d delta = pos2 - pos1;
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
		for (int i = 0; i < 1024; i++)
			balls.push_back(Ball(
				olc::vf2d(FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight())),
				olc::vf2d(FloatRand(-100, 100), FloatRand(-100, 100)),
				FloatRand(2, 16),
				1.0f,
				0.5f
			));

		for (int i = 0; i < 8; i++)
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
		olc::vf2d acc = { 0, 0 };
		if (GetKey(olc::Key::W).bHeld)
			acc.y -= 1;
		if (GetKey(olc::Key::S).bHeld)
			acc.y +=1;
		if (GetKey(olc::Key::A).bHeld)
			acc.x -= 1;
		if (GetKey(olc::Key::D).bHeld)
			acc.x += 1;
		float mag2 = acc.mag2();
		if (mag2 > 0)
			balls.front().acc = acc * 0.002f / sqrt(mag2);
	}

	void Collision()
	{
		for (int i = 0; i < balls.size() - 1; i++)
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

	void Render()
	{
		for (auto& ball : balls)
		{
			DrawCircle(ball.pos, ball.radius, ball.color);

			DrawLine(ball.pos, ball.pos + ball.vel * 0.1f, olc::RED);
			DrawLine(ball.pos, ball.pos + ball.acc, olc::GREEN);
		}

		for (auto& wall : walls)
			DrawLine(wall.pos1, wall.pos2, wall.color);
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Unrender();
		Controls();
		Simulate(fElapsedTime, 10);
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