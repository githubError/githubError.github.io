#include<stdio.h>

void number_of_tasks_running (int output[], int start[], int end[], int n, int query[], int m);
void number_of_tasks_running_2 (int output[], int start[], int end[], int n, int query[], int m);

int main () {
    int n = 3, m = 4, start[] = {0, 5, 2}, end[] = {4, 7, 8}, query[] = {1, 9, 4, 3};
    int output[m];
    
    
//    number_of_tasks_running(&output, start, end, n, query, m);
    
    number_of_tasks_running_2(&output, start, end, n, query, m);
    
    for (int i = 0; i < m; i++) {
        printf("%d ",*(output + i));
    }
}


// Method 2: O(m),(m>n) OR O(n),(m<n)
void number_of_tasks_running_2 (int *output, int start[], int end[], int n, int query[], int m) {
    
    int max = (m > n)?m:n;
    int min = (m > n)?n:m;
    
    for (int i = 0; i < max; i++) {
        int count = 0;
        int query_num = query[i];
        
        if (i > min) return;
        
        int start_num = start[i];
        int end_num = end[i];
        
        if (start_num > end_num) {
            int tmp = start_num;
            start_num = end_num;
            end_num = tmp;
        }
        
        if (query_num >= start_num && query_num < end_num) {
            count++;
        }
        *(output+i) = count;
    }
}


// Method 1: O(m*n)
void number_of_tasks_running (int *output, int start[], int end[], int n, int query[], int m) {
    
    for (int q = 0; q < m; q++) {
        int count = 0;
        int query_num = query[q];
        for (int i = 0; i < n; i++) {
            int start_num = start[i];
            int end_num = end[i];
            
            if (start_num > end_num) {
                int tmp = start_num;
                start_num = end_num;
                end_num = tmp;
            }
            
            if (query_num >= start_num && query_num < end_num) {
                count++;
            }
        }
        *(output+q) = count;
    }
}
