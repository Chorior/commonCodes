#include <iostream>
#include <random>

class PZsort
{
public:
	PZsort() = default;
	~PZsort() = default;

	void shell_sort(int *array, int length);
	void fast_sort(int *array, int length);
	//void dijkstra_fast_sort(int *array, int length);
	//void improved_dijkstra_fast_sort(int *array, int length);
	bool is_sorted(int *array, int length);
private:	
	void swap(int *array, int indexA, int indexB);	
	int get_random_num(int from, int to);
	void shuffle(int *array, int length);
	int partition(int *array, int indexA, int indexB);
	
};

bool PZsort::is_sorted(int *array, int length)
{
	for(int i = 1; i < length; ++ i)
		if(*(array + i) < *(array + i - 1))
			return false;
	return true;
}

void PZsort::swap(int *array, int indexA, int indexB)
{
	int temp = *(array + indexA);
	*(array + indexA) = *(array + indexB);
	*(array + indexB) = temp;
}

int PZsort::get_random_num(int from, int to)
{
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(from, to);
	return distr(eng);
}

void PZsort::shuffle(int *array, int length)
{
	for(int i = 0; i < length; ++ i)
	{
		int j = i + get_random_num(0, length - i - 1);
		int temp = *(array + i);
		*(array + i) = *(array + j);
		*(array + j) = temp;
	}
}

int PZsort::partition(int *array, int indexA, int indexB)
{
	int i = indexA, j = indexB + 1;
	//int random_index = get_random_num(indexA, indexB);
	int v = *(array + indexA);
	while(1)
	{
		while( array[++i] < v) if(i == indexB) break;
		while(v < array[--j]) if(j == indexA) break;
		if(i >= j) break;
		swap(array, i, j);
	}	
	swap(array, indexA, j);
	return j;
}

void PZsort::shell_sort(int *array, int length)
{
	int h = 1;
	while(h < length/3) h = 3*h + 1;
	while(h >= 1)
	{
		for(int i = h; i < length; ++ i)
			for(int j = i; j >= h && *(array + j) < *(array + j - h); j -= h)
				swap(array, j, j-h);

		h = h/3;
	}
}


void PZsort::fast_sort(int *array, int length)
{
	shuffle(array,length);
	if(length <= 1) return;
	int j = partition(array, 0, length - 1);
	fast_sort(array, j);
	fast_sort(array + j + 1, length - 1  - j);
}










