#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

float InvSqrt(float number)
{
	long i = 0x5F1FFFF9 - (*(long*)&number >> 1);
	float tmp = *(float*)&i;
	return tmp * 0.703952253f * (2.38924456f - number * tmp * tmp);
}

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
	olc::vf2d normal;
	olc::Pixel color;

	Wall(olc::vf2d _pos1, olc::vf2d _pos2, olc::Pixel _color = olc::WHITE)
		: pos1(_pos1), pos2(_pos2), normal( pos2 - pos1 ), color(_color)
	{}
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
		for (int i = 0; i < 16; i++)
			balls.push_back(Ball(
				{ FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight()) },
				{ FloatRand(-1, 1), FloatRand(-1, 1) },
				FloatRand(2, 16)
			));

		for (int i = 0; i < 4; i++)
			walls.push_back(Wall(
				{ FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight()) },
				{ FloatRand(0, ScreenWidth()), FloatRand(0, ScreenHeight()) }
			));

		return true;
	}

	void Unrender()
	{
		for (auto& ball : balls)
		{
			DrawCircle(ball.pos, ball.radius, olc::BLACK);

			DrawLine(ball.pos, ball.pos + ball.vel * 10, olc::BLACK);
			DrawLine(ball.pos, ball.pos + ball.acc * 100, olc::BLACK);
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
			balls.front().acc = acc * 0.002f * InvSqrt(mag2);
	}

	void Collision()
	{
		for (int i = 0; i < balls.size() - 1; i++)
		{
			for (int j = i + 1; j < balls.size(); j++)
			{
				olc::vf2d delta = balls[j].pos - balls[i].pos;
				float dist2 = delta.mag2();
				float radii = balls[j].radius + balls[i].radius;
				if (dist2 < radii * radii && dist2 > 0.0001f)
				{
					olc::vf2d normal = delta / sqrt(dist2);
					olc::vf2d vecc = balls[i].vel - balls[j].vel;
					float elasticity = 2.0f / (balls[i].invElasticity + balls[j].invElasticity);
					float mass = 2.0f / (balls[i].invMass + balls[j].invMass);
					float dot = vecc.dot(normal) * mass * elasticity;
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
				olc::vf2d wallDelta = wall.pos2 - wall.pos1;
				float wallLength = wallDelta.mag();
				olc::vf2d wallNormal = wallDelta / wallLength;
				olc::vf2d ballDelta = ball.pos - wall.pos1;
				float dot = ballDelta.dot(wallNormal);
				dot = std::max(0.0f, std::min(dot, wallLength));
				olc::vf2d closestPoint = wall.pos1 + wallNormal * dot;

				float dist2 = (ball.pos - closestPoint).mag2();
				if (dist2 < ball.radius * ball.radius && dist2 > 0.0001f)
				{
					olc::vf2d ballNormal = (ball.pos - closestPoint) / sqrt(dist2);
					float elasticity = 2.0f / (ball.invElasticity + 1.0f);
					float dot = ball.vel.dot(ballNormal) * elasticity * 2.0f;
					if (dot < 0)
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
			ball.acc = { 0, 0 };
			//ball.vel *= 0.999f;
		}
	}

	void Render()
	{
		for (auto& ball : balls)
		{
			DrawCircle(ball.pos, ball.radius, ball.color);

			DrawLine(ball.pos, ball.pos + ball.vel * 10, olc::RED);
			DrawLine(ball.pos, ball.pos + ball.acc * 100, olc::GREEN);
		}

		for (auto& wall : walls)
			DrawLine(wall.pos1, wall.pos2, wall.color);
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Unrender();
		Controls();
		Collision();
		Update(fElapsedTime * 100);
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