/*
 * TODO - 
 * 1. the type of class [record] 's key & value to be flexible (c++ template programming)
 * 2.
 */
#ifndef _MY_NAIVE_DB
#define _MY_NAIVE_DB
#define ULL unsigned long long
#define DBG
#include <stdio.h>
#include <malloc.h>
#include <string.h>
void _update_value(void* dest, int start_idx, int len, void* src){
    memcpy((void*)((char*)dest + start_idx), src, len);
}

void *get_field(void* value, const char *field_name){
    //valid field: w_time | r_time | max_rsize | min_rsize | avg_rsize | std_rsize 
    int rcd_idx[] = {0, 8, 16, 20, 24, 28, -1};
    const char* valid[6] = {"w_", "r_", "ma", "mi", "av", "st"};
    int i = 0;
    for(i = 0; i < 6; i++)
        if(valid[i][0] == field_name[0] && valid[i][1] == field_name[1] ){
            break;
        }
    return rcd_idx[i] >= 0 ? (char*)value + rcd_idx[i] : 0; 
}

int is_new_ins(void* v_ptr){
    ULL wt = *(ULL*)get_field(v_ptr, "w_time");
    ULL rt = *(ULL*)get_field(v_ptr, "r_time");
//    printf("w r %llu %llu %d\n", wt, rt, (wt + rt) == 0 ? 1 : 0);
    return wt + rt == 0 ? 1 : 0;
}
/*
 * a record - <key, value>
 * key(id) | value                                                            |
 * 8B      |  8B     | 8B     | 4B        | 4B        | 4B        | 4B        |(total - 32Bytes)
 *         |  w_time | r_time | max_rsize | min_rsize | avg_rsize | std_rsize |
 * a record - 40Bytes
 * a rbt_node - (48)
 * a rbtree of 2,000,000 nodes -  around 100MB
 */
class record
{
public:
    record();
    record(ULL key);
    record(ULL key, void* v_ptr);
    ~record();
    ULL key;
    char* value;
};
record::record()
{
    key = 0;
    value = new char[32];
    memset(value, 0, sizeof(char)*32);
}
record::record(ULL key)
{
    this->key = key;
    value = new char[32];
    memset(value, 0, sizeof(char)*32);
}
record::record(ULL key, void* v_ptr)
{
    this->key = key;
    value = new char[32];//copy
    if(v_ptr)
        memcpy((void*)value, v_ptr, 32);
    else
        memset(value, 0, sizeof(char)*32);
}
record::~record()
{
    delete [] value;
}
/*
 * naive_db - a simple key-value in-memory db, 
 * whose record is the class defined before.
 * 
 * it's kernel data structure could be RB-tree or other.
 * By 2016.3.8 only implement RB-tree.
 * TODO - add AVL-tree & B-tree structure then compare the efficiency
 * 
 */
class naive_db//
{
public:
    ULL num;
    naive_db();
    virtual ~naive_db();//it will call the inhreit class's desturctor
    virtual record* search(ULL key) = 0;//pure virtual function
    virtual record* ins(ULL key) = 0;
    virtual int del(ULL key) = 0;
    void nodes_number(){
        printf("db's nodes: %llu\n", num);
    }
};
naive_db::naive_db()
{
    num = 0;
}
naive_db::~naive_db()
{
}


/*
 * naive_db's implement - rbtree
 */

/*
 * rbtree's node.
 */
class rbt_node : public record//the "public" must be add, or the base class's public member can't be access  
{
private:
    char color;//0 - red, 1 - black
public:
    rbt_node();
    rbt_node(ULL key);
    rbt_node(ULL key, void* v_ptr);
    ~rbt_node();
    class rbt_node* l;
    class rbt_node* r;
    class rbt_node* p;
    void set_black(){
        color = 1;
    }
    void set_red(){
        color = 0;
    }
    char get_color(){
        return color;
    }
};
rbt_node::rbt_node()
{
    p = l = r = 0;
    color = 0;
}
rbt_node::rbt_node(ULL key) : record(key)
{
    p = l = r = 0;
    color = 0;
}
rbt_node::rbt_node(ULL key, void* v_ptr) : record(key, v_ptr)
{
    p = l = r = 0;
    color = 0;
}
rbt_node::~rbt_node()
{
    
}

class naive_db_rbt : public naive_db
{
private:
    rbt_node* root;
    void _insert_fix(rbt_node* x);
    rbt_node* _search(ULL key);
    void _l_rotate(rbt_node* x);
    void _r_rotate(rbt_node* x);
    void _travel_rbt_del(rbt_node* r);
    void _travel_rbt_pr(rbt_node* r);
public:
    void travel_rbt_pr();
    naive_db_rbt();
    ~naive_db_rbt();
    record* search(ULL key);
    record* ins(ULL key);
    int del(ULL key);
};

