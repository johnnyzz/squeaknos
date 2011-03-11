
#include "fonttex.h"
#include "string.h"
#include "stdio.h"
#include "console.h"

/**
 * Activate this if you wan't to directly draw to the framebuffer, like
 * when the VM is unexpectedly crashing and you want to know why.
 * Most of the time using
 * Transcript show: Computer current primPullDebugString is better.
**/

Console console;


char* first_char_of_line_ending_at(char *actualChar, char *stringStart)
{
	//printf("string starts at...%p ends at: %p\n", stringStart, actualChar);
	
	actualChar--; // first char is end of string or end of line.
	
	while (actualChar >= stringStart && *actualChar != '\n')
	{
		actualChar--;
	}
	
	return ++actualChar;
}



void text_pen_initialize( TextPen *pen, int glyph_width, int glyph_height, int rect_width)
{
	pen->positionX    = 0;
	pen->positionY    = glyph_height;
	
	pen->width        = rect_width;
	
	pen->glyph_width  = glyph_width;
	pen->glyph_height = glyph_height;
}

void text_pen_carriage_return(TextPen *text_pen)
{
	text_pen->positionX = 0;
	text_pen->positionY += text_pen->glyph_height;
}

void text_pen_advance_one_char(TextPen *text_pen)
{
	text_pen->positionX += (text_pen->glyph_width + 1);
}

void text_pen_advance_n_chars(TextPen *text_pen, int n)
{
	text_pen->positionX = (text_pen->glyph_width + 1) * n;
}

void text_pen_correct_margin_if_necessary(TextPen *text_pen)
{
	if (text_pen->positionX + text_pen->glyph_width > text_pen->width)
		text_pen_carriage_return(text_pen);
}

void text_pen_advance_char(TextPen *text_pen, unsigned char nextChar)
{
	switch (nextChar)
	{
		case '\n': text_pen_carriage_return(text_pen);		return;
		case '\t': text_pen_advance_n_chars(text_pen, 4); 	break;
		default:   text_pen_advance_one_char(text_pen);		break;
	}
	
	text_pen_correct_margin_if_necessary(text_pen);
}

int text_pen_collected_height(TextPen *pen)
{
	return pen->positionY - (pen->positionX == 0 ? pen->glyph_height : 0);
}


void font_draw_char(TextPen *pen, char character)
{
	if (character < 32)
		return;
		
	bitblt_32bit_to_fb(rasters[character-32], FONT_GLYPH_WIDTH, FONT_GLYPH_HEIGHT, pen->positionX, pen->positionY);
	fill_rectangle(1, FONT_GLYPH_HEIGHT, pen->positionX+FONT_GLYPH_WIDTH, pen->positionY, 0x00000000);
}


void console_initialize(Console *console, int width, int height)
{
	console->glyph_width = 8;
	console->glyph_height = 16;
	
	console->width = width;
	console->height = height;
	
	console->text[0] = 0;
	console->text_size = 0;
	
	console->debugging_now = 1;
}

void console_clear(Console *console)
{
	console->text[0] = 0;
	console->text_size = 0;
}

void initialize_std_console()
{
	console_initialize(&console, 1000, 600);
}

void set_std_console_debugging(int debugging)
{
	console.debugging_now = debugging;
}

void console_fill_remaining_with_background(Console *console, int left, int top)
{
	fill_rectangle(console->width - left, FONT_GLYPH_HEIGHT, left, top, 0x00000000);
	fill_rectangle(console->width, console->glyph_height - FONT_GLYPH_HEIGHT, 0 , top + console->glyph_height - FONT_GLYPH_HEIGHT, 0);
}

void console_draw_string(Console *console, char *string)
{
	if (!console->debugging_now)
		return;
		
	TextPen pen;
	text_pen_initialize(&pen, console->glyph_width, console->glyph_height, console->width);
	
	while (*string != 0)
	{
		font_draw_char(&pen, *string);
		if (*string == '\n')
			console_fill_remaining_with_background(console, pen.positionX, pen.positionY);
		text_pen_advance_char(&pen, *string);
		string++;
	}

	
}


int console_calc_height_of_line(Console *console, char *lineStart)
{
	TextPen pen;
	text_pen_initialize(&pen, console->glyph_width, console->glyph_height, console->width);
	
	//printf("bla!\n");
		
	while (*lineStart != 0 && *lineStart != '\n')
	{
		text_pen_advance_char(&pen, *lineStart);
		lineStart++;
	}
	//printf("height...%d\n", text_pen_collected_height(&pen));
	return text_pen_collected_height(&pen);
}

/*
void console_first_char_shown_with(Console *console, char *lineStart, int remainingHeight)
{
	int fitting_lines = fitting_lines_in(console, remainingHeight);
	int characters_in_line = fitting_characters_in_line(console);
	char *first_shown = lineStart;
	char
	while (actualChar != '\n' && actualChar != 0 )
	{
		actualChar = next_console_line_from(actualChar);
		remainingHeight -= console->charHeight;
		if (remainingHeight < 0)
			firstShown = actualChar;
	} 
}
*/
/**
 * Do it by iterating lines backwards until reaching begining of string or it's height
 * surpases display height.
**/
char* console_calc_fitting_text(Console *console)
{
	int textHeight = 0;
	int previousLineHeight;
	
	char *actualChar = console->text + console->text_size;
	char *actualLineStart, *previousLineStart;
	
	while (actualChar > console->text)
	{
		previousLineStart  = first_char_of_line_ending_at(actualChar, console->text);
		
		//printf("previous...%p\n", previousLineStart);
		
		previousLineHeight = console_calc_height_of_line(console, previousLineStart);
		//printf("console height...%d\n", console->height);
		
		if (textHeight + previousLineHeight > console->height)
			break;

		textHeight += previousLineHeight;

		//printf("got here\n");
		actualLineStart = previousLineStart;
		actualChar = actualLineStart - 1;
	}
	
	return actualLineStart;
	//return console_first_char_shown_with(console, actualLineStart, displayHeight - textHeight);
	
}

void console_draw(Console *console)
{
	char *text_start = console_calc_fitting_text(console);
	//printf("calculated... %p start: %p, string:\n%s\n", text_start, console->text, text_start);
	
	console_draw_string(console, text_start);
}

void console_push_string(Console *console, char string[])
{
	//printf("viewing len...\n");
	int len = strlen(string);
	//printf("len is %d...\n", len);
	if (console->text_size + len > CONSOLE_TEXT_BUFFER_SIZE)
		return;
	
	//printf("start %p...\n", console->text + console->text_size);
	strcpy(console->text + console->text_size, string);
	//printf("copied...\n");
	
	console->text_size += len;
	
	console->text[console->text_size] = 0; //just in case strcpy didn't do it.
}

void std_console_put_string(char string[])
{
	console_push_string(&console, string);
	console_draw(&console);
	
}

void std_console_put_char(char c)
{
	char str[2] = { c, 0 };
	console_push_string(&console, str);
	console_draw(&console);
	
}
