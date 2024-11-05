#define SIZE 1024
int __attribute__((aligned(4096))) A[SIZE] = { 0 };
int sum = 0;
int _start() {
  for (int i = 0; i < SIZE; i++) A[i] = 2;
  for (int i = 0; i < SIZE; i++)
    sum += A[i];
  return sum;
}

/*
User _start return value = 2048
Total page faults: 3
Total page allocations: 3
Total internal fragmentation: 11.89 KB
*/
