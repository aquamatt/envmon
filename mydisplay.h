#define LOWER_FIVE 0b00111110
#define MYDISPLAY_SHIFT 1  // if display doesn't start on PIN0, shift this amt

/***********************
 * Display to our 5-bit output
 */
void mydisplay_init(void);
void display(uint8_t v);
void displayf(float v);
void flash(uint8_t v, uint8_t count);
