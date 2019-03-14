#include <stdio.h>
#include "mld.h"
#include <stdlib.h>

char *DATA_TYPE[] = {"UINT8", "UINT32", "INT32",
                     "CHAR", "OBJ_PTR", "FLOAT",
                     "DOUBLE", "OBJ_STRUCT"};

//char struct_name[MAX_STRUCTURE_NAME_SIZE];
//strcpy(struct_name, struct_rec->struct_name);
//int fields = struct_rec->n_fields;
//(obj->fname)
 
//object_db_rec_t *head = object_db_look_up(object_db, ptr);
//object_db_rec_t *headprev = object_db_look_up1(object_db, ptr);
//obj->fname
//field_info_t *fields = struct_rec-> fields;


/*return 0 on success, -1 on failure for some reason*/
int add_structure_to_struct_db(struct_db_t *struct_db, struct_db_rec_t *struct_rec)
{
   if(!(struct_db -> head))
   {
       struct_db -> head =  struct_rec;
       struct_rec-> next = NULL;
       return 0;
   }  
   else
   {
       struct_rec->next = struct_db -> head;
       struct_db -> head = struct_rec; 
       return 0;
   }
   struct_db ->count =  struct_db ->count+ 1;
}


void print_structure_rec(struct_db_rec_t *struct_rec)
{
  printf("%s\t %d\t %d\n", struct_rec->struct_name, struct_rec->ds_size, struct_rec->n_fields);
  printf("Below is diffrent field of this structure\n");
  
  int j=0; 
  //field_info_t * t = struct_rec->fields;
  //field_info_t * t= &struct_rec->fields[j];
  for(j; j<struct_rec->n_fields;j++)
    {
      field_info_t * t= &struct_rec->fields[j];
      printf("%s\t %s\t %d\t %d\t %s\n", t->fname,DATA_TYPE[t->dtype],t->size, t->offset,t->nested_str_name);
    }
   printf("\n***********************************************\n");
}


void print_structure_db(struct_db_t *struct_db)
{
    struct_db_rec_t *struct_rec= struct_db -> head;
    while(struct_rec)
    {
      print_structure_rec(struct_rec);
      struct_rec = struct_rec->next;
    }
}


struct_db_rect_t*  struct_db_look_up(struct_db_t* struct_db, char* struct_name)
{
    if(!struct_db || !(struct_db->head))
       return NULL;

    struct_db_rec_t* t =  struct_db->head;
    while(t)
    {
       if(!strcmp(t->struct_name, struct_name))
          return t;
         
       t= t -> next;
    }

    return NULL;
}



void mld_dump_object_rec_details(object_db_rec_t* obj_rec)
{
  struct_db_rec_t *struct_rec =  obj_rec -> struct_rec;
  unsigned int n = obj_rec -> units;
  struct_rec->struct_name * obj =  obj_rec->ptr;
  int i=0,j=0;
 
  for(i; i<n; i++)
   {
     for(j; j< struct_rec-> n_fields;j++)
       {
          field_info_t * t= &struct_rec->fields[j];
          printf("%s[%d] -> %s = %s\n", struct_rec->struct_name,i,t->fname,(char*)(obj-> (t->fname)));
       }
   }
}



static object_db_rec_t *
object_db_look_up(object_db_t *object_db, void *ptr){

    object_db_rec_t *head = object_db->head;
    if(!head) return NULL;
    
    for(; head; head = head->next){
        if(head->ptr == ptr)
            return head;
    }
    return NULL;
}



void xfree(object_db_t *object_db, void* ptr)
{
   assert(object_db_look_up(object_db, ptr));
  
   object_db_rec_t *head = object_db->head;
   object_db_rec_t *head1 = head;
   
   if(!head) return ;

   if(head->ptr == ptr){
     head = head -> next;
     free(head1->ptr);
     free(head);
     return;
   }

   //head -> next;
   while(head){
    if(head->next->ptr == ptr){
        head1 = head->next;
        head->next = head1->next;
        free(head1->ptr);
        free(head1);
        head = head->next;
     }
     else
        head = head->next;
   }
}



























