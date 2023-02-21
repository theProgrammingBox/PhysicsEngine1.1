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
	Ball* highlightedBall = nullptr;
	olc::Pixel highlightedPastColor;
	
	SystemVisualizer()
	{
		sAppName = "System Visualizer";
	}
	
	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
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
			balls.push_back(ball);
		}
		
		float minDistance = FLT_MAX;
		Ball* closestBall = nullptr;
		for (auto& ball : balls)
		{
			float dx = ball.x - GetMouseX();
			float dy = ball.y - GetMouseY();
			float distance = dx * dx + dy * dy;
			if (distance < 100.0f && distance < minDistance)
			{
				minDistance = distance;
				closestBall = &ball;
			}
		}

		if (highlightedBall != closestBall)
		{
			if (highlightedBall)
				highlightedBall->color = highlightedPastColor;
			if (closestBall)
			{
				highlightedPastColor = closestBall->color;
				closestBall->color = olc::RED;
			}
			highlightedBall = closestBall;
		}

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
		
		return true;
	}

	void Render()
	{
	}
};

int main()
{
	SystemVisualizer systemVisualizer;
	if (systemVisualizer.Construct(1600, 900, 1, 1))
		systemVisualizer.Start();
	return 0;
}