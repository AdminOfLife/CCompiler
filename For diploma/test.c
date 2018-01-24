/* Little C. ���������������� ��������� �1.

   ��� ��������� ������������� ������ ���� �������
   ����� C, �������������� ��������������� Little C.
*/

int i, j;   /* ���������� ���������� */
char ch;

int main()
{
  int i, j;  /* ��������� ���������� */
  /*
  struct test
  {
	  int a;
  };
  */
  puts("��������� ������������ Little C.");

  print_alpha();

  do {
    puts("������� ����� (0, ���� �����): ");
    i = getnum();
    if(i < 0 ) {
      puts("����� ������ ���� ��������������, ������� ���");
    }
    else {
      for(j = 0; j < i; j=j+1) {
        print(j);
        print("����� �����");
        print(sum(j));
        puts("");
      }
    }
  } while(i!=0);

  return 0;
}

/* ����� ����� �� 0 �� ���������� �����. */
int sum(int num)
{
  int running_sum;

  running_sum = 0;

  while(num) {
    running_sum = running_sum + num;
    num = num - 1;
  }
  return running_sum;
}

/* ����� �� ����� ����������� ��������. */
int print_alpha()
{
  for(ch = 'A'; ch<='Z'; ch = ch + 1) {
    putch(ch);
  }
  puts("");

  return 0;
}