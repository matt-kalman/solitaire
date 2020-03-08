#ifndef CARDS_H
#define CARDS_H

#include <vector>
#include <SDL.h>
#include "images.h"

using namespace std;

SDL_Renderer* renderer;
SDL_Surface* image;
SDL_Texture* texture;
int cw = 107, ch = 144;

SDL_Surface* surface(int x, int w, int h, int a) {
	Uint32 rmask, gmask, bmask, amask;
	if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		int shift = (a == 3) ? 8 : 0;
		rmask = 0xff000000 >> shift;
		gmask = 0x00ff0000 >> shift;
		bmask = 0x0000ff00 >> shift;
		amask = 0x000000ff >> shift;
	} else {
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = (a == 3) ? 0 : 0xff000000;
	}

	return SDL_CreateRGBSurfaceFrom((void*)img[x], w, h, 8 * a, a * w, rmask, gmask, bmask, amask);
}

struct Card {
	int rank, suit;
	bool faceUp = false, color = false;
	SDL_Rect rect = {0,0,cw,ch};

	Card(int r, int s) {
		rank = r;
		suit = s;

		if (suit == 0 || suit == 3) {
			color = true;
		}
	}

	Card(const Card &other) {
		memcpy(this, &other, sizeof(Card));
	}

	void flip() {
		faceUp = !faceUp;
	}

	void show() {
		if (!faceUp) {
			image = surface(52, 71, 96, 3);
		} else {
			image = surface(suit*13 + rank, 71, 96, 3);
		}

		texture = SDL_CreateTextureFromSurface(renderer, image);
		SDL_FreeSurface(image);
		image = NULL;

		SDL_RenderCopy(renderer, texture, 0, &rect);
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
};

struct Pile {
	vector<Card*> cards;
	SDL_Rect rect = {0,0,cw,ch};

	void add(Card* card) {
		cards.push_back(card);
	}

	void give(Card* card, Pile &other) {
		cards.erase(remove(begin(cards), end(cards), card), end(cards));
		other.add(card);
	}
};

#endif