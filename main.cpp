#include <raylib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <raymath.h>
#include <deque>

using namespace std;
//Load resources
Texture2D spaceship_texture;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGH = 728;

class Bullet
{
public:
	Vector2 position = { 0, 0 };
	Vector2 direction = { 1.0f, 0.0f };
	Rectangle shape = { position.x, position.y, 10, 10 };
	float bullet_speed = 5.0;
	int val = 0;

	Bullet(Vector2 spawn_position, Vector2 dir) : position(spawn_position) , direction(dir){}
	
	void spawned()
	{
		
	}

	void Move()
	{
		Vector2 dir = { direction.x * bullet_speed, direction.y* bullet_speed };
		position = Vector2Add(position, dir);
		shape.x = position.x;
		shape.y = position.y;
	}

	void Draw()
	{
		DrawRectangleRec(shape, YELLOW);
	}
};

class PlayerTrail
{
	deque<Vector2> trail_points = {};
	int trail_lenght = 20;
	double last_spawn_time = 0.0;
public:
	PlayerTrail(){}

	void Update(Vector2 position)
	{
		if (refresh_time(1.0))
		{
			if (trail_points.size() >= trail_lenght)
			{
				trail_points.pop_back();
			}
			Vector2 _pos = { position.x + GetRandomValue(-3, 3), position.y + GetRandomValue(-3, 3) };
			trail_points.push_front(_pos);
		}
	}

	void Draw()
	{
		if (trail_points.size() < 2) return;  // Ensure there are at least 2 points to draw a line

		for (size_t i = 0; i < trail_points.size() - 1; ++i)
		{
			DrawLineEx(trail_points[i], trail_points[i + 1], 5.0f, RED);
		}
	}


	bool refresh_time(double interval)
	{
		double ctime = GetTime();
		if (ctime - last_spawn_time >= interval)
		{
			return true;
			last_spawn_time = ctime;
		}
		return false;
	}
};

class Player
{
public:
	float spaceship_rotation = 0.0f;
	Vector2 position = { 370, 150 };
	Rectangle spaceship_rect = { position.x, position.y, 48, 48 };
	double player_speed = 4;
	deque<Bullet> bullets = {};
	Vector2 direction = { double(1.0), double(0.0) };
	PlayerTrail trail = PlayerTrail();
	double last_shoot_time = 0.0;

	Player()
	{
		Image spaceship_img = LoadImage("");
		spaceship_texture = LoadTextureFromImage(spaceship_img);
		UnloadImage(spaceship_img);
		spaceship_texture.height = 48;
		spaceship_texture.width = 48;
	}

	void Input()
	{
		if (IsKeyDown(KEY_A))
		{
			spaceship_rotation -=2;
			set_direction();
		} 
		else if (IsKeyDown(KEY_D))
		{
			spaceship_rotation +=2;
			set_direction();
		}
		if (IsKeyDown(KEY_W))
		{
			position = Vector2Add(position, direction);
			spaceship_rect = { position.x, position.y, 48, 48 };
		}

		if (IsKeyDown(KEY_SPACE))
		{
			if (can_shoot(0.1))
			{
				Bullet blt = Bullet(position, direction);
				bullets.push_front(blt);
				blt.spawned();
			}
		}

		for (Bullet& blt : bullets) {
			blt.Move();
		}

		trail.Update(position);
	}

	void set_direction()
	{
		float radian = spaceship_rotation * (3.1416 / 180);
		direction = { cos(radian), sin(radian) };
		direction.x *= player_speed; direction.y *= player_speed;
	}

	bool can_shoot(double interval)
	{
		double ctime = GetTime();
		if (ctime - last_shoot_time >= interval)
		{
			last_shoot_time = ctime;
			return true;
		}
		return false;
	}

	void Draw()
	{
		trail.Draw();
		DrawTexturePro(spaceship_texture, Rectangle{ 0, 0, 48, 48 }, spaceship_rect, { 24, 24 }, spaceship_rotation, WHITE);
		for (Bullet& blt : bullets) {
			blt.Draw();
		}
	}

	void Unload()
	{
		UnloadTexture(spaceship_texture);
	}
};

class Cam
{
public:
	Camera2D camera = Camera2D();