void naive_db_rbt::_travel_rbt_pr(rbt_node* r){
    if(r){
        printf("%llu\n", r->key);
        _travel_rbt_pr(r->l);
        _travel_rbt_pr(r->r);
    }
}
void naive_db_rbt::travel_rbt_pr()
{
    _travel_rbt_pr(root);
}
void naive_db_rbt::_travel_rbt_del(rbt_node* r)
{
    if(r){
        _travel_rbt_del(r->l);
        _travel_rbt_del(r->r);
        //printf("del key -  %llu\n", r->key);
        delete r;
    }
}

naive_db_rbt::naive_db_rbt()
{
    root = 0;
}

naive_db_rbt::~naive_db_rbt(){
    #ifdef DBG
    printf("rbt's destructor.free(delete) %llu inserted nodes.\n", this->num);
    #endif
    //go through the whole rbtree.
    _travel_rbt_del(root);
}

rbt_node* naive_db_rbt::_search(ULL key)
{
    rbt_node *ptr = root, *ptr_n = 0;
    while(ptr){
        if(key == ptr->key)
            return ptr;
        else{
            ptr_n = ptr;
            ptr = key > ptr->key ? ptr->r : ptr->l;
        }
    }
    return ptr_n;
}


/*
 * search the node of @key
 */
record* naive_db_rbt::search(ULL key)
{
    rbt_node *ptr = _search(key);
    return ptr->key == key ? ptr : 0;
}

void naive_db_rbt::_l_rotate(rbt_node* x)
{
    rbt_node *y = x->r;
    x->r = y->l;
    if(x->r)
        x->r->p = x;
    y->p = x->p;    
    if(x->p){
        if(x == x->p->l)
            x->p->l = y;
        else
            x->p->r = y;
    }else{
        this->root = y;
    }
    y->l = x;
    x->p = y;
}

void naive_db_rbt::_r_rotate(rbt_node* x)
{
    rbt_node *y = x->l;
    //in rbtree, if "right_rotate" is called, @y is not NULL
    x->l = y->r;
    if(x->l)
        x->l->p = x;
    y->p = x->p;
    if(x->p){
        if(x == x->p->l)
            x->p->l = y;
        else
            x->p->r = y;
    }else{
        this->root = y;
    }
    y->r = x;
    x->p = y;
}

void naive_db_rbt::_insert_fix(rbt_node* x)
{
    rbt_node *pa = x->p, *u;//parent, uncle
    while(pa && (!pa->get_color())){
        //p is red.
        if(pa == pa->p->l){
            u = pa->p->r;
            if(u && (!u->get_color())){
                pa->set_black();u->set_black();
                pa->p->set_red();
                x = pa->p;
                pa = x->p;
                continue;
            }else{
                //u is black.
                if(x == pa->l){
                    pa->set_black();
                    pa->p->set_red();
                    _r_rotate(pa->p);
                    break;
                }else{
                    _l_rotate(pa);
                    x = pa;
                    pa = x->p;
                    continue;
                }
            }
        }else{
            //mirror.
            u = pa->p->l;
            if(u && (!u->get_color())){
                pa->set_black();u->set_black();
                pa->p->set_red();
                x = pa->p;
                pa = x->p;
                continue;
            }else{
                if(x == pa->r){
                    pa->set_black();
                    pa->p->set_red();
                    _l_rotate(pa->p);
                    break;
                }else{
                    _r_rotate(pa);
                    x = pa;
                    pa = x->p;
                    continue;
                }
            }
        }
    }
}


/*
 * insert a new record.
 * if the record already exist in the tree, return the record.
 * to see if already-exist, just look at the value field if all zero.
 * set the value field after insertion.
 */
record* naive_db_rbt::ins(ULL key)
{
    rbt_node *ptr = _search(key), *tmp;
    if(!ptr){
        ptr = this->root = new rbt_node(key);
        this->num++;
        goto out;
    }
    if(ptr->key == key){
        goto out;
    }
    tmp = new rbt_node(key);
    this->num++;
    if(key > ptr->key)
        ptr->r = tmp;
    else
        ptr->l = tmp;
    tmp->p = ptr;
    _insert_fix(tmp);
    ptr = tmp;
out:
    this->root->set_black();
    return ptr;
}


int naive_db_rbt::del(ULL key)//not completed!
{
    
    return 0;
}
#endif