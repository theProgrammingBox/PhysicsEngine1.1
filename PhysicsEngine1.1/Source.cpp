#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct Ball
{
	float x, y;
	float vx, vy;
	float radius;
	float mass;
	olc::Pixel color;
};

struct Link
{
	Ball* ball1 = nullptr;
	Ball* ball2 = nullptr;
	operator bool() const { return ball1 && ball2; }
};

class SystemVisualizer : public olc::PixelGameEngine
{
public:
	std::vector<Ball> balls;
	std::vector<Link> links;
	std::vector<Ball*> highlightedBalls;
	float mouseRadius = 0.0f;
	
	SystemVisualizer()
	{
		sAppName = "System Visualizer";
	}
	
	bool OnUserCreate() override
	{
		return true;
	}

	void KeyboardInput()
	{
		if (GetKey(olc::SPACE).bPressed)
		{
			Ball ball;
			ball.x = GetMouseX();
			ball.y = GetMouseY();
			ball.vx = 0.0f;
			ball.vy = 0.0f;
			ball.radius = 10.0f;
			ball.mass = 1.0f;
			ball.color = olc::GREY;
			balls.emplace_back(ball);
		}
	}

	void MouseInput()
	{
		highlightedBalls.clear();
		for (auto& ball : balls)
		{
			float dx = ball.x - GetMouseX();
			float dy = ball.y - GetMouseY();
			float totalRadius = mouseRadius + ball.radius;
			float distanceSquared = dx * dx + dy * dy;
			if (*(int32_t*)&(distanceSquared -= totalRadius * totalRadius) & 0x80000000)
				highlightedBalls.emplace_back(&ball);
		}
		if (GetMouseWheel() > 0)
			mouseRadius += 1.0f;
		else if (GetMouseWheel() < 0)
			mouseRadius -= 1.0f;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		KeyboardInput();
		MouseInput();
		Render();
		
		return true;
	}

	void Render()
	{
		Clear(olc::BLACK);
		for (auto& ball : balls)
		{
			DrawCircle(ball.x, ball.y, ball.radius, ball.color);
		}
		for (auto& link : links)
		{
			if (link)
				DrawLine(link.ball1->x, link.ball1->y, link.ball2->x, link.ball2->y, olc::WHITE);
		}
		for (auto& ball : highlightedBalls)
		{
			DrawCircle(ball->x, ball->y, ball->radius, olc::RED);
		}
		DrawCircle(GetMouseX(), GetMouseY(), mouseRadius, olc::RED);
	}
};

int main()
{
	SystemVisualizer systemVisualizer;
	if (systemVisualizer.Construct(1600, 900, 1, 1))
		systemVisualizer.Start();
	return 0;
}