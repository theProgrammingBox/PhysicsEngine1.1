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
	olc::Pixel color;

	Ball(olc::vf2d _pos, olc::vf2d _vel, olc::vf2d _acc, float _radius, olc::Pixel _color)
		: pos(_pos), vel(_vel), acc(_acc), radius(_radius), color(_color)
	{}
};

class Example : public olc::PixelGameEngine
{
public:
	std::vector<Ball> balls;

	bool OnUserCreate() override
	{
		for (int i = 0; i < 100; i++)
			balls.push_back(Ball(
				{ float(rand() % ScreenWidth()), float(rand() % ScreenHeight()) },
				{ 0, 0 },
				{ 0, 0 },
				10.0f,
				olc::Pixel(rand() % 255, rand() % 255, rand() % 255)
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
			balls.front().acc = acc * 0.02f * InvSqrt(mag2);
	}

	void Collision()
	{
		for (int i = 0; i < balls.size(); i++)
		{
			for (int j = i + 1; j < balls.size(); j++)
			{
				olc::vf2d delta = balls[j].pos - balls[i].pos;
				float dist2 = delta.mag2();
				float radii = balls[j].radius + balls[i].radius;
				if (dist2 < radii * radii && dist2 > 0.0001f)
				{
					float dist = sqrt(dist2);
					olc::vf2d normal = delta / dist;
					olc::vf2d tangent = { -normal.y, normal.x };
					float dpTan1 = balls[i].vel.dot(tangent);
					float dpTan2 = balls[j].vel.dot(tangent);
					float dpNorm1 = balls[i].vel.dot(normal);
					float dpNorm2 = balls[j].vel.dot(normal);
					float m1 = (dpNorm1 * (balls[i].radius - balls[j].radius) + 2.0f * balls[j].radius * dpNorm2) / (balls[i].radius + balls[j].radius);
					float m2 = (dpNorm2 * (balls[j].radius - balls[i].radius) + 2.0f * balls[i].radius * dpNorm1) / (balls[i].radius + balls[j].radius);
					balls[i].vel = tangent * dpTan1 + normal * m1;
					balls[j].vel = tangent * dpTan2 + normal * m2;
					balls[i].pos = balls[j].pos - normal * radii;
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

	void Update()
	{
		for (auto& ball : balls)
		{
			ball.vel += ball.acc;
			ball.acc = { 0, 0 };
			ball.vel *= 0.99f;
			ball.pos += ball.vel;
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
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Unrender();
		Controls();
		Collision();
		Update();
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