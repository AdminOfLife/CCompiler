/* Little C. Демонстрационная программа №1.

   Эта программа демонстрирует работу всех средств
   языка C, поддерживаемых интерпритатором Little C.
*/

int main()
{
  int i, j;  /* локальные переменные */

  struct test
  {
	int a;
	struct test2
	{
		int b;
	}
  };

  puts("Программа демонстрации Little C.");
  getnum();
  return 0;
}
