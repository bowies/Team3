// Free RTOS Headers
// See API reference Document in http://www.freertos.org/

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

// NDS / Sevencore Board Headers
#include <nds.h>
#include <sevencore_io.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define COLOR_RED       RGB(31,  0,  0) /* Bright Red  	*/
#define COLOR_WHITE     RGB(31, 31, 31) /* Bright White */
#define COLOR_YELLOW     RGB(31, 31, 0) /* Bright White */
#define COLOR_BLACK     RGB( 0,  0,  0)	/* Black : Zero	*/
#define COLOR_GRAY     RGB(16, 16, 16)
#define COLOR_GREEN     RGB( 0, 31,  0) /* Bright White */
#define COLOR_BLUE      RGB( 0,  0, 31) /* Bright White */
#define COLOR_PURPLE     RGB(16, 0, 16) /* Bright White */
#define COLOR_SELECT     RGB(31, 0, 31) /* Bright White */

#define BOX_WIDTH		8
#define BOX_HEIGHT	8
#define BOX_Y_POS	13
#define BOX_X_MAX	(SCREEN_WIDTH / BOX_WIDTH)

#define BIG_BOX_WIDTH		32
#define BIG_BOX_HEIGHT		32
#define WALL_Y_POS			3
#define BIG_BOX_X_MAX (SCREEN_WIDTH / BIG_BOX_WIDTH)

#define LEFT		1
#define RIGHT		2

#define BIG_RATE	(BIG_BOX_WIDTH / BOX_WIDTH)
#define SEM(x)	(((x) / BIG_RATE) - 1)

// �ʿ��� global ����
xSemaphoreHandle xSemaphore[BIG_BOX_X_MAX - 1];
u8 limit_x;
static volatile u8 virtual_puzzle;

void
draw_my_box(int pos_x, int pos_y, u16 color)
{
	int i, j;
	u32 *basePoint, pixel;

	pixel = (color << 16) + color;
	for (i = 0; i < BOX_HEIGHT; i++) {
		basePoint = (u32 *)BG_GFX +
				((((pos_y * BOX_HEIGHT) + i) * SCREEN_WIDTH) + pos_x * BOX_WIDTH) / 2;
		for (j = 0; j < (BOX_WIDTH / 2); j++)
			*basePoint++ = pixel;
	}
}

void
draw_my_wall(int pos_x, int pos_y, u16 color)
{
	// draw big white box
	int i, j;
	u32 *basePoint, pixel, blank_pixel;

	pixel = (color << 16) + color;
	blank_pixel = (COLOR_GRAY << 16) + COLOR_GRAY;
	for (i = 0; i < BIG_BOX_HEIGHT; i++) {

		basePoint = (u32 *) BG_GFX_SUB+
				((((pos_y * BIG_BOX_HEIGHT) + i) * SCREEN_WIDTH) +
						pos_x * BIG_BOX_WIDTH) / 2;

		for (j = 0; j < (BIG_BOX_WIDTH / 2); j++) {
			if ((i < 2) || (i > (BIG_BOX_HEIGHT-3))
					|| (j == 0) || (j == (BIG_BOX_HEIGHT/2-1))) {
				*basePoint++ = blank_pixel;
				continue;
			}
			*basePoint++ = pixel;
		}
	}
}
void
select_my_wall(int pos_x, int pos_y, u16 color)
{
	// draw big white box
	int i, j;
	u32 *basePoint, pixel;

	pixel = (color << 16) + color;

	for (i = 0; i < BIG_BOX_HEIGHT; i++) {

		basePoint = (u32 *) BG_GFX_SUB+
				((((pos_y * BIG_BOX_HEIGHT) + i) * SCREEN_WIDTH) +
						pos_x * BIG_BOX_WIDTH) / 2;

		for (j = 0; j < (BIG_BOX_WIDTH / 2); j++) {
			if ((i < 2) || (i > (BIG_BOX_HEIGHT-3))
					|| (j == 0) || (j == (BIG_BOX_HEIGHT/2-1))) {
				*basePoint++ = pixel;
				continue;
			}
			*basePoint++;
		}
	}
}

u16 set_color(int n) {
	switch (n) {
	case 0:
		return COLOR_YELLOW;
	case 1:
		return COLOR_RED;
	case 2:
		return COLOR_GREEN;
	case 3:
		return COLOR_BLUE;
	case 4:
		return COLOR_PURPLE;
	}
}

void write_puzzle(u8 value)
{
	virtual_puzzle = value & 0x3F;
}

static
u8
read_puzzle(void)
{
	touchPosition touch;

	if (!(virtual_puzzle & 0x3F))
		return 0;
	touchRead(&touch);
	touch.py = (touch.py / BIG_BOX_HEIGHT);
	touch.px = (touch.px / BIG_BOX_WIDTH) - 2;
	if ((touch.py < 0) || (touch.py > 5) || (touch.px < 0) || (touch.px > 5))
		return 0;
	if (!(virtual_puzzle & (0x20 >> touch.py))) return 0;
	return (0x20 >> touch.px);
}


extern xTaskHandle BallTask;

void
Exp_8_Homework_A(void)
{
	u8 key;
	u8 pos_x = 0;
	u8 prev_x;

	while (1) {
		if (kbhit()) {
			if ((key = getkey()) < 8) {
				prev_x = pos_x;
				pos_x = key;

				draw_my_wall(prev_x, WALL_Y_POS, COLOR_BLACK);
				draw_my_wall(pos_x, WALL_Y_POS, COLOR_WHITE);
			}
		}
		if (NDS_SWITCH() & KEY_START) {
			draw_my_wall(pos_x, WALL_Y_POS, COLOR_BLACK);
			break;
		}
		vTaskDelay(10);
	}
	while (NDS_SWITCH() & KEY_START)
		vTaskDelay(10);		// Wait while START KEY is being pressed
}

