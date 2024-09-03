/**
 * =====================================
 * circular_queue.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include <string.h>

#ifndef __CIRCULARQUEUE_H__
#define __CIRCULARQUEUE_H__



/**
 * ==================
 * Circular_queue
 * ------------------
 *   Circular_queue<uint8> double_buf(2);
 *   Enqueue at the tail
 *   Dequeue from the head
 * ------------------
 */
template<class T> class Circular_queue {
public:

	uint8_t head_u8; // Read
	uint8_t tail_u8; // Write
	uint8_t count_u8;
	uint8_t size_u8;
	T *queue_a;

public:
	Circular_queue(uint8_t qsize_u8) {
		size_u8=qsize_u8;
		//queue_a=(T*)malloc(size_u8*sizeof(T));
		queue_a=new T [size_u8];
		count_u8=0;
		head_u8=0;
		tail_u8=0;
	}

	~Circular_queue() {
		size_u8=0;
		delete(queue_a);
		queue_a=(T*)0;
		count_u8=0;
		head_u8=0;
		tail_u8=0;
	}

	bool dequeue(uint8_t* data_pu8,uint8_t len_u8) {
		LOG_D("T(%d) - len_u8(%d)",sizeof(T),len_u8);
#if DEV_MODE
		ASSERT((int16_t)len_u8 <= (int16_t)sizeof(T));
#endif
		if (is_empty()) return false;

		memcpy((void*)data_pu8,(void*)(queue_a+head_u8),len_u8);

		count_u8--;
		head_u8++;
		if (head_u8==size_u8) head_u8=0;

		return true;
	}

	bool enqueue(uint8_t* data_pu8,uint8_t len_u8)
	{
		LOG_D("T(%d) - len_u8(%d)",sizeof(T),len_u8);
#if DEV_MODE
		ASSERT((int16_t)len_u8 <= (int16_t)sizeof(T));
#endif
		if (is_full()) return false;

		memcpy((void*)(queue_a+tail_u8),(void*)data_pu8,len_u8);

		count_u8++;
		tail_u8++;
		if (tail_u8==size_u8) tail_u8=0;

		return true;
	}

	uint8_t* __new(uint8_t len_u8)
	{
		LOG_D("T(%d) - len_u8(%d)",sizeof(T),len_u8);
#if DEV_MODE
		ASSERT((int16_t)len_u8 <= (int16_t)sizeof(T));
#endif
		if (is_full()) return (uint8_t*)0;
		uint8_t index_u8=tail_u8;
		uint8_t* data_pu8=(uint8_t*)(queue_a+tail_u8);

		count_u8++;
		tail_u8++;
		if (tail_u8==size_u8) tail_u8=0;

		memset((void*)(queue_a+index_u8),0,len_u8);

		return data_pu8;
	}

	inline bool is_empty()
	{
		return count_u8==0;
	}


	inline bool is_full()
	{
		return (count_u8==size_u8);
	}

	inline uint8_t size()
	{
		return size_u8;
	}


	inline uint8_t count()
	{
		return count_u8;
	}

	inline uint8_t space()
	{
		return size_u8-count_u8;
	}
};

/*
 * =====================
 * Circular_queue_simple
 * ---------------------
 *   Enqueue at the tail
 *   Dequeue from the head
 * ---------------------
 */
template<class T> class Circular_queue_simple : public Circular_queue<T> {
public:
	T def_T;
	inline Circular_queue_simple(uint8_t sz_u8,T def):Circular_queue<T>(sz_u8){def_T=def;}

	inline bool enqueue(T data){
		if (Circular_queue<T>::is_full()) return false;

		Circular_queue<T>::queue_a[Circular_queue<T>::tail_u8]=data;
		Circular_queue<T>::count_u8++;
		Circular_queue<T>::tail_u8++;
		if (Circular_queue<T>::tail_u8==Circular_queue<T>::size_u8) Circular_queue<T>::tail_u8=0;
		return true;
	}

	inline T dequeue() {
		if (Circular_queue<T>::is_empty()) return (T)def_T;
		T rtn=Circular_queue<T>::queue_a[Circular_queue<T>::head_u8];
		Circular_queue<T>::count_u8--;
		Circular_queue<T>::head_u8++;
		if (Circular_queue<T>::head_u8==Circular_queue<T>::size_u8) Circular_queue<T>::head_u8=0;
		return rtn;
	}

	/*
	 * -----------------
	 * Test if present
	 * -----------------
	 */
	inline bool is_present(T data)
	{
       for (uint8_t i=0;i<Circular_queue<T>::count_u8;i++)
		{
			if (data==Circular_queue<T>::queue_a[(Circular_queue<T>::head_u8+i)%Circular_queue<T>::size_u8]) return true;
		}
		return false;
	}
	//virtual inline bool operator == (const T* T_p){return false;}
};



/*
 * =====================
 * Chained_list
 * ---------------------
 *
 * ---------------------
 */

template<class T> class List
{
public:
	T data;
	void * next_p;
	void * prev_p;
};

