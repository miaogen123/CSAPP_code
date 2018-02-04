
//this version updates the ways of searching. 
//the last one without numbers in the name  has a way of  searching  from  head;
//this has  a way of searching from the position last stopped.



#pragma once
#include"memlib.c"

//Basic constant and macors
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)   //break incr by CHUNKSIZE

#define MAX(x,y) ((x)>(y)?(x):(y))

#define PACK(size, alloc) ((size)|(alloc))

#define GET(p)  (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p)=(val))

#define GET_SIZE(p) (GET(p)&~0x7)
#define GET_ALLOC(p)  (GET(p) &0x1)

#define HDRP(bp) ((char *)(bp) -WSIZE)   //head ptr of a block
#define FTRP(bp) ((char*)(bp)-GET_SIZE(HDRP(bp)- DSIZE))   //tail ptr of a block

#define NEXT_BLKP(bp)  ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp)+GET_SIZE(((char*)(bp)-DSIZE )))


int mm_init(void);
static void* extend_heap(size_t words);
void mm_free(void* bp);
static void* coalesce(void *bp);
void* mm_malloc(size_t size);
static void* find_fit(size_t asize);
static void place(void*bp, size_t size);


int mm_init(void)
{
	char *heap_listp;
	if((heap_listp=(char*)mem_sbrk(4*WSIZE))==(void*)-1)
		return -1;
	PUT(heap_listp,0);
	PUT(heap_listp+(1*WSIZE), PACK(DSIZE, 1));
	PUT(heap_listp+(2*WSIZE), PACK(DSIZE, 1));
	PUT(heap_listp+(3*WSIZE), PACK(0,1));
	heap_listp += (2*WSIZE);


	if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
		return -1;
	return 0;
}
static void* extend_heap(size_t words)
{
	char *bp;
	size_t size;
	size=(words % 2)?(words+1)*WSIZE:(words*WSIZE);
	if((long)(bp=(char*)mem_sbrk(size))==-1)
		return NULL;
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(size, 0));

	return  coalesce(bp);

}

void mm_free(void* bp)
{
	size_t size=(GET_SIZE(bp));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));

	coalesce(bp);

}


static void* coalesce(void *bp)
{
	size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc=GET_ALLOC(HDRP(PREV_BLKP(bp)));
	size_t size=GET_SIZE(HDRP(bp));

	if(prev_alloc&&next_alloc)   //prev and next both are allocated
		return bp;
	else if(prev_alloc&&!next_alloc){
		size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));

		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if(!prev_alloc&&next_alloc){
		size+=GET_SIZE(HDRP(PREV_BLKP(bp)));

		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		bp=HDRP(PREV_BLKP(bp));
	}
	else if(!prev_alloc&&!next_alloc){
		size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
		size+=GET_SIZE(HDRP(PREV_BLKP(bp)));

		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

		bp=HDRP(PREV_BLKP(bp));
	}

}

void* mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char *bp;

	if(size==0)
		return NULL;
	if(size<=DSIZE)
		asize=2*DSIZE;
	else
		asize=DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);
	if((bp=(char* )find_fit(asize))!=NULL){
		place(bp, asize);
		return bp;
	}
	
	extendsize=MAX(asize, CHUNKSIZE);
	if((bp=(char*)extend_heap(extendsize/WSIZE))==NULL)
		return NULL;
	place(bp, asize);
	return bp;
}


//below is the prototype in the book P603, which confused me
//the requirement is that accomplish a first match
//but there is no ways to access the first bytes pointer, it's tatic.
//to make it, i modify the memlib.c and add some interface.

//this func modifid
static void* find_fit(size_t asize)
{
	static char *pre_position=NULL;
	char *head=(pre_position==NULL)?(char*)Return_Head():pre_position;
	char *tail=(char*)Return_Tail();
	while(GET_ALLOC(HDRP(head))||GET_SIZE(HDRP(head))<asize ){   // when allocated || less than asize, move to the next
		head+=GET_SIZE(HDRP(head));
		if(head>tail){
			if(pre_position==NULL){
				printf("there is no suitable size\n");
				return NULL;
			}
			else if(tail!=pre_position){
				tail=pre_position;
				head=(char* )Return_Head();
			}
			else{

				printf("there is no suitable size\n");
				return NULL;
			}

		}
	}
	pre_position=head;
	return (void *)head;

}
static void place(void*bp, size_t size)
{
	char *hdrp=HDRP(bp);
	size_t pre_size=GET_SIZE(hdrp);

	if(pre_size-size<2*DSIZE){
		PUT(hdrp, PACK(pre_size,1));
		PUT(pre_size, PACK(pre_size,1));

	}

	char *ftrp=FTRP(bp);
	PUT(hdrp, PACK(size, 1));
	hdrp+=size;
	PUT(hdrp-WSIZE, PACK(size, 1));

	size_t new_size=(pre_size-size)&~7;
	PUT(hdrp, PACK(new_size, 0));
	PUT(hdrp+new_size-2*WSIZE, PACK(new_size, 0));
}


