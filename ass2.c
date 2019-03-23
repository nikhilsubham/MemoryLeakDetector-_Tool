#include <stdio.h>
#include <stdlib.h>
#include "mld.h"
#include "LinkedList/LinkedListApi.h"

/*Compile the assignment */
/*
gcc -g -c mld.c -o mld.o
gcc -g -c LinkedList/LinkedListApi.c -o LinkedList/LinkedListApi.o
gcc -g -c ass1.c -o ass1.o
gcc -g -o exe1 ass1.o LinkedList/LinkedListApi.o mld.o
./exe1
*/

typedef struct student_ {
    char stud_name[32];
    ll_t *book_lst;  /*List of books borrowed by this student*/
    struct student_ *left;
    struct student_ *right;
} student_t ;


typedef struct university_ {
    student_t *student_lst;
} university_t; 

typedef struct book_ {
    char book_name[32];
    float price;
    student_t *stud; /*Student who borrowed this book*/
    struct book_ *left;
    struct book_ *right;
} book_t; 

typedef struct section_ {
    char section_name[32];
    book_t *book_list;   /* Doubly linked list of books in this section */
} section_t; 

typedef struct library_ {
    char lib_name[32];
    section_t *sections[2];
} library_t;    


void
init_register_application_structures_with_mld(object_db_t *object_db){
    
    /* In this function you should write code to register all application
     * related data structures above with mld library*/
    static field_info_t student_t_fields[] = {
        FIELD_INFO(student_t, stud_name, CHAR, 0),
        FIELD_INFO(student_t, book_lst, OBJ_PTR,  ll_t),
        FIELD_INFO(student_t, left, OBJ_PTR, student_t),
        FIELD_INFO(student_t, right, OBJ_PTR, student_t)
    };
    REG_STRUCT(object_db->struct_db, student_t, student_t_fields);

    static field_info_t university_t_fields[] = {
        FIELD_INFO(university_t, student_lst, OBJ_PTR, student_t)
    };
    REG_STRUCT(object_db->struct_db, university_t, university_t_fields);

    static field_info_t book_t_fields[] = {
        FIELD_INFO(book_t, book_name, CHAR, 0),
        FIELD_INFO(book_t, price, FLOAT, 0),
        FIELD_INFO(book_t, stud, OBJ_PTR, student_t),
        FIELD_INFO(book_t, left, OBJ_PTR, book_t),
        FIELD_INFO(book_t, right, OBJ_PTR, book_t)
    };
    REG_STRUCT(object_db->struct_db, book_t, book_t_fields);
    

    static field_info_t section_t_fields[] = {
        FIELD_INFO(section_t, section_name, CHAR, 0),
        FIELD_INFO(section_t, book_list, OBJ_PTR, book_t)
    };
    REG_STRUCT(object_db->struct_db, section_t, section_t_fields);

    static field_info_t library_t_fields[] = {
        FIELD_INFO(library_t, lib_name, CHAR, 0),
        FIELD_INFO(library_t, sections[0], OBJ_PTR, section_t),
        FIELD_INFO(library_t, sections[1], OBJ_PTR, section_t)
    };
    REG_STRUCT(object_db->struct_db, library_t, library_t_fields);    
}




section_t* search_book_sec(library_t *lib, char* b_name)
{
   section_t* section1 = NULL;
   book_t* books = NULL;
   int i=0;
   for(i; i<2;i++)
    {
        section1 = lib->sections[i];
        books =  section1-> book_list;

       while(books)
       {
           if(!(strncmp(books->book_name, b_name, sizeof(books->book_name)))){
                printf("The book %s is present\n", books->book_name);
                return section1;
           }
           books = books ->right;
        }
     }
  printf("The book is not present in library\n");
  return NULL;
}




book_t* search_book_lib(library_t *lib, char* b_name)
{
   section_t* section1 = NULL;
   book_t* books = NULL;
   int i=0;
   for(i; i<2;i++)
    {
        section1 = lib->sections[i];
        books =  section1-> book_list;

       while(books)
       {
           if(!(strncmp(books->book_name, b_name, sizeof(books->book_name)))){
                printf("The book %s is present\n", books->book_name);
                return books;
           }
           books = books ->right;
        }
     }
  printf("The book is not present in library\n");
  return NULL;
}




student_t* search_student_univ(university_t *univ, char* s_name)
{
  student_t* students = univ->student_lst;

  while(students)
  {
    if(!(strncmp(students-> stud_name,s_name, sizeof(students-> stud_name)))){
      printf("Given student is present\n");
      return students;
    }
     students = students->right;
  }
  return NULL;
}




