/* Little C. ���������������� ��������� �1.

   ��� ��������� ������������� ������ ���� �������
   ����� C, �������������� ��������������� Little C.
*/

int main()
{
  int i, j;  /* ��������� ���������� */

  struct test
  {
	int a;
	struct test2
	{
		int b;
	}
  };

  puts("��������� ������������ Little C.");
  getnum();
  return 0;
}
