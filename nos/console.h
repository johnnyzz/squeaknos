
#ifndef _SQ_CONSOLE_H_
#define _SQ_CONSOLE_H_


#define CONSOLE_TEXT_BUFFER_SIZE 1000*1000

typedef struct
{
	int positionX;
	int positionY;
	
	int width;
	int glyph_width;
	int glyph_height;
	
} TextPen;

typedef struct
{
	int glyph_width;
	int glyph_height;
	
	int width;
	int height;
	
	char text[CONSOLE_TEXT_BUFFER_SIZE];
	int text_size;
	
	int debugging_now;

} Console;

#endif  // _SQ_CONSOLE_H_

