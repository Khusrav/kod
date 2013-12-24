//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

const int NUM_INT = 0;		//Константы операций и лексем
const int NUM_FLOAT = 1;
const int MULT = 2;
const int DIV = 3;
const int PLUS = 4;
const int MINUS = 5;
const int LBRA = 6;
const int RBRA = 7;
const int EQUAL = 8;
const int LEX_ERROR = 9;
const int SYNT_ERROR = 10;
const int EMPTY = 11;

struct Node                //Узел дерева
{
	int Op;                //Операция
	struct Node *Left;     //Ссылка на левый узел-операнд
	struct Node *Right;    //Ссылка на правый узел-операнд
	union
	{
		int integer;       //Если узел является листом дерева (число)
		float real;        //сохраняем число в union
	} Value;
};

struct Stack               //Стек для хранения последовательности
{                          //операций и операндов
	struct Node *Current;
	struct Stack *Next;
};

int getToken(char *, void **, int *, int);    	//Возвращает номер лексемы
												//и, если это число, записывает
												//его в нераспрделенную память void*
int verifySyntax(char *, int, struct Node**);   //Проверяет синтаксис выражений
												//методом конечного автомата
struct Node* create(int, void *);               //Создание узла дерева
void push(struct Node *, struct Stack **);      //Функции для работы
struct Node* pop(struct Stack **);              //со стеком
int getPriority(int);                           //Возвращает приоритет операций
double calculate(struct Node *);                //Рассчитывает результат
void Draw(struct Node **, int, int);           	//Рисует дерево в консоли

int main(int argc, char* argv[])
{
	char *string;
	int i, size, token;
	void *value;
	double result = 0.0;
	struct Node *tree;

	string = (char *) malloc(40);
	printf("-->");
	scanf("%[^\r\n]", string);      			//ввод выражения
	size = strlen(string);
	if (verifySyntax(string, size, &tree) == 0) //проверка синтаксиса
	{
		result = calculate(tree);               //если все нормально
		printf("RESULT %f", result);            //рассчитываем рещультат
		printf("\n\nTREE\n");                   //и рисуем дерево
		Draw(&tree, 63, 0);
    }
	getch();
	return 0;
}

int getToken(char *buffer, void **value, int *i, int size)
{
	int k, l;
	int real; //метка для числа с пл. точкой
	char *id;

	real = 0;
	if (*i >= size)
		return EMPTY; //если мы вышли за пределы выражения, то лексем больше нет
	while (buffer[*i] == ' ') *i = *i + 1; //пропускаем пробелы
	switch (buffer[*i])                    //проверка на лексемы-операции
	{
		case '*' : return MULT;
		case '/' : return DIV;
		case '+' : return PLUS;
		case '-' : return MINUS;
		case '(' : return LBRA;
		case ')' : return RBRA;
		case '=' : return EQUAL;
	}
	if (buffer[*i] >= '0' && buffer[*i] <= '9')  //проверка на лексемы-числа
	{
		k = *i;
		while (k <= size && buffer[k] >= '0' && buffer[k] <= '9')
		{
			k++;
		}
		if (buffer[k] == ',') //встретили запятую - лексическая ошибка!
			return LEX_ERROR;
		if (buffer[k] == '.') //встретили точку - ожидаем число с пл. точкой
		{
			if (buffer[k + 1] < '0' && buffer[k + 1] > '9') //следующий символ
				return LEX_ERROR;                           //не цифра - ошибка!
			real = 1;
			k++;
			while (k < size && buffer[k] >= '0' && buffer[k] <= '9')
				k++;
		}
		id = (char*) malloc(k - *i);    //записываем все символы числа в строку
		for (l = *i; l < k; l++)
		{
			id[l - *i] = buffer[l];
		}
		id[l - *i] = '\0';
		if (real)                       //получаем значение числа, записываем
		{                               //в память void*
			*value = malloc(sizeof(float));
			*((float *)*value) = atof(id);
		}
		else
		{
			*value = malloc(sizeof(int));
			*((int *)*value) = atoi(id);
		}
		*i = k - 1;
		free(id);
		id = NULL;
		if (real)
			return NUM_FLOAT;
		else
			return NUM_INT;
	}
	return LEX_ERROR;                   //все остальное - лексические ошибки!
}

