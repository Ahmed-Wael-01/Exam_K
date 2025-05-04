#include "ray.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* image = NULL;
SDL_Texture* textures[2];

double px, py, pa, dirx, diry;

int world[8][8] =
{
	{1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 2, 2, 0, 0, 0, 1},
	{1, 0, 0, 2, 0, 0, 1, 1},
	{1, 0, 0, 0, 0, 0, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 1, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1},
};

int init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Unable to initialize SDL: %s\n",
				SDL_GetError());
		return (1);
	}
	window = SDL_CreateWindow("SDL2",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 624, 512, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		fprintf(stderr, "SDL_CreateWindow Error: %s\n",
				SDL_GetError());
		SDL_Quit();
		return (1);
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL)
	{
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n",
				SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return (1);
	}
	return (0);
}

void init_textures(void)
{
	image = SDL_LoadBMP("brick.bmp");
        textures[0] = SDL_CreateTextureFromSurface(renderer, image);
	image = SDL_LoadBMP("arrow.bmp");
        textures[1] = SDL_CreateTextureFromSurface(renderer, image);

}

void draw_grid(void)
{
	SDL_Rect rect;
	int i, j;

	rect.x = 0;
	rect.y = 0;
	rect.w = 64;
	rect.h = 64;
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (world[i][j] == 1)
				SDL_SetRenderDrawColor(renderer,
						255, 255, 255, 0);
			else
				SDL_SetRenderDrawColor(renderer,
                                                0, 0, 0, 0);
			SDL_RenderFillRect(renderer, &rect);
			rect.x += 64;
		}
		rect.y += 64;
		rect.x = 0;
	}
	SDL_RenderDrawLine(renderer, i * 64, 0, i * 64, j * 64);
	SDL_RenderDrawLine(renderer, 0, j * 64, i * 64, j * 64);
}

double calc_dist(double startX, double endX, double startY, double endY, double angle)
{
	double distance;

	//distance = abs(startX - endX) / cos(angle);
	
	distance = sqrt((startX - endX) * (startX - endX) + (startY - endY) * (startY - endY));

	return (distance);
}

double cast_ray(int x, int y, double a, int i)
{
	SDL_Rect srcrect, dstrect;
	double pointAX, pointAY;
        double pointBX, pointBY;
        double ya, xa, slice;
        double vWallX, vWallY;
        double hWallX, hWallY;
        double vDist, hDist;

	if (a > 0 && a < PI)
        {
                pointAY = (y/64) * 64 - 0.001;
                ya = -64;
                pointAX = x + (y - pointAY) / tan(a);
                xa = 64 / tan(a);
        }
        else
        {
                pointAY = (y/64) * 64 + 64;
                ya = 64;
                pointAX = x + (y - pointAY) / tan(a);
                xa = -64 / tan(a);
        }

        hWallX = pointAX;
        hWallY = pointAY;

        while (tan(a) && hWallX / 64 < 8 && hWallY / 64 < 8 && hWallX > 0 && hWallY > 0)
        {
                if (world[(int)hWallY / 64][(int)hWallX / 64])
                        break;
                hWallX = hWallX + xa;
                hWallY = hWallY + ya;
        }

	if (a > (PI * 3) / 2 || a < PI / 2)
        {
                pointBX = (x/64) * 64 + 64;
                xa = 64;
                pointBY = y + (x - pointBX) * tan(a);
                ya = -64 * tan(a);
        }
        else
        {
                pointBX = (x/64) * 64 - 0.001;
                xa = -64;
                pointBY = y + (x - pointBX) * tan(a);
                ya = 64 * tan(a);
        }

        vWallX = pointBX;
        vWallY = pointBY;

        while (tan(a) && vWallX / 64 < 8 && vWallY / 64 < 8 && vWallX > 0 && vWallY > 0)
        {
                if (world[(int)vWallY / 64][(int)vWallX / 64])
                        break;
                vWallX = vWallX + xa;
                vWallY = vWallY + ya;
        }

        vDist = calc_dist(x, vWallX, y, vWallY, a);
        hDist = calc_dist(x, hWallX, y, hWallY, a);

	SDL_SetRenderDrawColor(renderer, 51, 51, 51, 0);

        if (vDist < hDist)
	{
		vDist = vDist * cos(pa - a);
                slice = (SIZE / vDist) * 540;
                dstrect.x = i + RENDER_START;
                dstrect.y = (SCREEN_HEIGHT - slice) / 2;
                dstrect.w = 1;
                dstrect.h = slice;
		if (a < PI / 2 || a > PI * 3 / 2)
			srcrect.x = (int)vWallY % 64;
		else
			srcrect.x = 63 - ((int)vWallY % 64);
		srcrect.y = 0;
		srcrect.w = 1;
		srcrect.h = 64;
		SDL_RenderCopy(
				renderer,
				textures[world[(int)vWallY / 64][(int)vWallX / 64] - 1],
				&srcrect, &dstrect);
		SDL_RenderDrawLine(renderer, RENDER_START + i,
					(SCREEN_HEIGHT - slice) / 2 + slice - 1,
					RENDER_START + i,
					SCREEN_HEIGHT);
		return (vDist);
	}
	hDist = hDist * cos(pa - a);
	slice = (SIZE / hDist) * 540;
                dstrect.x = i + RENDER_START;
                dstrect.y = (SCREEN_HEIGHT - slice) / 2;
                dstrect.w = 1;
                dstrect.h = slice;
		if (a <= PI && a >= 0)
                	srcrect.x = (int)hWallX % 64;
		else
			srcrect.x = 63 - ((int)hWallX % 64);
                srcrect.y = 0;
                srcrect.w = 1;
                srcrect.h = 64;
                SDL_RenderCopy(
				renderer,
				textures[world[(int)hWallY / 64][(int)hWallX / 64] - 1],
				&srcrect, &dstrect);
	SDL_RenderDrawLine(renderer, RENDER_START + i, (SCREEN_HEIGHT - slice) / 2 + slice - 1, RENDER_START + i, SCREEN_HEIGHT);
	return (hDist);
}

