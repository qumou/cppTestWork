#include <stdio.h>
#include <stdlib.h>

struct aRecord
{
   char accountNo[25];
   char availBal[13];
   _Bool abSign;
   char ledgerBal[13];
   _Bool lbSign;
   char updDate[9];
   char updTime[7];
};

char tag[3] = {0};
char size[3] = {0};

int status; // 1--tag; 2--size; 3--value;
int iter;
int bodySize;
int numberOfTags;
int commitLimit;
int commitIter;

_Bool dateStat;
_Bool signOfAmount;
_Bool newRecord;

struct aRecord record;
static const struct aRecord EmptyStruct;


const char testStr[] = "NA0810000001AB13C000000001000LB13C000000002000UT192022-05-01 09:00:00\nNA09100000002AB13D000000005000LB13C000000000050UT192022-05-01 10:00:00\r\nNA101000000003AB13C000000010000UT192022-05-01 11:00:00\n";
// find nonzero token 
int findToken(char *str, int len)
{    
    for (int i = 0; i < len-1; i++)
    {
        if (str[i] != '0' & str[i] != '\0')
        {
            //printf("i - %i\n",i);
            return i;
        }        
    }
    return 666;
}
//insert sign in value;
int addSign(char *str, int len, _Bool sign)
{
    int index = 0;
    index = findToken(str, len);
    if (sign)
    {
     str[index-1]='-';    
    }else
        {
         str[index-1]='+';       
        }
return index-1;
}

void generateSQL(struct aRecord *record)
{
 int signAB;
 int signLB;

 signAB = addSign(record->availBal,sizeof(record->availBal),record->abSign);
 if(findToken(record->ledgerBal,sizeof(record->ledgerBal)) != 666 ) 
  {
   signLB = addSign(record->ledgerBal,sizeof(record->ledgerBal),record->lbSign);
   printf("INSERT INTO \"T_ACCOUNT\" (\"ACCOUNT_NO\",\"AVAIL_BAL\",\"LEDGER_BAL\",\"UPD_DATE\",\"UPD_TIME\") VALUES (\'%s\',%s,%s,%s,%s)\n",
            record->accountNo,&record->availBal[signAB],&record->ledgerBal[signLB],record->updDate,record->updTime);
  }
  else
    {
     printf("INSERT INTO \"T_ACCOUNT\" (\"ACCOUNT_NO\",\"AVAIL_BAL\",\"LEDGER_BAL\",\"UPD_DATE\",\"UPD_TIME\") VALUES (\'%s\',%s,NULL,%s,%s)\n",
        record->accountNo,&record->availBal[signAB],record->updDate,record->updTime);
    }
}
 
