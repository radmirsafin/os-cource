#include "stdio.h"
#include "malloc.h"

struct thread {
    int thread_id;
    struct thread* next;
    struct thread* prev;
};

struct scheduler {
    int time_slice;
    int current_tick;
    struct thread* queue_head;
    struct thread* queue_tail;
};

struct scheduler* planner = NULL;


/**
 * Функция будет вызвана перед каждым тестом, если вы
 * используете глобальные и/или статические переменные
 * не полагайтесь на то, что они заполнены 0 - в них
 * могут храниться значения оставшиеся от предыдущих
 * тестов.
 *
 * time_slice - квант времени, который нужно использовать.
 * Поток смещается с CPU, если пока он занимал CPU функция
 * timer_tick была вызвана time_slice раз.
 **/
void scheduler_setup(int time_slice)
{
    planner = (struct scheduler*) malloc(sizeof(struct scheduler));
    planner->time_slice = time_slice;
    planner->current_tick = 0;
    planner->queue_head = NULL;
    planner->queue_tail = NULL;
}

void push_thread(int thread_id) {
    struct thread* new_thread = (struct thread*) malloc(sizeof(struct thread));
    new_thread->thread_id = thread_id;
    new_thread->next = planner->queue_tail;
    new_thread->prev = NULL;

    if (planner->queue_tail) {
        planner->queue_tail->prev = new_thread;
    }

    planner->queue_tail = new_thread;

    if (planner->queue_head == NULL) {
        planner->queue_head = planner->queue_tail;
    }
}

int pop_thread() {
    planner->current_tick = 0;

    struct thread* head_thread = planner->queue_head;
    if (head_thread == NULL) {
        return -1;
    }

    planner->queue_head = planner->queue_head->prev;
    if (planner->queue_head) {
        planner->queue_head->next = NULL;
    } else {
        planner->queue_tail = NULL;
    }

    return head_thread->thread_id;
}

/**
 * Функция должна возвращать идентификатор потока, который в
 * данный момент занимает CPU, или -1 если такого потока нет.
 * Единственная ситуация, когда функция может вернуть -1, это
 * когда нет ни одного активного потока (все созданные потоки
 * либо уже завершены, либо заблокированы).
 **/
int current_thread()
{
    if (planner->queue_head) {
        return planner->queue_head->thread_id;
    } else {
        return -1;
    }
}

/**
 * Функция вызывается, когда создается новый поток управления.
 * thread_id - идентификатор этого потока и гарантируется, что
 * никакие два потока не могут иметь одинаковый идентификатор.
 **/
void new_thread(int thread_id)
{
    push_thread(thread_id);
}

/**
 * Функция вызывается, когда поток, исполняющийся на CPU,
 * завершается. Завершится может только поток, который сейчас
 * исполняется, поэтому thread_id не передается. CPU должен
 * быть отдан другому потоку, если есть активный
 * (незаблокированный и незавершившийся) поток.
 **/
void exit_thread()
{
    pop_thread();
}

/**
 * Функция вызывается, когда поток, исполняющийся на CPU,
 * блокируется. Заблокироваться может только поток, который
 * сейчас исполняется, поэтому thread_id не передается. CPU
 * должен быть отдан другому активному потоку, если таковой
 * имеется.
 **/
void block_thread()
{
    pop_thread();
}

/**
 * Функция вызывается, когда один из заблокированных потоков
 * разблокируется. Гарантируется, что thread_id - идентификатор
 * ранее заблокированного потока.
 **/
void wake_thread(int thread_id)
{
    push_thread(thread_id);
}

/**
 * Ваш таймер. Вызывается каждый раз, когда проходит единица
 * времени.
 **/
void timer_tick()
{
    planner->current_tick += 1;
    if (planner->current_tick == planner->time_slice) {
        planner->current_tick = 0;
        int popped = pop_thread();
        if (popped >= 0) {
            push_thread(popped);
        }
    }
}
