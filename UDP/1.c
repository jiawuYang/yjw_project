#include <stdio.h>

//  20 34 -5 67 -24 67 98 5 0 


int main()
{
    int arr[10] = {0};
    int sum = 0;
    int i = 0, j = 0, k = 0;
    for (i = 0; i < 10; i++) {
        scanf("%d", &arr[i]);
    }
    for (j = 0; j < 10; j++) {
        if (arr[j] > 0) {
            sum = sum + arr[j];
        }
    }

    printf("%d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], arr[4]);
    printf("%d %d %d %d %d\n", arr[5], arr[6], arr[7], arr[8], arr[9]);
    printf("sum = %d\n", sum);
}