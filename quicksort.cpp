#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <omp.h>

#define N 10000000
#define MAX_RAND 1000000
#define NUM_THREADS 6
#define NUM_EXECUTIONS 5

void swap(int *x,int *y) {
  int temp = *x;
  *x = *y;
  *y = temp;
}

int choose_pivot(int i,int j ) {
  return((i+j) / 2);
}

void quicksort(int list[],int m,int n) {
  int key, i, j, k; 

  if(m < n) {
    k = choose_pivot(m,n);
    swap(&list[m],&list[k]);
    key = list[m];
    i = m+1;
    j = n;

    while(i <= j) {
      while((i <= n) && (list[i] <= key))
        i++;
      while((j >= m) && (list[j] > key))
        j--;
      if(i < j)
        swap(&list[i],&list[j]);
    }

    // swap two elements
    swap(&list[m],&list[j]);

    // recursively sort the lesser list
    quicksort(list, m, j-1);
    quicksort(list, j+1, n);
  }
}

int *merge(int *a1, int a1_size, int *a2, int a2_size, int *result) {
  int i = 0, j = 0, k = 0;
  int *L = new int[a1_size];
  int *R = new int[a2_size];

  for(int i = 0; i < a1_size; i++) {
    L[i] = a1[i];
  }

  for(int i = 0; i < a2_size; i++) {
    R[i] = a2[i];
  }

  while((i < a1_size) && (j < a2_size)) {
    if(L[i] <= R[j]) {
      result[k] = L[i];
      i++;
    }
    else {
      result[k] = R[j];
      j++;
    }
    k++;
  }

  while(i < a1_size) {
    result[k] = L[i];
    i++;
    k++;
  }

  while(j < a2_size) {
    result[k] = R[j];
    j++;
    k++;
  }

  delete[] L;
  delete[] R;
}

void top_quicksort(int list[],int m,int n) {
  int key, i, j, k; 

  if(m < n) {
    k = choose_pivot(m,n);
    swap(&list[m],&list[k]);
    key = list[m];
    i = m+1;
    j = n;

    while(i <= j) {
      while((i <= n) && (list[i] <= key))
        i++;
      while((j >= m) && (list[j] > key))
        j--;
      if(i < j)
        swap(&list[i],&list[j]);
    }

    // swap two elements
    swap(&list[m],&list[j]);

    // recursively sort the lesser list
    #pragma omp parallel sections
    {
      #pragma omp section
      {
        quicksort(list, m, j-1);
      }

      #pragma omp section
      {
        quicksort(list, j+1, n);
      }
    }
  }
}

bool verify(int list[], int size) {
  for(int i = 0; i < size - 1; i++) {
    if(list[i] > list[i + 1]) {
      return false;
    }
  }
  return true;
}
    
int main() {
  int *array_serial = new int[N];
  int *array_parallel = new int[N];
  int rand_num;
  clock_t start_serial[NUM_EXECUTIONS];
  clock_t stop_serial[NUM_EXECUTIONS];
  clock_t start_parallel[NUM_EXECUTIONS];
  clock_t stop_parallel[NUM_EXECUTIONS];
  clock_t average_serial = 0, average_parallel = 0;
  
  srand(time(NULL));
  
  omp_set_num_threads(NUM_THREADS);
  omp_set_nested(0);
  omp_set_dynamic(0);
  
  for(int i = 0; i < NUM_EXECUTIONS; i++) {
    //Create random array and ensure it is not already sorted
    do {
      for(int j = 0; j < N; j++) {
        rand_num = (rand() % MAX_RAND);
        array_serial[j] = rand_num;
        array_parallel[j] = rand_num;
      }
    } while(verify(array_serial, N));

    start_serial[i] = clock();
    quicksort(array_serial, 0, N - 1);
    stop_serial[i] = clock();
  
    start_parallel[i] = clock();

    #pragma omp parallel sections
    {
      #pragma omp section
      {
        top_quicksort(array_parallel, 0, (N / 2) - 1);
      }

      #pragma omp section
      {
        top_quicksort(array_parallel, N / 2, N - 1);
      }
    }

    merge(array_parallel, N / 2, &array_parallel[N / 2], N / 2, array_parallel);
    stop_parallel[i] = clock();

    if(verify(array_serial, N)) {
      std::cout << "[S" << i+1 << "]: Array sorted in " << ((float) stop_serial[i] - start_serial[i]) / CLOCKS_PER_SEC << " seconds.\n";
    }
    else {
      std::cout << "[S" << i+1 << "]: Array incorrectly sorted.\n";
    }

    if(verify(array_parallel, N)) {
      std::cout << "[P" << i+1 << "]: Array sorted in " << ((float) stop_parallel[i] - start_parallel[i]) / CLOCKS_PER_SEC << " seconds.\n";
    }
    else {
    std::cout << "[P" << i+1 << "]: Array incorrectly sorted.\n";
    }

    //Update average times
    average_serial += stop_serial[i] - start_serial[i];
    average_parallel += stop_parallel[i] - start_parallel[i];
  }

  std::cout << "\n[S]: Average execution time: " << ((float) average_serial) / (NUM_EXECUTIONS * CLOCKS_PER_SEC) << "\n";
  std::cout << "[P]: Average execution time: " << ((float) average_parallel) / (NUM_EXECUTIONS * CLOCKS_PER_SEC) << "\n";

  delete[] array_serial;
  delete[] array_parallel;
  
  return 0;
}
