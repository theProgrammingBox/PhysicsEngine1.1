#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

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
			balls.front().acc = acc * 0.02f / sqrt(mag2);
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