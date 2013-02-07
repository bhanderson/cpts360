 main()
{ int a,b,c;
	a = 1; b = 2; c = 3;
	c = mysub(a,b);
	printf("c=%d\n", c); 
}

int mysub(int x, int y)
{
	int u, v;
	int *fpC, *temp;
	u = 4; v = 5;
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
	return (x+y+u+v);
}
