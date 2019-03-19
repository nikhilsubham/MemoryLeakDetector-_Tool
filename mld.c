#include <stdio.h>
#include "mld.h"
#include <stdlib.h>
#include "css.h"
#include <string.h>

char *DATA_TYPE[] = {"UINT8", "UINT32", "INT32",
                     "CHAR", "OBJ_PTR", "FLOAT",
                     "DOUBLE", "OBJ_STRUCT"};

//head = head->next;
/*phase1 starts here*/


/*register application structure with structure database of library*/ 
void reg_structure(struct_db_t *struct_db, char* struct_name,int struct_size,field_info_t* fields_arr,int fields)
{
   struct_db_rec_t *rec = calloc(1, sizeof(struct_db_rec_t));
   strncpy(rec->struct_name, struct_name, MAX_STRUCTURE_NAME_SIZE);
   rec->ds_size = struct_size;
   rec->n_fields = fields;
   rec->fields = fields_arr; 
   
   if(add_structure_to_struct_db(struct_db, rec)){              
            assert(0);                                               
        }               
}


/*return 0 on success, -1 on failure for some reason*/
int add_structure_to_struct_db(struct_db_t *struct_db, struct_db_rec_t *struct_rec)
{
   if(!struct_db)
      return -1;

   if(!(struct_db -> head))
   {
       struct_db -> head =  struct_rec;
       struct_rec-> next = NULL;
       struct_db ->count =  struct_db ->count+ 1;
       return 0;
   }  
   
   struct_rec->next = struct_db -> head;
   struct_db -> head = struct_rec; 
   struct_db ->count =  struct_db ->count+ 1;
   return 0;
}


/*Print entire structure databbase*/
void print_structure_db(struct_db_t *struct_db)
{
    if(!struct_db || !(struct_db -> head))
      return;

    struct_db_rec_t *struct_rec= struct_db -> head;

    while(struct_rec)
    {
      print_structure_rec(struct_rec);
      struct_rec = struct_rec->next;
    }
    return;
}


/*Given a structure database and structure name finding structure record*/
static struct_db_rec_t*  struct_db_look_up(struct_db_t* struct_db, char* struct_name)
{
   if(!struct_db || !(struct_db->head))
      return NULL;

    struct_db_rec_t* struct_rec =  struct_db->head;

    while(struct_rec)
    {
       if(!strcmp(struct_rec->struct_name, struct_name))
          return struct_rec;
         
       struct_rec = struct_rec -> next;
    }
   return NULL;
}


/*Print a given particular structure record*/
void print_structure_rec(struct_db_rec_t *struct_rec)
{
  if(!struct_rec)
    return;

  printf("%-20s | size %-9d | fields %-9d\n\n", struct_rec->struct_name, 
            struct_rec->ds_size, struct_rec->n_fields);
  int j=0; 
  
  for(j; j<struct_rec->n_fields;j++)
    {
      field_info_t * t= &struct_rec->fields[j];
      printf("name  %-13s | D_type  %-8s | size  %-3d | offset  %-3d | %-6s\n", 
             t->fname, DATA_TYPE[t->dtype], t->size, t->offset, t->nested_str_name);
    }
  printf("----------------------------------------------------------------------------|\n");
}


/* phase 1 Ends here */
/* phase2 starts here*/


/*This function creates applicaton ojects and create a object database */
void * xcalloc(object_db_t *object_db, char *struct_name, int units)
{
    assert(object_db);
      
    struct_db_rec_t *struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
    assert(struct_rec);
    void *ptr = calloc(units, struct_rec->ds_size);
    add_object_to_object_db(object_db, ptr, units, struct_rec);
    return ptr;
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




/*Working with objects*/
static void
add_object_to_object_db(object_db_t *object_db, 
                     void *ptr, 
                     int units,
                     struct_db_rec_t *struct_rec)
{
     
    object_db_rec_t *obj_rec = object_db_look_up(object_db, ptr);
    /*Dont add same object twice*/
    assert(!obj_rec);

    obj_rec = calloc(1, sizeof(object_db_rec_t));

    obj_rec->next = NULL;
    obj_rec->ptr = ptr;
    obj_rec->units = units;
    obj_rec->struct_rec = struct_rec;

    object_db_rec_t *head = object_db->head;
        
    if(!head){
        object_db->head = obj_rec;
        obj_rec->next = NULL;
        object_db->count++;
        return;
    }

    obj_rec->next = head;
    object_db->head = obj_rec;
    object_db->count++;
}





void mld_dump_object_rec_details(object_db_rec_t* obj_rec)
{
  if(!obj_rec || !(obj_rec -> struct_rec))
    return;

  struct_db_rec_t *struct_rec =  obj_rec -> struct_rec;
  unsigned int n = obj_rec -> units;
  
  int i=0,j=0;
 
  for(i; i<n; i++)
   {
     for(j; j< struct_rec-> n_fields;j++)
       {
          field_info_t * t= &struct_rec->fields[j];
          unsigned int offset = t -> offset;
          switch(t->dtype)
           {
             case UINT8:
             case UINT32:
             case INT32:
             printf("%s[%d] -> %s = %d\n", struct_rec->struct_name,i,t->fname, 
                               *(int*)((obj_rec->ptr + (i*struct_rec->ds_size))+ offset));
             break;

             case CHAR: 
             printf("%s[%d] -> %s = %s\n", struct_rec->struct_name,i,t->fname, 
                               (char*)((obj_rec->ptr + (i*struct_rec->ds_size))+ offset));
             break;

             case FLOAT:
             printf("%s[%d] -> %s = %f\n", struct_rec->struct_name,i,t->fname, 
                              *(float*)((obj_rec->ptr + (i*struct_rec->ds_size))+ offset));
             break;

             case DOUBLE:
             printf("%s[%d] -> %s = %lf\n", struct_rec->struct_name,i,t->fname, 
                              *(double*)((obj_rec->ptr + (i*struct_rec->ds_size))+ offset));
             break;

             default:
             printf("%s[%d] -> %s = %p\n", struct_rec->struct_name,i,t->fname, 
                              (void*)(int*)((obj_rec->ptr + (i*struct_rec->ds_size))+ offset));
        }
       }
     printf("\n");
     j= 0;
   }
}



/*Delete object record from object database as well as freeing the object from memory*/
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
        return;
     }
     else
        head = head->next;
   }
}




/*Dumping Functions for Object database*/
void 
print_object_rec(object_db_rec_t *obj_rec, int i){
    
    if(!obj_rec) return;
    printf("---------------------------------------------------------------------------|\n");
    printf("%-3d ptr = %-8p | next = %-8p | units = %-4d | str_name = %-10s\n\n", 
        i, obj_rec->ptr, obj_rec->next, obj_rec->units, obj_rec->struct_rec->struct_name); 
    printf("Corresponding_oject details are as follows|\n");
}


void
print_object_db(object_db_t *object_db){

    if(!object_db || !object_db->head)
      return;

    object_db_rec_t *head = object_db->head;
    unsigned int i = 0;
    printf("\nPrinting OBJECT DATABASE\n");
    for(; head; head = head->next){
        print_object_rec(head, i++);
        mld_dump_object_rec_details(head);
    }
}



















