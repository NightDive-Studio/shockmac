/* Test parser for hashtable */
%token IDENT 1 INTLIT 2 ASSIGN 3 SEMI 4 DELETE 5 INSERT 6 LIST 7
%start file
%{
#include "lg.h"
#include "error.h" 
#include "string.h" 
#include "hash.h" 
#include "hashtest.h"
#include "mprintf.h"
#define yyerror printf
Hashtable TestHash;
extern bool my_iter_func(void* elem, void* data);
%}
%% 

        file : sequence ;

        sequence : statement SEMI | sequence statement SEMI ;

        statement : _set | _insert  | _delete | _null | _lookup | _list ;

        _null : ; 

        _set : IDENT ASSIGN INTLIT 
               {
                 HashElem elem;
                 strncpy(elem.key,(char*)$1,HASHELEMSIZE);
                 elem.val = $3;
                 hash_set(&TestHash,&elem);
               } ; 

        _insert : INSERT IDENT ASSIGN INTLIT 
               {
                 HashElem elem;
                 strncpy(elem.key,(char*)$2,HASHELEMSIZE);
                 elem.val = $4;
                 hash_insert(&TestHash,&elem);
               } ; 

        _delete : DELETE IDENT 
               {
                 HashElem elem;
                 strncpy(elem.key,(char*)$2,HASHELEMSIZE);
                 hash_delete(&TestHash,&elem);
               } ; 

        _lookup : IDENT
               {
                 HashElem elem,*result;
                 strncpy(elem.key,(char*)$1,HASHELEMSIZE);
                 hash_lookup(&TestHash,&elem,&result);
                 if (result != NULL)
                         printf("%s: %d\n",$1,result->val);
                      else printf("%s not found\n",$1);
               }  ;

        _list   : LIST { hash_iter(&TestHash,my_iter_func,NULL); } |
                  LIST IDENT { hash_iter(&TestHash,my_iter_func,(void*)$2); } ;

%% 

int my_hash_func(void* duh)
{
  HashElem* elem = (HashElem*)duh;
  int i,sum = 0;
  for (i = 0; i < HASHELEMSIZE; i++)
    if (elem->key[i] == '\0') return sum;
    else sum += elem->key[i];
  return sum;
}

int my_equ_func(void* arg1, void* arg2)
{
   HashElem  *elem1 = (HashElem*)arg1, 
             *elem2 = (HashElem*)arg2;
   return strncmp(elem1->key,elem2->key,HASHELEMSIZE);
}

bool my_iter_func(void* elem, void* data)
{
  HashElem *e = (HashElem*)elem;
  char* s = (char*)data;
  printf("%s: %d\n",e->key,e->val);
  return data != NULL && strncmp(e->key,s,HASHELEMSIZE) == 0;
}

main()
{
  errtype err;
  DbgMonoConfig();
  err = hash_init(&TestHash,sizeof(HashElem),3,my_hash_func,my_equ_func);
  if (err != OK) printf("Error on init: %d\n",err);
  while(TRUE) yyparse();
}