void Assign_book(university_t *univ, library_t *lib, char* b_name, char* s_name)
{
    book_t* book = search_book_lib(lib, b_name);
    student_t* student = search_student_univ(univ, s_name);
    //student_t* student = book ->stud; 

    if(!book || !student)
      {
         printf("book can not be assigned\n");
         return;
      }
   
    if(book->stud != NULL){
        printf("book is assigned to someone else ");
        return;
     }

    book->stud = student;

    ll_t* book_lst = student->book_lst;
    singly_ll_node_t* list_book3 = singly_ll_init_node(book);

    singly_ll_add_node(book_lst, list_book3);
    return;
}



void Delete_book(object_db_t *object_db, library_t *lib, char*b_name)
{
   book_t* book = search_book_lib(lib, b_name);
   section_t* section1 = search_book_sec(lib, b_name);
   student_t* student = book ->stud; 
   ll_t * list = student -> book_lst;
   
   if(!book){
      printf("The book to be deleted is not preesent in the libbrary\n");
      return;
   }

   if(book -> stud != NULL){
     printf("The book is assignd to some student So, can not be deleted\n");  
     return;
   }

   if(book->left == NULL)
     {
       section1-> book_list = book->right;
       book->right->left = NULL;
       singly_ll_remove_node_by_dataptr(list, book);
       xfree(object_db,book);
       return;
     }

   book ->left->right = book ->right;
   if(book->right)
       book->right ->left = book->left;
   singly_ll_remove_node_by_dataptr(list, book);
   xfree(object_db, book);
   return;  

} 



void Delete_student(object_db_t *object_db, university_t *univ, char* s_name)
{
   student_t* student = search_student_univ(univ, s_name);
   ll_t* ll = student-> book_lst;

   if(!ll) {
        printf("Invalid Linked List\n"); 
        return;
    }
    if(is_singly_ll_empty(ll)){
        printf("Empty Linked List\n");
        return;
    }

    singly_ll_node_t* trav = GET_HEAD_SINGLY_LL(ll);
    book_t* book = NULL;
    while(trav){
        //printf("%d. Data = %p, node = %p\n", i, trav->data, trav);
        book = (book_t*)trav->data;
        book ->stud= NULL;
        trav = trav->next;
    }
    xfree(object_db,student);
} 


void De_assign_book(library_t *lib, char* b_name)
{
  book_t* book = search_book_lib(lib, b_name);
  student_t* student = book->stud;
  ll_t* ll = student-> book_lst;

  
   if(!ll) {
        printf("Invalid Linked List\n"); 
        return;
    }
    if(is_singly_ll_empty(ll)){
        printf("Empty Linked List\n");
        return;
    }
 
   singly_ll_node_t* trav = GET_HEAD_SINGLY_LL(ll);
   while(trav){
        
        if(book == (book_t*)trav->data){
            singly_ll_remove_node_by_dataptr(ll, book);
            book ->stud= NULL;
            trav = trav->next;
            return;
         }
    }
  
}






static void
main_menu(object_db_t *object_db, university_t *univ, library_t *lib){

    /*Your Main Menu goes here*/
    while(1)
    {
      printf("1. Search a particualar book in a library\n");
      printf("2. Assign a book to student\n");
      printf("3. De assign a book to student\n");
      printf("4. Delete a book\n");
      printf("5. Delete a student\n");
      printf("6. Run Mld algorithm\n");
      printf("7. Report leaked objects\n");
      printf("8. Exit\n");
   
      int choice;
      printf("enter your choice\n ");
      scanf("%d\n", &choice);
   
      switch(choice)
       {
         case1:
            printf("Please Enter a book name to search\n");
            char b_name[100];
            fgets(b_name,100,stdin);
            search_book_lib(lib, b_name);
            break;

         case2:
            printf("Please Enter a book name to Assign\n");
            fgets(b_name,100,stdin);
            printf("Please Enter a student name to Assign\n");
            char s_name[100];
            fgets(s_name,100,stdin);
            Assign_book(univ, lib, b_name, s_name);
            break;

         case3:
            printf("Please Enter a book name to De-assign\n");
            fgets(b_name,100,stdin);
            De_assign_book(lib, b_name);
            break;

          case4:
            printf("Please Enter a book name to De-assign\n");
            fgets(b_name,100,stdin);
            Delete_book(object_db,lib,b_name);  
            break;


          case5:
            printf("Please Enter a student name to Delete\n");
            fgets(s_name,100,stdin);
            Delete_student(object_db,univ,s_name); 
            break;


          case6:
            run_mld_algorithm(object_db);
            break;

          case7:
            report_leaked_objects(object_db);
            break;

          case8:
            return;
          
          Default:
            printf("\nwronge choice\n");
            break;
            
        }
   
   
    }
 
    
}