template<class T> class Chained_list
{
public:
	List<T>* orig_p;
	uint8_t max_u8;
	uint8_t count_u8;
	/*
	 * Constructor
	 */
	inline Chained_list(uint16_t mx_u16){
		orig_p=(List<T> *)0;
		max_u8=mx_u16;
		count_u8=0;
	};
	/*
	 * Empty/full
	 */
	inline bool is_empty(){return (count_u8==0);}
	inline bool is_full(){return (count_u8==max_u8);}

	/*
	 * ---------------------------
	 * enqueue
	 * ---------------------------
	 * Insert at origin
	 * and return T.
	 * ---------------------------
	 * input: _p - address of T pointer
	 * output: in *_p
	 * ---------------------------
	 */
	inline bool enqueue(T** _p) {
		void *nxt_p;
		*_p=(T*)0;
		if (is_full()){
			LOG_D("Is full");
			return false;
		}
		List<T> *nlst_p=new(List<T>);
		/*
		 * New element take the place of orig_p.
		 * orig_p null means it is the first element, and the next_p of the last is null,
		 * Else next_p is orig_p.
		 * prev_p of new element is null.
		 */
		if (orig_p==0){
			nxt_p=0;
		} else {
			nxt_p=orig_p;
			orig_p->prev_p=(void*)nlst_p;
		}
		orig_p=nlst_p;
		orig_p->prev_p=(void*)0;
		orig_p->next_p=(void*)nxt_p;
		*_p=&(orig_p->data);
		count_u8++;
		return true;
	}


	/*
	 * ---------------------------
	 * del_next
	 * ---------------------------
	 * Delete next of prev_p
	 * ---------------------------
	 * input: lst_p pointer on List<T>
	 * ---------------------------
	 */
	inline bool del_next(List<T> **prev_p){
		List<T>* nxt_p;
		LOG_D("count_u8(%d) - *prev_p(0x%04x) - orig_p(0x%04x)",count_u8,*prev_p,orig_p);
		/*
		 * prev_p null means it is the origin
		 * nexp_p null means it is the last element
		 */
		if (orig_p==(List<T>*)0){
			LOG_E("count_u8(%d)",count_u8);
			return false;
		} else if (*prev_p==(List<T>*)0){
			nxt_p=(List<T>*)(orig_p->next_p);
			delete(orig_p);
			orig_p=(List<T>*)0;
			if (nxt_p){
				orig_p=nxt_p;
				orig_p->prev_p=(void*)0;
			}
		} else if ((*prev_p)->next_p){
			/*
			 * Get nxt of next of *prev_p
			 * Delete next of *prev_p
			 * Set next of *prev_p and prev of nxt
			 */
			nxt_p=(List<T>*)(((List<T>*)((*prev_p)->next_p))->next_p);
			delete((List<T>*)((*prev_p)->next_p));
			(*prev_p)->next_p=(void*)nxt_p;
			if(nxt_p) nxt_p->prev_p=(void*)(*prev_p);
		} else {
			LOG_E("nb_u16(%d)",count_u8);
			return false;
		}
		count_u8--;
		LOG_D("count_u8(%d)",count_u8);
		return true;
	}

	/*
	 * -----------------
	 * Test
	 * -----------------
	 * Virtual - overwritten when derived
	 */
	virtual inline bool test(const uint8_t type_u8,const T* T1_p,const T* T2_p) {ASSERT(false);return true;}

	/*
	 *  Delete T
	 */
	inline bool del_T(const uint8_t type_u8,const T* T1_p) {
		List<T> *prev_p,*cur_p;
		if (get_(type_u8,T1_p,&prev_p,&cur_p)){
			if (prev_p) del_next(&prev_p);
			LOG_V("true");
			return true;
		} else {
			LOG_V("false");
			return false;
		}
	}

	/*
	 * Get T regarding test type.
	 */
	inline bool get_T(const uint8_t type_u8,const T* T1_p,T** data_p,List<T> **prev_p,List<T>**cur_p)
	{
		if (get_(type_u8,T1_p,prev_p,cur_p))
		{
			if (cur_p) *data_p=&((*cur_p)->data);
			LOG_V("true");
			return true;
		} else {
			LOG_V("false");
			*data_p=(T*)0;
			return false;
		}
	}

	inline bool get_T(const uint8_t type_u8,const T* T1_p,T** data_p)
	{
		List<T> *prev_p,*cur_p;
		return get_T(type_u8,T1_p,data_p,&prev_p,&cur_p);
	}

	/*
	 * -----------------------
	 * first_T
	 * -----------------------
	 * Output:
	 *    T1_p: pointer on 1st T data
	 *    first_p: pointer to 1st lst
	 * -----------------------
	 */
	inline bool first_T(T** T1_p,List<T> **first_p) {
		if (is_empty())return false;
		//*first_p=(List<T> *)(orig_p->next_p);
		*first_p=(List<T> *)(orig_p);
		*T1_p=&((*first_p)->data);
		return true;
	}

	/*
	 * -----------------------
	 * next_T
	 * -----------------------
	 * Input:
	 *    cur_p: current list
	 *    Init with cur_p=(List<T>*)orig_p
	 * Output:
	 *    T1_p: pointer on 1st T
	 *    next_p: pointer to next lst
	 * -----------------------
	 */
	inline bool next_T(List<T> *cur_p,T** T_p,List<T> **next_p)
	{
		*T_p=(T*)0;
		if (cur_p==(List<T>*)0){
			LOG_E("No current");
			return false;
		}
		/*
		 * Set data pointer
		 */
		*T_p=&(cur_p->data);
		/*
		 * Set next
		 */
		*next_p=(List<T> *)(cur_p->next_p);
		return true;
	}

	/*
	 * Search *cur_p identified by T1_p regarding test type.
	 * Return previous and current.
	 */
	inline bool get_(const uint8_t type_u8,const T* T1_p,List<T> **prev_p,List<T> **cur_p)
	{
		*cur_p=(List<T>*)orig_p;
		*prev_p=(List<T>*)0;
		while ((*cur_p)&&!test(type_u8,&((*cur_p)->data),T1_p))
		{
			*prev_p=*cur_p;
			*cur_p=(List<T> *)((*cur_p)->next_p);
		}
		return (*cur_p)&&test(type_u8,&((*cur_p)->data),T1_p);
	}
};




#endif
