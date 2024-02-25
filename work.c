#include <stdio.h>
#include <string.h>

int findToken(char *str, int len)
{    
    for (int i = 0; i < len-1; i++)
    {
        if (str[i] != '0')
        {
            printf("i - %i\n",i);
            return i;
        }        
    }
return 666;
}

int main(int argc, char const *argv[])
{
    // char origStr[12] = "00000000000";
    // int index = 0;
    // index = findToken(origStr, sizeof(origStr));
    

    // if (index != 666)
    // {
    // origStr[index-1]='-';
    // printf ("new str = %s",&origStr[index-1]);
    // }
    // else printf("666");

    int d = 2;
    int dd = 2;

    int ddd = 2%2;

    printf("%i",ddd);

    return 0;
}