void draw_player(int x, int y, double a)
{
	SDL_Rect rect;

	rect.x = x;
        rect.y = y;
        rect.w = 7;
        rect.h = 7;

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
	SDL_RenderDrawRect(renderer, &rect);
	SDL_RenderDrawLine(renderer,
			rect.x + (rect.w / 2), rect.y + (rect.h / 2),
			rect.x + (rect.w / 2) + (cos(a) * 20),
			rect.y + (rect.h / 2) - (sin(a) * 20));
	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
}

void draw_fov(void)
{
	int i;
	double dist;
	double slice;
	double curr;
	double view = PI / 3;
	double inc = view / SCREEN_WIDTH;

	curr = pa + PI / 6;
	for (i = 0; i < SCREEN_WIDTH; i++)
	{
		if (curr > PI * 2)
			curr -= PI * 2;
		dist = cast_ray(px, py, curr, i);
		//dist = dist * cos(pa - curr);
		//slice = (SIZE / dist) * 277;
		//SDL_RenderDrawLine(renderer, RENDER_START + i, (SCREEN_HEIGHT - slice) / 2, RENDER_START + i, (SCREEN_HEIGHT - slice) / 2 + slice);
		
		curr -= inc;
	}
}

void draw_bar(void)
{
	SDL_Rect rect = {0, 400, 624, 112};

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderFillRect(renderer, &rect);
}

int main()
{
	px = 100;
	py = 100;
	pa = 0;
	dirx = cos(pa) * 5;
	diry = sin(pa) * 5;
	
	if (init_sdl() != 0)
		return (1);

	init_textures();
	while (1)
	{
		SDL_SetRenderDrawColor(renderer, 127, 127, 127, 0);
		SDL_RenderClear(renderer);
		SDL_Event event;
		SDL_KeyboardEvent key;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					return (1);
				case SDL_KEYDOWN:
					key = event.key;
					if (key.keysym.scancode == 0x29)
						return (1);
					else if (key.keysym.scancode == 0x4f)
					{
						pa -= 0.05;
						if (pa <= 0)
							pa += 2 * PI;
						dirx = cos(pa) * 5;
						diry = sin(pa) * 5;
					}
					else if (key.keysym.scancode == 0x50)
					{
						pa += 0.05;
                                                if (pa > 2 * PI)
                                                        pa -= 2 * PI;
                                                dirx = cos(pa) * 5;
                                                diry = sin(pa) * 5;
					}
					else if (key.keysym.scancode == 0x51)
					{
						px -= dirx;
						py += diry;
					}
					else if (key.keysym.scancode == 0x52)
					{
						px += dirx;
						py -= diry;
					}
			}
		}
		draw_fov();
		draw_bar();
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
