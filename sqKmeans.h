#ifndef __SQ_KMEANS_H__
#define __SQ_KMEANS_H__

#include "circularqueue.h"
#include <math.h>

class Node
{
public:
    Node()
    {
        x = y = width = height = 0;
        next = 0;
    }
    Node(int x_, int y_, int width_, int height_)
    {
        x = x_;
        y = y_;
        width = width_;
        height = height_;
        next = 0;
    }
public:
    int x;
    int y;
    int width;
    int height;
    Node *next;
};

class Seq
{
public:
    Seq()
    {
        length = 0;
        x = y = 0;
        Node *n = new Node();
        head = tail = n;
        next = 0;
    }
    ~Seq()
    {
        release();
    }
    void release()
    {
        Node *p;
        while(head->next)
        {
            p = head->next->next;
            delete head->next;
            head->next = p;
        }
        tail = head;
    }
    int add(Node* node)
    {
        tail->next = node;
        tail = node;
        x = (x*length+node->x)/(length+1);
        y = (y*length+node->y)/(length+1);
        length++;
        return length;
    }
    bool del()
    {
        if(length<1)
            return false;
        Node *tmp = head;
        x = (x*length-tmp->x)/(length-1);
        y = (y*length-tmp->y)/(length-1);
        length--;
        head = head->next;
        delete tmp;
        return true;
    }
public:
    int length;
    double x;
    double y;
    Node *head;
    Node *tail;
    Seq *next;
};

class SqKmeans
{
public:
    SqKmeans(int maxsize_, int rtsize_, int radius_=10)
    {
        maxsize = maxsize_;
        rtsize = rtsize_;
        radius = radius_;
        flag_rt = false;
        head = new Seq(); 
        tail = head;
        maxSeq = 0;
        cq = new CircularQueue<Seq*>(maxsize);
    }

    ~SqKmeans()
    {
        release();
    }

    void release()
    {
        Seq *p;
        while(head->next)
        {
            p = head->next->next;
            delete head->next;
            head->next = p;
        }
        tail = head;
        maxSeq = 0;
        flag_rt = false;
        cq->clear();
    }

    bool add(int x, int y, int w, int h)
    {
        Node *node = new Node(x,y,w,h);
        Seq *p = head;
        Seq *q = 0;
        bool flag = false;
        while(p->next)
        {
            q = p->next;
            if( fabs(q->x-x) < radius && fabs(q->y-y) < radius )
            {
                q->add(node);
                Seq* tmp = cq->enqueue(q);
                
                if(tmp)
                    tmp->del();
                flag = true;
                if(q->length>=rtsize)
                {
                    flag_rt = true;
                    maxSeq = q;
                }
                break;
            }
            p = q;
        }
        if(!flag)
        {
            Seq *ns = new Seq();
            ns->add(node);
            cq->enqueue(ns);
            tail->next = ns;
            tail = ns;
        }
        return flag_rt;
    }

    void getResult(int &x, int &y, int &w, int &h)
    {
        x = maxSeq->x;
        y = maxSeq->y;
        w = h = 0;
        Node *p = maxSeq->head->next;
        while(p)
        {
            w += p->width;
            h += p->height;
            p = p->next;
        }
        w /= maxSeq->length;
        h /= maxSeq->length;
    }

private:
    double radius;
    CircularQueue<Seq*> *cq;

    int maxsize;
    int rtsize;
    bool flag_rt;
    Seq *head;
    Seq *tail;
    Seq *maxSeq;
};

#endif
