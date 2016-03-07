/*
 * TODO - 
 * 1. the type of class [record] 's key & value to be flexible (c++ template programming)
 * 2.
 */
#ifndef _MY_NAIVE_DB
#define _MY_NAIVE_DB
#define ULL unsigned long long

class record
{
public:
    ULL key;
    void* value;
};

class naive_db
{

public:
    ULL num;
    virtual void* search(ULL key) = 0;//pure virtual function
    virtual int ins(class record *rcd) = 0;
    virtual int del(ULL key) = 0;
};


class rbt_node : record{
public:
    class rbt_node* l;
    class rbt_node* r;
};

class naive_db_rbt : naive_db
{
private:
   class rbt_node* root;
public:
    void* search(ULL key);
    int ins(class record *rcd);
    int del(ULL key);
};


#endif