void
Exp_8_Homework_B(void)
{
	int i, j;
	u8 old_key = 0, key;
	const int SP = 2;
	const int N_PUZZLE = 6;
	const int N_BLOCK = 5;

	srand((int)time(NULL));

	for (i = SP; i < SP + N_PUZZLE; i++)
		for (j = 0; j < N_PUZZLE; j++)
			draw_my_wall(i, j, set_color(rand() % N_BLOCK));

	while (1) {
		key = getkey();

		if (key <= 36) {
			printf("%d ", key);
			//select_my_wall(old_key, old_key, COLOR_GRAY);
			//select_my_wall(key, key, COLOR_SELECT);

			key--;
			select_my_wall(old_key % N_PUZZLE + SP, old_key / N_PUZZLE, COLOR_GRAY);
			select_my_wall(key % N_PUZZLE + SP, key / N_PUZZLE, COLOR_SELECT);
			old_key = key;

		}

	}
}

portTASK_FUNCTION(Ball_Task, pvParameters)
{
	u8 direction = LEFT;
	u8 pos_x = BOX_X_MAX - 1;
	u8 prev_x;
	//u8 limit_x = (BIG_BOX_X_MAX - 1) * BIG_RATE;
	int tick_time;
	int i;
	portTickType xLastWakeTime = xTaskGetTickCount();

	while (1) {
		prev_x = pos_x;

		// semaphore 진입 시점 마다 limit_x check
		if (direction == LEFT) {
			if ((pos_x != 0) && (pos_x % 4 == 0)) {
				if (xSemaphoreTake( xSemaphore[SEM(pos_x)], 0 ) == pdTRUE)
					//limit_x -= BIG_RATE;
					;
				else
					direction = RIGHT;
			}
		}
		else {
			if (pos_x % 4 == 0) {
				xSemaphoreGive( xSemaphore[SEM(pos_x)] );
				//limit_x += BIG_RATE;
			}
		}

		tick_time = 1000 / (BOX_X_MAX - limit_x);

		//tick_time = 1000 / BOX_X_MAX;
		// 방향 전환 및 위치 이동
		if (direction == LEFT) {
			if (pos_x > limit_x) pos_x--;
			else { pos_x++; direction = RIGHT; }
		}
		else {
			if (pos_x < (BOX_X_MAX - 1)) pos_x++;
			else { pos_x--; direction = LEFT; }
		}

		draw_my_box(prev_x, BOX_Y_POS, COLOR_BLACK);
		draw_my_box(pos_x, BOX_Y_POS, COLOR_RED);

		vTaskDelayUntil(&xLastWakeTime, MSEC2TICK(tick_time));
	}
}

// Key Matrix Scanning Task

void
key_init(void)
{
	int i;
	u8 key;

	for (i = 0; i < MAX_KEY_LOG; i++)
		xQueueReceive(KeyQueue, &key, 0);
}

int
kbhit(void)
{
	u8 key;
	int ret = xQueuePeek(KeyQueue, &key, 0);
	return (ret == pdPASS);
}
u8
getkey(void)
{
	u8 key;
	xQueueReceive(KeyQueue, &key, portMAX_DELAY);
	return key;
}

portTASK_FUNCTION(Key_Task, pvParameters)
{
	// Variables
	u8 key, scan = 0;
	u8 key_pressed = FALSE;
	u8 pre_line;

	while (1) {

		//for (i = 0; i < 100000; i++);
		// Fill Here

		if (!key_pressed) {

			write_puzzle(0x20 >> scan);
			key = scan * 6;

			pre_line = read_puzzle();
			switch (read_puzzle()) {
			case 32 : key += 1; break;
			case 16 : key += 2; break;
			case 8 : key += 3; break;
			case 4 : key += 4; break;
			case 2 : key += 5; break;
			case 1 : key += 6; break;
			default : key = 255; break;
			}

			scan++;
			if (scan == 6)
				scan = 0;

			if (key <= 36) {
				key_pressed = TRUE;
				xQueueSend(KeyQueue, &key, 0);
			}
			/*
			writeb_virtual_io(KEY_MATRIX, 0x80 >> scan);
			key = scan * 4;

			pre_line = readb_virtual_io(KEY_MATRIX);
			switch (readb_virtual_io(KEY_MATRIX)) {
			case 8 : key += 1; break;
			case 4 : key += 2; break;
			case 2 : key += 3; break;
			case 1 : key += 4; if (key == 16) key = 0; break;
			default : key = 255; break;
			}

			scan++;
			if (scan == 4)
				scan = 0;

			if (key < 16) {
				key_pressed = TRUE;
				xQueueSend(KeyQueue, &key, 0);
			}
			 */
		}

		/*
		if (key_pressed
				&& ((readb_virtual_io(KEY_MATRIX) == 0)
						|| (readb_virtual_io(KEY_MATRIX) != pre_line)))
			key_pressed = FALSE;
		 */

		if (key_pressed
				&& ((read_puzzle() == 0)
						|| (read_puzzle() != pre_line)))
			key_pressed = FALSE;

		vTaskDelay(MSEC2TICK(25));
	}
}
