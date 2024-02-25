#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Функция для обмена двух элементов массива
void swap(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

// Функция для разделения массива на две части относительно опорного элемента
int partition(int *a, int l, int r) {
  // Выбираем случайный элемент в качестве опорного
  int pivot = a[rand() % (r - l + 1) + l];
  // Перемещаем все элементы, меньшие или равные опорному, в левую часть
  // Перемещаем все элементы, большие или равные опорному, в правую часть
  int i = l - 1;
  int j = r + 1;
  while (1) {
    do {
      i++;
    } while (a[i] < pivot);
    do {
      j--;
    } while (a[j] > pivot);
    if (i >= j) {
      return j;
    }
    swap(&a[i], &a[j]);
  }
}

// Функция для нахождения k-й порядковой статистики в массиве a[l..r]
int kth_order_statistic(int *a, int l, int r, int k) {
  // Если массив состоит из одного элемента, то он является искомым
  if (l == r) {
    return a[l];
  }
  // Разделяем массив на две части относительно опорного элемента
  int q = partition(a, l, r);
  // Определяем, в какой части находится искомая k-я порядковая статистика
  int left_size = q - l + 1;
  if (k <= left_size) {
    // Если в левой части, то запускаем рекурсивный вызов для нее в отдельном потоке
    int result;
    #pragma omp parallel
    {
      #pragma omp single nowait
      {
        result = kth_order_statistic(a, l, q, k);
      }
    }
    return result;
  } else {
    // Если в правой части, то запускаем рекурсивный вызов для нее в отдельном потоке
    int result;
    #pragma omp parallel
    {
      #pragma omp single nowait
      {
        result = kth_order_statistic(a, q + 1, r, k - left_size);
      }
    }
    return result;
  }
}

// Функция для вывода массива на экран
void print_array(int *a, int n) {
  for (int i = 0; i < n; i++) {
    printf("%d ", a[i]);
  }
  printf("\n");
}

// Главная функция
int main() {
  // Задаем размер и k-й элемент
  int n = 10;
  int k = 2;
  // Создаем и заполняем массив случайными числами
  int *a = (int *)malloc(n * sizeof(int));
  for (int i = 0; i < n; i++) {
    a[i] = rand() % 100;
  }
  // Выводим исходный массив на экран
  printf("Original array:\n");
  print_array(a, n);
  // Находим k-ю порядковую статистику
  int kth = kth_order_statistic(a, 0, n - 1, k);
  // Выводим результат на экран
  printf("%d-th order statistic is %d\n", k, kth);
  // Освобождаем память
  free(a);
  return 0;
}