	Cam()
	{
		camera.zoom = 1.0f;
		camera.offset = { 0, 0 };
		camera.rotation = 0.0f;
	}

	void Update(Vector2 pos)
	{
		camera.target = pos;
		camera.offset = { SCREEN_WIDTH / 2, SCREEN_HEIGH / 2 };
	}
};

class Background
{
	Texture2D texture;
	Rectangle rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGH };
	Vector2 origin = { 0, 0 };
public:

	Background()
	{
		Image background_image = LoadImage("C:\\Users\\moham\\source\\repos\\RaylibSnake\\RaylibSnake\\assets\\SpaceBackground.png");
		texture = LoadTextureFromImage(background_image);
		SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
		UnloadImage(background_image);
	}

	void Draw()
	{
		DrawTexturePro(texture, rect, Rectangle{ origin.x-SCREEN_WIDTH/2, origin.y-SCREEN_HEIGH/2, SCREEN_WIDTH, SCREEN_HEIGH }, {0, 0}, 0.0f, WHITE);
	}

	void UpdatePosition(Vector2 position)
	{
		rect.x = position.x/10;
		rect.y = position.y/10;
		origin = position;
	}

	double position = 0.0;
};

class Rock
{
	Vector2 position = { 0, 0 };
	Rectangle collision_shape = { position.x, position.y, 20, 20 };
	int rotation_angle = 0;
	int rotation = 0;
public:
	bool active = true;
	Rock(Vector2 init_pos)
	{
		position = init_pos;
		rotation = GetRandomValue(0, 360);
		rotation_angle = GetRandomValue(-10, 10);
	}

	void Collisions(Rectangle rect)
	{
		if (CheckCollisionRecs(collision_shape, rect))
		{
			active = false;
			cout << "HIT!" << endl;
		}
	}

	void Update()
	{
		rotation += rotation_angle;
	}

	void Draw()
	{
		DrawRectangleV(position, { 20, 20 }, BLUE);
	}
};

class GameElements
{
	Player player;
	Vector2 last_player_position = { 0, 0 };
	deque<Rock> rocks = {};
public:
	GameElements(Player& _player)
	{
		player = _player;
	}

	void Update(Vector2 player_position, Player plr)
	{
		for (Rock& _rock: rocks)
		{
			for (Bullet& blt : plr.bullets)
			{
				_rock.Collisions(blt.shape);
			}
			if (_rock.active)
			{
				_rock.Update();
			}
		}

		if (GetPlayerMilestone(player_position) > 1)
		{
			last_player_position = player_position;
			Vector2 _sp = GetValidPosition(player_position);
			Spawn(_sp);
		}
	}

	void Draw()
	{
		for (Rock& _rock : rocks)
		{
			_rock.Draw();
		}
	}

	void Spawn(Vector2 spawn_pos)
	{
		Rock newRock = Rock(spawn_pos);
		rocks.push_front(newRock);
	}

	float GetPlayerMilestone(Vector2 pos)
	{
		float x = abs(pos.x - last_player_position.x);
		float y = abs(pos.y - last_player_position.y);
		float total_distance = x + y;
		
		return total_distance;
	}

	Vector2 GetValidPosition(Vector2 p_pos)
	{
		int vo = GetRandomValue(0, 1);
		int ho = GetRandomValue(0, 1);
		Vector2 random_vector = {0, 0};
		random_vector.y = (p_pos.y - SCREEN_HEIGH / 2 - 50);
		random_vector.x = GetRandomValue(p_pos.x - SCREEN_WIDTH, p_pos.x + SCREEN_WIDTH);
		return random_vector;
	}
};

int main()
{
	int target_fps = 60;
	SetTargetFPS(target_fps);

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGH, "Raylib Game");

	Player player = Player();
	Background background = Background();
	Cam camera = Cam();
	GameElements game = GameElements(player);

	while (!WindowShouldClose())
	{
		player.Input();
		background.UpdatePosition(player.position);
		camera.Update(player.position);
		game.Update(player.position, player);


		BeginDrawing();

		BeginMode2D(camera.camera);
		background.Draw();
		player.Draw();
		game.Draw();

		ClearBackground(WHITE);
		EndMode2D();

		EndDrawing();

	}

	player.Unload();

	return 0;
}