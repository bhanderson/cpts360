#include <stdio.h>
main(int argc, char *argv[], char *env[])
{
	int a,b,c;
	printf("enter main\n");
	a=1; b=2; c=3;
	A(a,b);
	printf("exit main\n");
}

int A(int x, int y)
{
	int d,e,f;
	printf("enter A\n");
	d=3; e=4; f=5;
	B(d,e);
}

int B(int x, int y)
{
	int g,h,i;
	printf("enter B\n");
	g=6; h=7; i=8;
	C(g,h);
	printf("exit B\n");
}

int C(int x, int y)
{
	int u,v,w, *fpC, *temp;
	printf("enter C\n");
	u=9; v=10; w=11;
	// call assembly files now
	fpC = (int*)get_ebp();


	printf("\nAddress\t\tContents\n--------\t--------\n");
	do{
		printf("%p\t%x\n",temp,(*temp));
		temp = fpC;
		do{
			printf("%p\t%x\n",temp,(*temp));
			temp++;
		}while(*temp!=0);
		printf("%p\t%x\n",temp,(*temp));
		printf("--------\t--------\n");
		fpC = (int*)*fpC;
	}while(*fpC!=0);

}