int 
main(int argc, char **argv){

   
     struct_db_t *struct_db = calloc(1, sizeof(struct_db_t));
     object_db_t *object_db = calloc(1, sizeof(object_db_t));
     object_db->struct_db = struct_db;

     init_register_application_structures_with_mld(object_db);
     init_register_linked_list_structures_with_mld(object_db);

     
     library_t *lib = xcalloc(object_db, "library_t", 1);
     mld_set_dynamic_object_as_root(object_db, lib);

     university_t *univ = xcalloc(object_db, "university_t", 1);
     //mld_set_dynamic_object_as_root(object_db, univ);


    book_t* book0 = xcalloc(object_db, "book_t", 1);
    book_t* book1 = xcalloc(object_db, "book_t", 1);
    book_t* book2 = xcalloc(object_db, "book_t", 1);
    book_t* book3 = xcalloc(object_db, "book_t", 1);
    book_t* book4 = xcalloc(object_db, "book_t", 1);

    section_t* section1 = xcalloc(object_db, "section_t", 1);
    section_t* section2 = xcalloc(object_db, "section_t", 1);

    //library_t* campus = xcalloc(object_db, "library_t", 1);

    student_t* stud0 = xcalloc(object_db, "student_t", 1);
    student_t* stud1 = xcalloc(object_db, "student_t", 1);

    //university_t* univ = xcalloc(object_db, "university_t", 1);

    
    ll_t*  list1 =  init_singly_ll();
    singly_ll_node_t* list_book1 = singly_ll_init_node(book0);
    singly_ll_node_t* list_book2 = singly_ll_init_node(book3);
    singly_ll_add_node(list1, list_book1);
    singly_ll_add_node(list1, list_book2);

    ll_t*  list2 =  init_singly_ll();
    singly_ll_node_t* list_book3 = singly_ll_init_node(book1);
    singly_ll_add_node(list2, list_book3);
    

    
    strncpy(stud0 -> stud_name, "Nikhil", strlen("Nikhil"));
    stud0->book_lst = list1;
    stud0->left = NULL;
    stud0->right = stud1;

    strncpy(stud1 -> stud_name, "Sachin", strlen("Sachin"));
    stud1->book_lst = list2;
    stud1->left = stud0;
    stud1->right = NULL;

    univ-> student_lst = stud0;


    strncpy(book0 -> book_name, "Magic of thnking big", strlen("Magic of thnking big"));
    book0->price = 200;
   // book0->stud = stud0;
    book0->stud = NULL;
    book0->left = NULL;
    book0->right = book1;

    strncpy(book1 -> book_name, "Great India", strlen("Great India"));
    book1->price = 300;
    //book1->stud = stud1;
    book1->stud = NULL;
    book1->left = book0;
    book1->right = book2;

    strncpy(book2 -> book_name, "How to become a rich", strlen("How to become a rich"));
    book2->price = 400;
    book2->stud = NULL;
    book2->left = book1;
    book2->right = NULL;

    strncpy(book3 -> book_name, "The earth", strlen("The earth"));
    book3->price = 500;
    //book3->stud = stud0;
    book3->stud = NULL;
    book3->left = NULL;
    book3->right = book4;

    strncpy(book4 -> book_name, "Harry potter", strlen("Harry potter"));
    book4->price = 600;
    book4->stud = NULL;
    book4->left = book3;
    book4->right = NULL;

   strncpy(section1 -> section_name, "art", strlen("art"));
   section1->book_list = book0;

   strncpy(section2 -> section_name, "social", strlen("social"));
   section2->book_list = book3;

   strncpy(lib -> lib_name, "sayaji", strlen("sayaji"));
   lib->sections[0] = section1;
   lib->sections[1] = section2;

   print_object_db(object_db);
   printf("\nprinting structure database\n");
   print_structure_db(object_db->struct_db);


   run_mld_algorithm(object_db);
   report_leaked_objects(object_db);

   //main_menu(object_db, univ, lib);


    search_book_lib(lib, "How to become a rich");
    search_book_lib(lib, "Hello world");

    return 0;
}
