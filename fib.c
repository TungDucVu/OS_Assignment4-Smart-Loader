/*
 * No changes are allowed in this file
 */
int arr[1024];

int fib(int n) {
  if(n<2) return n;
  else return fib(n-1)+fib(n-2);
}

int _start() {
   for (int i = 0; i < 1024; i++) {
      arr[i] = fib(3);

   }
	int val = fib(40);
	return val;
}
