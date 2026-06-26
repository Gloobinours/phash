#include "utils.h"

float
quickselect(float* arr, int n, int k)
{
  int left = 0, right = n - 1;
  while (left < right) {
    float pivot = arr[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
      if (arr[j] <= pivot) {
        i++;
        float tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
      }
    }
    float tmp = arr[i + 1];
    arr[i + 1] = arr[right];
    arr[right] = tmp;
    int pivot_idx = i + 1;

    if (pivot_idx == k)
      return arr[k];
    else if (pivot_idx > k)
      right = pivot_idx - 1;
    else
      left = pivot_idx + 1;
  }
  return arr[k];
}
