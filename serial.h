/******
 * Build notes
 * - F_CPU must be defined in the Makefile
 * - BAUD must be defined in the Makefile
 */
void uart_init(void);
void uart_putchar(char c, FILE *stream);
void redirect_stdout(void);