int verifySyntax(char *buffer, int size, struct Node **TREE)
{
	int state_machine[][6] = {            //Конечный автомат состояний:
		{  1, -1, -1,  0, -1, -1 },       //строки - состояния (-1 = ошибка),
		{ -1,  2,  2, -1,  3,  4 },       //столбцы - классы лексем.
		{  1, -1, -1,  0, -1, -1 },       //Столбцы-классы:
		{ -1,  2,  2, -1,  3,  4 }        //0 - число, 1 - выс. приор. опер.
		};                                //2 - низ. приор. опер., 3 - л. скобка
										  //4 - п. скобка, 5 - равно(конец)

	int counter = 0, state = 0;           //counter - подсчет скобок
	int i, token, type;                   //л. скобка = counter++
	void *value;                          //п. скобка = counter--

	struct Stack *OPERATIONS = NULL, *OPERANDS = NULL;
	struct Node *cur, *temp;
	struct Node *operation, *operand_left, *operand_right;

	i = 0;
	for (i = 0; i <= size; i++)
	{
		token = getToken(buffer, &value, &i, size);  //получаем лексему
		switch (token)
		{
			case 0:
				printf("NUM %d\n", *((int *) value));
				break;
			case 1:
				printf("NUM %f\n", *((float *) value));
				break;
			case 2:
				printf("MULT\n");
				break;
			case 3:
				printf("DIV\n");
				break;
			case 4:
				printf("PLUS\n");
				break;
			case 5:
				printf("MINUS\n");
				break;
			case 6:
				printf("LBRA\n");
				break;
			case 7:
				printf("RBRA\n");
				break;
			case 8:
				printf("EQUAL\n");
				break;
			case 9:
				printf("Lexical error in %d\n", i + 1);
				return -(i + 1);
			default:
				printf("EMPTY\n");
		}
		if (token == EMPTY)
			break;

		//Составляем дерево - по алгоритму сортировочный станции
		//с двумя стеками: стек ОПЕРАЦИЙ и стек ОПЕРАНДОВ.
		//При выталкивании очередных операций и операндов из стеков
		//мы связываем их между собой и посылаем в стек ОПЕРАНДОВ!
		//Таким образом в конце мы получим готовое дерево (это будет
		//единственный оставшийся элемент в стеке ОПЕРАНДОВ)
		cur = create(token, value);
		if (token == 0 || token == 1)
			push(cur, &OPERANDS);
		else if (OPERATIONS != NULL)
		{
			if (cur->Op == RBRA)
			{
				while (OPERATIONS->Current->Op != LBRA && OPERATIONS != NULL)
				{
					operation = pop(&OPERATIONS);
					operand_right = pop(&OPERANDS);
					operand_left = pop(&OPERANDS);
					operation->Right = operand_right;
					operation->Left = operand_left;
					push(operation, &OPERANDS);
				}
				if (OPERATIONS != NULL)
					pop(&OPERATIONS);
			}
			else if (cur->Op == LBRA)
            	push(cur, &OPERATIONS);
			else
			{
				while (getPriority(cur->Op) <= getPriority(OPERATIONS->Current->Op))
				{
					operation = pop(&OPERATIONS);
					operand_right = pop(&OPERANDS);
					operand_left = pop(&OPERANDS);
					operation->Right = operand_right;
					operation->Left = operand_left;
					push(operation, &OPERANDS);
					if (OPERATIONS == NULL) break;
				}
				push(cur, &OPERATIONS);
			}
		}
		else
        	push(cur, &OPERATIONS);

		switch (token) 			//узнаем класс лексемы
		{
			case 0: case 1:
				type = 0;
				break;
			case 2: case 3:
				type = 1;
				break;
			case 4: case 5:
				type = 2;
				break;
			default:
				type = token - 3;
		}
		state = state_machine[state][type];  //находим новое состояние
		if (state == -1) break;              //ошибка
		if (type == 3) counter++;
		if (type == 4) counter--;
		if (counter < 0) break;              //ошибка
	}

	if (state == 1 || state == 3 || state == 4)  //легальные конечные состояния
	{
		printf("Non syntax error\n");
		while (OPERATIONS != NULL)               //если остались операции в стеке
		{                                        //дособираем дерево
			operation = pop(&OPERATIONS);
			operand_right = pop(&OPERANDS);
			operand_left = pop(&OPERANDS);
			operation->Right = operand_right;
			operation->Left = operand_left;
			push(operation, &OPERANDS);
		}
		*TREE = (*OPERANDS).Current;
		return 0;
	}
	else if (counter < 0)                      //ОШИБКИ
	{
		printf("Too many parenthesis in %d\n", i + 1);
		*TREE = NULL;
		return -(i + 1);
	}
	else if (state == -1 || state == 2)
	{
		printf("Syntax error in %d\n", i+1);
		*TREE = NULL;
		return -(i + 1);
	}
}

