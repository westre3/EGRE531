quicksort:
	g++ quicksort.cpp -O0 -fopenmp -lpthread -o quicksort
  
clean:
	rm ./quicksort
