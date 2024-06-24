#include <stdio.h>
#include <string>
#include<iostream>
#include<Windows.h>
#include <time.h>
#pragma warning (disable:4996)

#define MAX_ROW 100
#define MAX_COL 100
#define MAX_THREAD 10000;

using namespace std;

#define DELIM		" \r\t"
#define MAX_LINE	1000

struct Param
{
	int* param1;
	int* param2;
	int size;
	//~Param() { delete[] param1, param2; }
	Param(int arr1[], int arr2[], int n)
	{
		param1 = arr1; param2 = arr2; size = n;
	}
};

DWORD WINAPI Calculate(LPVOID arg)
{
	Param param = *(Param*)arg;
	DWORD temp = 0;

	for (int i = 0; i < param.size; ++i)
	{
		temp = temp + param.param1[i] * param.param2[i];
	}
	delete (Param*)arg;
	return temp;
}



void CopyArray(int src[], int dest[], int size)
{
	for (int i = 0; i < size; ++i)
	{
		dest[i] = src[i];
	}
}

void CalculateMatrix(int mat1[][100], int mat1_h, int mat1_w, int mat2[][100], int mat2_h, int mat2_w, int result[][100], int* result_h, int* result_w)
{
	int* param1_copyed, * param2_copyed;
	*result_h = mat1_h;
	*result_w = mat2_w;
	int total = mat1_h * mat2_w;
	DWORD* threadId = new DWORD[total];
	HANDLE* hThread = new HANDLE[total];
	int param1[MAX_COL], param2[MAX_COL];
	DWORD ret;
	int count = 0;


	for (int i = 0; i < mat1_h; ++i)
	{
		for (int k = 0; k < mat2_w; ++k)
		{
			for (int j = 0; j < mat1_w; ++j)
			{
				param1[j] = mat1[i][j];
				param2[j] = mat2[j][k];
			}

			param1_copyed = new int[mat1_w];
			param2_copyed = new int[mat1_w];
			CopyArray(param1, param1_copyed, mat1_w);
			CopyArray(param2, param2_copyed, mat1_w);

			hThread[count] = CreateThread(NULL, 0, Calculate, new Param(param1_copyed, param2_copyed, mat1_w), 0, &threadId[count]);
			if (hThread[count] == NULL)
			{
				cout << "CreateThread Error" << endl << i << ' ' << k << ' ' << count;
				exit(0);
			}
			++count;
		}
	}

	WaitForMultipleObjects(count, hThread, TRUE, INFINITE);

	count = 0;
	for (int i = 0; i < mat1_h; ++i)
	{
		for (int k = 0; k < mat2_w; ++k)
		{
			GetExitCodeThread(hThread[count], &ret);
			result[i][k] = ret;
			CloseHandle(hThread[count]);
			++count;
		}
	}

	delete[] threadId, hThread;
}


void read_matrix(int A[MAX_ROW][MAX_COL], int& Ar, int& Ac, char* filename)
{
	char line[MAX_LINE], * tok;
	FILE* fp;

	if (!(fp = fopen(filename, "r")))
	{
		cout << "ERROR: file open" << endl;
		exit(0);
	}
	Ar = 0;
	while (fgets(line, MAX_LINE, fp))
	{
		tok = strtok(line, DELIM);
		Ac = 0;
		do
		{
			A[Ar][Ac++] = atoi(tok);

		} while (tok = strtok(NULL, DELIM));
		Ar++;
	}
	fclose(fp);
}

void WriteResult(int data[][100], int height, int width)
{
	FILE* fp = fopen("result.txt", "w");
	if (fp == NULL)
	{
		cout << "Can't write to file";
		exit(0);
	}

	string str;

	for (int i = 0; i < height; ++i)
	{
		str = "";
		for (int k = 0; k < width; ++k)
		{
			str = str + to_string(data[i][k]) + "\t";
		}
		str = str + "\n";
		fputs(str.c_str(), fp);
	}
	fclose(fp);
}


int main(int argc, char** argv)
{
	clock_t start, end;
	start = clock();


	if (argc <= 2 || argc >= 12)
	{
		cout << "ERROR: input sequence!..." << endl;
		cout << "Usage: mm <filename> <filename> ... (Maximum : 10)" << endl;
		return 0;
	}



	int matrix[10][MAX_ROW][MAX_COL];
	int width[10], height[10];
	int result[100][100];
	int result_width, result_height;
	int n = argc - 1;


	for (int i = 0; i < n; ++i)
		read_matrix(matrix[i], height[i], width[i], argv[i + 1]);


	for (int i = 0; i < n - 1; ++i)
	{
		printf("(%d x %d)  x  (%d x %d)  =  (%d x %d)\n", height[0], width[i], height[i + 1], width[i + 1], height[0], width[i + 1]);
		CalculateMatrix(matrix[0], height[0], width[0], matrix[i + 1], height[i + 1], width[i + 1], matrix[0], &height[0], &width[0]);
		Sleep(1000);
	}

	cout << endl;
	for (int i = 0; i < height[0]; ++i)
	{
		for (int k = 0; k < width[0]; ++k)
		{
			cout << matrix[0][i][k] << "\t";
		}
		cout << endl;
	}
	cout << endl;

	WriteResult(matrix[0], height[0], width[0]);

	end = clock();

	cout << endl << "Duration : " << (double)(end - start) / CLOCKS_PER_SEC << "sec" << endl;
}

