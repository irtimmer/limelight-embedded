#include "text.h"

int text_ascent(TTF_Font *font) {
  return TTF_FontAscent(font);
}

void measure_text(TTF_Font *font, const char *text, int *width, int *height) {
  // Measure the text size
  TTF_SizeUTF8(font, text, width, height);
}

void draw_text(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center, int truncate_width) {
  Rect clip = {
    .x = 0,
    .y = 0,
    .width = gui.width,
    .height = gui.height
  };

  draw_clipped_text(font, text, x, y, &clip, color, align_center, truncate_width);
}

void draw_clipped_text(TTF_Font *font, const char *text, int x, int y, Rect *clip, uint32_t color, bool align_center, int truncate_width) {
  // Measure the text size
  int textWidth, textHeight;
  TTF_SizeUTF8(font, text, &textWidth, &textHeight);

  // Render the text into a surface
  SDL_Color fg;
  fg.r = color & 0xFF;
  fg.g = (color >> 8) & 0xFF;
  fg.b = (color >> 16) & 0xFF;
  fg.a = (color >> 24) & 0xFF;

  SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, fg);

  // Handle truncation
  if (truncate_width >= 0 && truncate_width < textWidth) {
    textWidth = truncate_width;

    if (SDL_MUSTLOCK(textSurface)) {
      SDL_LockSurface(textSurface);
    }

    fprintf(stderr, "[GUI, text] Surface format: %s\n", SDL_GetPixelFormatName(textSurface->format->format));

    uint32_t *pixels = textSurface->pixels;
    int pitch = textSurface->pitch;

    for (size_t y = 0; y < textHeight; y++) {
      for (size_t x = TRUNCATE_FADE_WIDTH; x > 0; x--) {
        size_t pos = y*(pitch/sizeof(uint32_t)) + (textWidth - x);
        uint32_t origColor = pixels[pos];
        uint32_t blankColor = origColor & 0x00ffffff;
        pixels[pos] = interpolate(blankColor, origColor, (float)x / TRUNCATE_FADE_WIDTH);
      }
    }

    if (SDL_MUSTLOCK(textSurface)) {
      SDL_UnlockSurface(textSurface);
    }
  }

  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(gui.renderer, textSurface);
  SDL_FreeSurface(textSurface);

  // Render the surface at a specific location
  SDL_Rect destinationRect;

  if (align_center) {
      destinationRect.x = x - textWidth/2;
      destinationRect.y = y - textHeight/2;
      destinationRect.w = textWidth;
      destinationRect.h = textHeight;
  }
  else {
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = textWidth;
    destinationRect.h = textHeight;
  }

  draw_clipped_texture(textTexture, destinationRect.x, destinationRect.y, destinationRect.w, destinationRect.h, clip);
  SDL_DestroyTexture(textTexture);
}