struct Node* create(int token, void *value)
{
	struct Node *ptr;

	if ((ptr = malloc(sizeof(struct Node))) == NULL)
	{
		printf("Memory error\n");
		exit(-1);
	}
	ptr->Op = token;
	ptr->Left = NULL;
	ptr->Right = NULL;
	if (token == 0)
	{
		ptr->Value.integer = *((int *) value);
	}
	else if (token == 1)
	{
		ptr->Value.real = *((float *) value);
	}
	return ptr;
}

void push(struct Node *node, struct Stack **head)
{
	struct Stack *ptr;

	if ((ptr = malloc(sizeof(struct Stack))) == NULL)
	{
		printf("Memory error\n");
		exit(-1);
	}
	ptr->Current = node;
	ptr->Next = *head;
	*head = ptr;
}

struct Node* pop(struct Stack **head)
{
	struct Node *ptr = NULL;
    struct Stack *temp = *head;

	if (*head == NULL) return NULL;
	ptr = (*head)->Current;
	*head = temp->Next;
	free(temp);
	return ptr;
}

int getPriority(int op)
{
	switch (op)
	{
		case 2: case 3:	return 3;
		case 4: case 5: return 2;
		case 6: case 7: return -1;
	}
}

double calculate(struct Node *tree)       		//рекурсивно обходим дерево
{
	double left, right;

	if (tree->Op == NUM_INT)
		return (double) tree->Value.integer;
	else if (tree->Op == NUM_FLOAT)
		return (double) tree->Value.real;
	else
	{
		left = calculate(tree->Left);
		right = calculate(tree->Right);
		if (tree->Op == MULT)
			return left * right;
		else if (tree->Op == DIV)
			return left / right;
		else if (tree->Op == PLUS)
			return left + right;
		else if (tree->Op == MINUS)
			return left - right;
	}
}

//Рекурсивная прорисовка дерева
//
//На каждом шаге на вход функции поступает массив узлов дерева, расположенных
//на одном уровне, ширина - количество пробелов между соседними узлами и уровень.
//Мы просматриваем массив, если очердной элемент != NULL, записываем его в консоль
//и записываем его потомков в новый массив, иначе записываем NULL.
//В конце прохода по уровням дерева мы получим входной массив с одними NULL.
//Алгоритм заканчивает работу.

void Draw(struct Node **treeL, int width, int level)
{
	int i, j;
	struct Node **nextL;
	int count = (int) pow(2, level);
	int null_count = 0;
	nextL = (struct Node **) malloc(2 * count * sizeof(struct Node*));


	for (j = 0; j < count; j++)
	{
		if (j == 0)
			for (i = 0; i < width / 2; i++)
				printf("%c", ' ');
		else
			for (i = 0; i < width; i++)
				printf("%c", ' ');
		if (treeL[j] == NULL)
		{
			printf("%c", ' ');
			nextL[2*j] = NULL;
			nextL[2*j+1] = NULL;
			null_count += 2;
			continue;
		}
		else
		{
			switch (treeL[j]->Op)
			{
				case 0:
					printf("%d", treeL[j]->Value.integer);
					break;
				case 1:
					printf("%.1f", treeL[j]->Value.real);
					break;
				case 2:
					printf("*");
					break;
				case 3:
					printf("/");
					break;
				case 4:
					printf("+");
					break;
				case 5:
					printf("-");
					break;
				case 8:
					printf("EQUAL\n");
					break;
			}
			nextL[2*j] = treeL[j]->Left;
			nextL[2*j+1] = treeL[j]->Right;
			if (nextL[2*j] == NULL)
				null_count++;
			if (nextL[2*j+1] == NULL)
				null_count++;
		}
    }
	printf("\n");
	if (null_count < 2 * count)
		Draw(nextL, width / 2, level+1);
}
//---------------------------------------------------------------------------
