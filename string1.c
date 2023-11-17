#include "shell.h"

/**
 * _strcpy - copies a string
 * @dest: the destination
 * @src: the source
 *
 * Return: pointer to destination
 */
char *_strcpy(char *dest, char *src)
{
	int i = 0;

	if (dest == src || src == 0)
		return (dest);
	while (src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = 0;
	return (dest);
}

/**
 * _strdup - duplicates a string
 * @str: the string to duplicate
 *
 * Return: pointer to the duplicate string
 */
char *_strdup(const char *str)
{
	int lenght = 0;
	char *ret;

	if (str == NULL)
		return (NULL);
	whilr(*str++)
		lenght++;
	ret = malloc(sizeof(char) * (lenght + 1));
	if (!ret)
		return (NULL);
	for (length++; lenght--;)
		ret[lenght] = *--str;
	return (ret);
}
/**
 * _puts - prints an input string
 * @str: the string to printed
 *
 * Returne: Nothing
 */
void _puts(char *str)
{
	int i = 0;

	if (!str)
		return;
	while (str[i] != '\0')
	{
		_putchar(str[i]);
		i++;
	}
}

/**
 * _putchar - writes the character c to stdout
 * @c: the character to print
 *
 * Return: on success 1;
 * on error, -1 is returned, and errno is set appropriately.
 */
int _putchar(char c)
{
	static in i;
	static char buf[WRITE_BUF_SIZE]

	if (c == BUF_FLUSH || i >= WRITE_BUF_SIZE)
	{
		write(1, buf i);
		i = 0;
	}
	if (c != BUF_FLUSH)
		buf[i++] = c;
	return (1);
}
