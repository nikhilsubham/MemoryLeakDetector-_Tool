#include <stdio.h>
#include "mld.h"
#include <stdlib.h>
#include "css.h"
#include <string.h>

char *DATA_TYPE[] = {"UINT8", "UINT32", "INT32",
                     "CHAR", "OBJ_PTR", "VOID_PTR",
                    "FLOAT","DOUBLE", "OBJ_STRUCT"};

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
    add_object_to_object_db(object_db, ptr, units, struct_rec,MLD_FALSE);
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
                     struct_db_rec_t *struct_rec,
                     mld_boolean_t is_root)
{
     
    object_db_rec_t *obj_rec = object_db_look_up(object_db, ptr);
    /*Dont add same object twice*/
    assert(!obj_rec);

    obj_rec = calloc(1, sizeof(object_db_rec_t));

    obj_rec->next = NULL;
    obj_rec->ptr = ptr;
    obj_rec->units = units;
    obj_rec->struct_rec = struct_rec;
    obj_rec->is_visited = MLD_FALSE;
    obj_rec->is_root = is_root;

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

             case OBJ_PTR:
               printf("%s[%d] -> %s = %p\n", struct_rec->struct_name,i,t->fname, 
                              (void*)*(int*)((obj_rec->ptr + (i*struct_rec->ds_size))+ offset));
               break;

             case OBJ STRUCT:
               break;

             default:
               break;
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
    printf("%-3d ptr = %-8p | next = %-8p | units = %-4d | str_name = %-10s| is_root = %s \n\n", 
        i, obj_rec->ptr, obj_rec->next, obj_rec->units, obj_rec->struct_rec->struct_name, 
        obj_rec->is_root ? "TRUE " : "FALSE"); 

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
        //mld_dump_object_rec_details(head);
    }
}


/*The global object of the application which is not created by xcalloc
 * should be registered with MLD using below API*/
void 
mld_register_global_object_as_root (object_db_t *object_db,
                          void *objptr,
                          char *struct_name,
                          unsigned int units){

    struct_db_rec_t *struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
    assert(struct_rec);

   /*Create a new object record and add to object database*/
   add_object_to_object_db(object_db, objptr, units, struct_rec, MLD_TRUE);  
}




/* Application might create an object using xcalloc , but at the same time the object
 * can be root object. Use this API to override the object flags for the object already
 * preent in object db*/
void
mld_set_dynamic_object_as_root(object_db_t *object_db, void *obj_ptr){

    object_db_rec_t *obj_rec = object_db_look_up(object_db, obj_ptr);
    assert(obj_rec);
    
    obj_rec->is_root = MLD_TRUE;
}


static void
init_mld_algorithm(object_db_t *object_db){

     object_db_rec_t *obj_rec = object_db->head;
     while(obj_rec){
         obj_rec->is_visited = MLD_FALSE;
         obj_rec = obj_rec->next;
     }
}

static object_db_rec_t *
get_next_root_object(object_db_t *object_db, 
                     object_db_rec_t *starting_from_here){

    object_db_rec_t *first = starting_from_here ? starting_from_here->next : object_db->head;
    while(first){
        if(first->is_root)
            return first;
        first = first->next;
    }
    return NULL;
}



/* Level 2 Pseudocode : This function explore the direct childs of obj_rec and mark
 * them visited. Note that obj_rec must have already visted.*/
static void
mld_explore_objects_recursively(object_db_t *object_db, 
                                object_db_rec_t *parent_obj_rec){
    
    unsigned int i , n_fields;
    char *parent_obj_ptr = NULL,
         *child_obj_offset = NULL;
    void *child_object_address = NULL;
    field_info_t *field_info = NULL;

    object_db_rec_t *child_object_rec = NULL;
    struct_db_rec_t *parent_struct_rec = parent_obj_rec->struct_rec;

    if(!parent_struct_rec){
        /* Handling void pointers : We cannot explore fields of objects which are of type void * in application.
         * Such objects will not have structure records, hence, nothing to do here*/
        return;
    }

    /*Parent object must have already visited*/
    assert(parent_obj_rec->is_visited);

    for( i = 0; i < parent_obj_rec->units; i++){

        parent_obj_ptr = (char *)(parent_obj_rec->ptr) + (i * parent_struct_rec->ds_size);

        for(n_fields = 0; n_fields < parent_struct_rec->n_fields; n_fields++){

            field_info = &parent_struct_rec->fields[n_fields];

            /*We are only concerned with fields which are pointer to
             * other objects*/
            switch(field_info->dtype){
                case UINT8:
                case UINT32:
                case INT32:
                case CHAR:
                case FLOAT:
                case DOUBLE:
                case OBJ_STRUCT:
                    break;
                case VOID_PTR:
                case OBJ_PTR:
                default:
                    ;

                /*child_obj_offset is the memory location inside parent object
                 * where address of next level object is stored*/
                child_obj_offset = parent_obj_ptr + field_info->offset;
                memcpy(&child_object_address, child_obj_offset, sizeof(void *));

                /*child_object_address now stores the address of the next object in the
                 * graph. It could be NULL, Handle that as well*/
                if(!child_object_address)
                    continue;
                  
                child_object_rec = object_db_look_up(object_db, child_object_address);
                //printf("\n%s\n",child_object_rec->ptr+0);

                assert(child_object_rec);
                
                if(!child_object_rec->is_visited){
                    child_object_rec->is_visited = MLD_TRUE;
                    mld_explore_objects_recursively(object_db, child_object_rec);
                }
                else{
                    continue; /*Do nothing, explore next child object*/
                }
            }
        }
    }
}




/* We will traverse the graph starting from root objects
 * and mark all reachable nodes as visited*/
void
run_mld_algorithm(object_db_t *object_db){

    /*Step 1 : Mark all objects in object databse as unvisited*/
    init_mld_algorithm(object_db);

    /* Step 2 : Get the first root object from the object db, it could be 
     * present anywhere in object db. If there are multiple roots in object db
     * return the first one, we can start mld algorithm from any root object*/
    object_db_rec_t *root_obj = get_next_root_object(object_db, NULL);
    //printf("\n%s\n",root_obj->ptr+0);

    while(root_obj){
        if(root_obj->is_visited){
           
            root_obj = get_next_root_object(object_db, root_obj);
            continue;
        }
        
        /*root objects are always reachable since application holds the global
         * variable to it*/ 
        root_obj->is_visited = MLD_TRUE;
        
        /*Explore all reachable objects from this root_obj recursively*/
        mld_explore_objects_recursively(object_db, root_obj);

        root_obj = get_next_root_object(object_db, root_obj);
    } 
}


void
report_leaked_objects(object_db_t *object_db){

    int i = 0;
    object_db_rec_t *head;

    printf("Dumping Leaked Objects\n");

    for(head = object_db->head; head; head = head->next){
        if(!head->is_visited){
            print_object_rec(head, i++);
            mld_dump_object_rec_details(head);
            printf("\n\n");
        }
    }
}
