int main(int argc, char *argv[])
{
    iter = -1;
    status = 1;
    bodySize = 0;
    dateStat = 1;
    newRecord = 0;
    commitLimit = -1;
    commitIter = 1;
    
    // commit limit argument
    if(argc == 3)
        {
        if(strcmp(argv[1], "-c") == 0)
            {
             commitLimit = atoi(argv[2]);
            }else
                {
                 printf("Unknown option: %s\n",argv[1]);
                 return 1;
                }
        }else if (argc > 3)
            {
             printf("Error: Too many arguments.\n");
             return 1;
            }
    //main loop
    for (int ch; (ch = getchar()) != EOF;)// read from stdin -- main loop
    //for (int i = 0; i < sizeof(testStr); i++) // testStr loop
    {   iter++;
    //    int ch = str[i]; // testStr loop
        switch (ch)
        {
        case '\n':
            //printf("\nEND OF RECORD\n");
            //printf("Parsed record:\n AN -- %s\n AB -- %s\n LB -- %s\n UD -- %s\n UT -- %s\n SignAB -- %i\n SignLB -- %i\n",
            //       record.accountNo,record.availBal,record.ledgerBal,record.updDate,record.updTime,record.abSign,record.lbSign);
            generateSQL(&record);
            record = EmptyStruct;
            if (commitLimit != -1)
            { if ((commitIter%commitLimit) == 0 )
                {
                printf("COMMIT STATEMENT\n"); // commit???
                }
                commitIter++;
            }
            
            status = 1;
            iter = -1;
            dateStat = 1;
            break;
        case '\r':
            break;
            iter = -1;
        default:
            switch (status)
            {
            case 1: // parse tag
                     tag[iter] = ch;
                     if (iter == 1)
                        {
                         iter = -1;
                         status = 2;
                        // printf("tag -- %s\n", tag);
                        }  
                     break;
            case 2: // parse size;
                size[iter] = ch;
                if (iter == 1)
                {
                 iter = -1;
                 status = 3;
                 bodySize = atoi(size);
                // printf("size -- %d\n", atoi(size));
                }
                break;
            case 3: // parse value
                if (strcmp(tag, "NA") == 0)
                {
                    if (iter < bodySize)
                    {
                     record.accountNo[iter] = ch;
                     break;
                    }else 
                        {
                         iter = 0;
                         status = 1;
                         if (ch!='\n' & ch !='\r' ) // not new record
                         {
                           tag[0] = ch; //add token to tag
                           break;
                         }
                        }
                }else if (strcmp(tag, "AB") == 0)
                {
                    if (iter < bodySize)
                    {
                     if (ch == 'C') 
                        {
                         record.abSign = 0; // positive
                         break;
                        }else if(ch == 'D')
                            {
                             record.abSign = 1; // negative
                             break;        
                            }
                     record.availBal[iter-1] = ch; // offset -1 
                     break;
                    }else 
                        {
                         iter = 0;
                         status = 1;
                         if (ch!='\n' & ch != '\r' ) // not new record
                         {
                           tag[0] = ch;
                           break;
                         }
                        }
                }else if (strcmp(tag, "LB") == 0)
                    {
                    if (iter < bodySize)
                    {
                    if (ch == 'C') 
                        {
                         record.lbSign = 0; // positive
                         break;
                        }else if(ch == 'D')
                            {
                             record.lbSign = 1; //negative
                             break;        
                            } 
                     record.ledgerBal[iter-1] = ch; // offset -1
                     break;
                    }else 
                        {
                         iter = 0;
                         status = 1;
                         if (ch!='\n' & ch !='\r' ) // not new record
                         {
                           tag[0] = ch; //add token to tag
                           break;
                         }
                        }
                }else if (strcmp(tag, "UT") == 0 )
                {   
                    if (dateStat) // parse DATE
                    { //printf("%c",ch);
                     if (iter < bodySize)
                     {
                      if(ch != '-' & ch != ' ') //skip "-" and " " token 
                        {
                         record.updDate[iter] = ch;
                         break;
                        }else iter--;
                      if(ch == ' ') 
                        {
                         dateStat = 0; // 0 -- parse time
                         //printf("DATE---%s\n",record.updDate);
                         iter = -1;
                         break; 
                        }
                     }
                    }else if (iter < 6) //parseTime
                     { //printf("%c",ch);
                        if(ch != ':') //skip ":" token
                        { 
                         record.updTime[iter] = ch;
                         break;
                        }else iter--;
                      break;
                     }else
                        {
                         iter = 0;
                         status = 1;
                         if (ch!='\n' & ch !='\r' ) // not new record
                         {
                           tag[0] = ch;
                           break;
                         }
                        }
                }else printf("Unknown tag");break;
            default:
                break ;
            }
                   
            break;
        }
    }
    // Test reason for reaching EOF.
    // if (feof(stdin)) // if failure caused by end-of-file condition
    //     puts("End of file reached");
    // else if (ferror(stdin)) // if failure caused by some other error
    // {
    //     perror("getchar()");
    //     fprintf(stderr, "getchar() failed \n");
    //     exit(EXIT_FAILURE);
    // }
   return EXIT_SUCCESS;
}