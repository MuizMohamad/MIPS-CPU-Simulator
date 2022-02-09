/*************************************************************************************|
|   1. YOU ARE NOT ALLOWED TO SHARE/PUBLISH YOUR CODE (e.g., post on piazza or online)|
|   2. Fill memory_hierarchy.c                                                        |
|   3. Do not use any other .c files neither alter mipssim.h or parser.h              |
|   4. Do not include any other library files                                         |
|*************************************************************************************/

#include "mipssim.h"

uint32_t cache_type = 0;

const int block_size = 16;
int num_of_data = 4;

int* LRUpolicy;


///////////////////////////////////
/// CACHE OPERATIONS PROTOTYPE
///////////////////////////////////

// DIRECT MAPPED CACHE OPERATIONS
int direct_mapped_cache_read(int address);
int direct_mapped_cache_read_miss(int address);
void direct_mapped_cache_write(int address,int write_data);
//

// FULLY ASSOCIATE CACHE OPERATIONS
int fully_assoc_cache_read(int address);
int fully_assoc_cache_read_miss(int address);
void fully_assoc_cache_write(int address,int write_data);

// TWO-WAY SET CACHE OPERATIONS
int two_way_cache_read(int address);
int two_way_cache_read_miss(int address);
void two_way_cache_write(int address,int write_data);

/////////////////////////////////////
/// QUEUE IMPLEMENTATION
/////////////////////////////////////

// Queue data structure for LRU policy

struct node{
    int value;
    struct node* next;
};

struct queue{
    struct node *head;
    struct node *tail;
};

//struct queue LRUqueue;

struct queue init_queue(){
    struct queue q;
    q.head = NULL;
    q.tail = NULL;

    return q;
}


bool enqueue(struct queue *q,int value){

    struct node *newnode = malloc(sizeof(struct node));
    newnode->value = value;
    newnode->next  = NULL;

    if (q->tail != NULL){
        q->tail->next = newnode;
    }

    q->tail = newnode;

    if (q->head == NULL){
        q->head = newnode;
    } 

    return true;
}

int dequeue(struct queue *q){
    // check if queue is empty
    if (q->head == NULL) return -1;

    // save the head of the queue
    struct node *tmp = q->head;

    // result
    int result = tmp->value;

    // remove the value
    q->head = q->head->next;

    if (q->head == NULL){
        q->tail = NULL;
    }

    free(tmp);

    return result;
}

void printQueue(struct queue q){
    printf("[");
    while (q.head != NULL){
        printf("%d",q.head->value);
        q.head = q.head->next;
        if (q.head != NULL) printf(",");
    }
    printf("]\n");
}

int getLastQueueValue(struct queue q){
    //printf("ikan");
    while (q.head != NULL){  
        q.head = q.head->next;
    }

    return q.tail->value;
    
}

/////////////////////////////////
/// CACHE IMPLEMENTATION
/////////////////////////////////

// implement cache_block as linked list of data
// valid bit, 0 - invalid , 1 - valid

struct cache_block{

    int tag;
    uint8_t valid_bit;
    int* data; // array in heap

};

// implement Cache as linked list of cache_blocks
struct Cache{

    int curr_cache_size;
    int block_num;

    struct cache_block* blocks;

};

/// Initializing cache for direct mapped and fully associative
struct Cache curr_Cache;

/// Set struct for 2-way-cache
struct set{
    
    struct queue setQueue;
    int curr_set_size;
    struct cache_block* blocks;

};

/// N-Way Cache struct for 2-way-cache
struct n_way_cache{

    int n_way;
    int block_num;

    struct set* sets;
};

/// Initializing two-way-cache
struct n_way_cache curr_two_way_Cache;

//////////////////////////////////////
/// Helper Functions
//////////////////////////////////////

/// Calculate number of bits for offset
int bits_for_offset(int block_size){
    return (int) (log10(block_size)/log10(2));
}

/// Calculate number of bits for index
int bits_for_index(int cache_type,int cache_size,int block_size){
    
    int i = 0;
    int ind = 1;
    int blocknum = cache_size/block_size;
   // printf("blocknum : %d\n",blocknum);

    while(ind != blocknum){
        i++;
        ind *= 2;
    }
    if (cache_type == CACHE_TYPE_2_WAY) return i-1;
    else return i;
}

/// Calculate number of bits for tag
int bits_for_tag(int cache_type,int cache_size,int block_size){
    
    
    int bit_for_index = bits_for_index(cache_type,cache_size,block_size);
   
    int bit_for_offset = bits_for_offset(block_size);
   
    int bits_for_tag = 32 - (bit_for_offset + bit_for_index);
   
    if (cache_type == CACHE_TYPE_FULLY_ASSOC) bits_for_tag = 32-bit_for_offset;
    
    return bits_for_tag;

}

void bringToLastArray(int* array,int value){

    int i = 0;
    while (array[i] != value){
        i++;
        //printf("AYAM\n");
    }

    while (array[i+1] != -1){
        int tmp = array[i+1];
        array[i+1] = array[i];
        array[i] = tmp;
        i++;
        
    }
}

///////////////////////////////////////////////
/// Initialisation function for each struct
///////////////////////////////////////////////

void init_LRU_policy(int* lruPolicy, int size){
    for (int i = 0; i < size ; ++i){
        lruPolicy[i] = i;
    }
    lruPolicy[size] = -1;
}

struct queue create_queue(int block_num){
    struct queue q;
    q = init_queue();
    for (int i = 0 ; i < block_num ; i++){
        enqueue(&q,i);
    }
    return q;
}

struct cache_block createBlock(){

    struct cache_block block;

    block.tag = 0;
    block.data = (int*) malloc(num_of_data*sizeof(int));
    block.valid_bit = 0;
  

    return block;
}

struct Cache createCache(int cache_type){

    struct Cache cache;
    
    if (cache_type == CACHE_TYPE_DIRECT || cache_type == CACHE_TYPE_FULLY_ASSOC){
    
        cache.block_num = cache_size/block_size;
        cache.blocks =  malloc((cache.block_num)*sizeof(struct cache_block));
        cache.curr_cache_size = 0;
        for (int i = 0 ; i < cache.block_num ; i++){
            cache.blocks[i] = createBlock() ;
        }
        
    }

    return cache;
             
}
struct set create_set(int n){

    struct set s;
   
    s.blocks =  malloc(n*sizeof(struct cache_block));
    s.curr_set_size = 0;
    s.setQueue = create_queue(n);
    for (int i = 0 ; i < n ; i++){
        s.blocks[i] = createBlock();
    }
   
   return s;
  
}
struct n_way_cache create_n_way_cache(int n){

    struct n_way_cache cache;

    cache.n_way = n;
    cache.block_num = (cache_size)/(block_size*n);
    cache.sets =  malloc((cache.block_num)*sizeof(struct set));
    
    for (int i = 0 ; i < cache.block_num ; i++){
        cache.sets[i] = create_set(n) ;
    }

    return cache;

}

/////////////////////////////////////////////
/// Memory state functions
/////////////////////////////////////////////

void memory_state_init(struct architectural_state *arch_state_ptr) {
   
    arch_state_ptr->memory = (uint32_t *) malloc(sizeof(uint32_t) * MEMORY_WORD_NUM);
  
    memset(arch_state_ptr->memory, 0, sizeof(uint32_t) * MEMORY_WORD_NUM);
   
    if (cache_size == 0) {
        // CACHE DISABLED
        memory_stats_init(arch_state_ptr, 0); // WARNING: we initialize for no cache 0
    } else {
        // CACHE ENABLED
        //assert(0); /// @students: remove assert and initialize cache
        /// @students: memory_stats_init(arch_state_ptr, X); <-- fill # of tag bits for cache 'X' correctly
        
        int bits_for_cache_tag;
  
        bits_for_cache_tag = bits_for_tag(cache_type,cache_size,block_size);
    
        memory_stats_init(arch_state_ptr, bits_for_cache_tag);
       
        switch(cache_type) {
            case CACHE_TYPE_DIRECT: // direct mapped

                curr_Cache = createCache(cache_type);

                break;
            case CACHE_TYPE_FULLY_ASSOC: // fully associative
               
                LRUpolicy = malloc(((cache_size/block_size) + 1)*sizeof(int));
                init_LRU_policy(LRUpolicy,(cache_size/block_size));
                curr_Cache = createCache(cache_type);

                break;
            case CACHE_TYPE_2_WAY: // 2-way associative

                
                curr_two_way_Cache = create_n_way_cache(2);

                break;
        }
    }
}


// returns data on memory[address / 4]
int memory_read(int address){
  
    arch_state.mem_stats.lw_total++;
    check_address_is_word_aligned(address);
    
    if (cache_size == 0) {
        // CACHE DISABLED
        return (int) arch_state.memory[address / 4];
    } else {
        // CACHE ENABLED
        //assert(0); /// @students: Remove assert(0); and implement Memory hierarchy w/ cache
     
        /// @students: your implementation must properly increment: arch_state_ptr->mem_stats.lw_cache_hits
        
        switch(cache_type) {
            case CACHE_TYPE_DIRECT: // direct mapped
                return direct_mapped_cache_read(address);
                break;
            case CACHE_TYPE_FULLY_ASSOC: // fully associative
                return fully_assoc_cache_read(address);
                break;
            case CACHE_TYPE_2_WAY: // 2-way associative
                return two_way_cache_read(address);
                break;
        }
    }
    return 0;
}

// writes data on memory[address / 4]
void memory_write(int address, int write_data) {

    arch_state.mem_stats.sw_total++;
    check_address_is_word_aligned(address);

    if (cache_size == 0) {
        // CACHE DISABLED
        arch_state.memory[address / 4] = (uint32_t) write_data;

    } else {
        // CACHE ENABLED
        //assert(0); /// @students: Remove assert(0); and implement Memory hierarchy w/ cache
        arch_state.memory[address / 4] = (uint32_t) write_data;
        /// @students: your implementation must properly increment: arch_state_ptr->mem_stats.sw_cache_hits
        
        switch(cache_type) {
            case CACHE_TYPE_DIRECT: // direct mapped
                direct_mapped_cache_write(address,write_data);
                break;
            case CACHE_TYPE_FULLY_ASSOC: // fully associative
                fully_assoc_cache_write(address,write_data);
                break;
            case CACHE_TYPE_2_WAY: // 2-way associative
                two_way_cache_write(address,write_data);
                break;
        }
    }
}


///////////////////////////////////////////////
/// Cache functions 
///////////////////////////////////////////////

/*---------------------------------------------------
    /------- DIRECT MAPPED CACHE FUNCTIONS ----------
    /---------------------------------------------------*/
int direct_mapped_cache_read(int address){
    
    check_address_is_word_aligned(address);

    int bits_for_tag = arch_state.bits_for_cache_tag;

    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int block_index = get_piece_of_a_word(address,bits_for_offset(block_size),bits_for_index(cache_type,cache_size,block_size));
    //printf("offset : %d \nindex : %d \ntag : %d\n",block_offset,block_index,block_tag);
    struct cache_block test_block = curr_Cache.blocks[block_index];
    bool valid = (test_block.valid_bit == 1);
    
    if ((test_block.tag == block_tag) && valid){
        //printf("Hit\n");
        arch_state.mem_stats.lw_cache_hits++;
        //printf("req data : %d \n",test_block.data[block_offset]);
        return test_block.data[block_offset];
    }
    else {
        //printf("Miss\n");
        return direct_mapped_cache_read_miss(address);
    }
}

int direct_mapped_cache_read_miss(int address){
    

    int offset = 0;
    int bits_for_tag = arch_state.bits_for_cache_tag;
    //printf("address : %d \n", address);

    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int block_index = get_piece_of_a_word(address,bits_for_offset(block_size),bits_for_index(cache_type,cache_size,block_size));
    int block_tag   = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    
    int required_memory = 0;
    curr_Cache.blocks[block_index].tag = block_tag;
    curr_Cache.blocks[block_index].valid_bit = 1;
    int baseAddOffset = address/4 - block_offset;

    for (offset = 0 ; offset < num_of_data ; ++offset){
    
        uint32_t read_data = (int) arch_state.memory[baseAddOffset + offset];
        curr_Cache.blocks[block_index].data[offset] = read_data;   
        //printf("index: %d, offset: %d, read_data: %d \n",block_index,offset,read_data);
        if (offset == block_offset){
            required_memory = read_data;
        }

    }
    //printf("req mem: %d \n", required_memory);
    return required_memory;
}

void direct_mapped_cache_write(int address,int write_data){

    int bits_for_tag = arch_state.bits_for_cache_tag;

    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int block_index = get_piece_of_a_word(address,bits_for_offset(block_size),bits_for_index(cache_type,cache_size,block_size)) ;

    struct cache_block test_block = curr_Cache.blocks[block_index];
    bool valid = test_block.valid_bit;

    if ((test_block.tag == block_tag) && valid){
        printf("Write Hit\n");
        curr_Cache.blocks[block_index].data[block_offset] = write_data;
        arch_state.mem_stats.sw_cache_hits++;
    
    }
    else {
        //printf("Write Miss\n");
    }
    
    
}

/*-------------------------------------------------------
    /------- FULLY ASSOCIATIVE CACHE FUNCTIONS ----------
    /-------------------------------------------------------*/

int fully_assoc_cache_read(int address){

    check_address_is_word_aligned(address);
    //printf("REAd\n");
    int bits_for_tag = arch_state.bits_for_cache_tag;

    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    
    //printf("offset : %d \nindex : %d \ntag : %d\n",block_offset,block_index,block_tag);
    
    //printf("address: %d, Read block tag: %d\n",address,block_tag);
    for (int i = 0 ; i < curr_Cache.block_num ; i++){
        struct cache_block test_block = curr_Cache.blocks[i];
        
        int test_tag = test_block.tag;
        bool valid = test_block.valid_bit;
        
        if ((test_tag == block_tag) && valid){
            //printf("Hit\n");
            arch_state.mem_stats.lw_cache_hits++;
            
            bringToLastArray(LRUpolicy,i);
        
            return test_block.data[block_offset];
        }
    }

    //printf("Miss\n");
    return fully_assoc_cache_read_miss(address);
    
}

int fully_assoc_cache_read_miss(int address){

    int offset = 0;
    int bits_for_tag = arch_state.bits_for_cache_tag;
    int cur_size = curr_Cache.curr_cache_size;

    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int block_tag   = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);

    int required_memory = 0;
    //printf("Read miss block tag: %d\n",block_tag);
    
    //printf("Removed from queue: %d\n",removed_index);
    if (cur_size < curr_Cache.block_num){
        
        curr_Cache.blocks[cur_size].tag = block_tag;
        curr_Cache.blocks[cur_size].valid_bit = 1;
        int baseAddOffset = address/4 - block_offset;
    
        for (offset = 0 ; offset < num_of_data ; ++offset){

            uint32_t read_data = (int) arch_state.memory[baseAddOffset + offset];
    
            //printf("offset: %d ,cur size : %d \n",offset,cur_size);
            curr_Cache.blocks[cur_size].data[offset] = read_data;   
         
            if (offset == block_offset){
                required_memory = read_data;
            }

        }
        //printf("AYAM");
       
        bringToLastArray(LRUpolicy,cur_size);
        
        //enqueue(&LRUqueue,cur_size);
        curr_Cache.curr_cache_size++;
        //printf("req mem: %d \n", required_memory);
        //printf("%d added Read MissFF\n", cur_size);
        
        
    }
    else { 
       
        int removed_index = LRUpolicy[0];
        //printf("Removed_index : %d" , removed_index);
        curr_Cache.blocks[removed_index].tag = block_tag;
        
        curr_Cache.blocks[removed_index].valid_bit = 1;
  
        int baseAddOffset = address/4 - block_offset;
     
        for (offset = 0 ; offset < num_of_data ; ++offset){
        
            uint32_t read_data = (int) arch_state.memory[baseAddOffset + offset];
            //printf("OK1\n");
            curr_Cache.blocks[removed_index].data[offset] = read_data;   
            //printf("OK2\n");
            if (offset == block_offset){
                required_memory = read_data;
            }

        }
        
        bringToLastArray(LRUpolicy,removed_index);
       
        //printf("%d added Read Miss\n", removed_index);
        //printf("req mem: %d \n", required_memory);
       
    }
    
    return required_memory;
}

void fully_assoc_cache_write(int address,int write_data){

    int bits_for_tag = arch_state.bits_for_cache_tag;

    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    
    //printf("address: %d , write block tag: %d\n",address,block_tag);
    for (int i = 0 ; i < curr_Cache.block_num ; i++){

        struct cache_block test_block = curr_Cache.blocks[i];
        
        int test_tag = test_block.tag;
        bool valid = test_block.valid_bit;
        //if (valid == 1) printf("test tag : %d \n",test_tag );
        if (valid && (test_tag == block_tag)){
            //printf("Write Hit\n");
            curr_Cache.blocks[i].data[block_offset] = write_data;
            arch_state.mem_stats.sw_cache_hits++;
            bringToLastArray(LRUpolicy,i);
        }
    }

}

/*-------------------------------------------------------------
    /------- TWO-WAY SET ASSOCIATIVE CACHE FUNCTIONS ----------
    /-------------------------------------------------------------*/

int two_way_cache_read(int address){
   
    int bits_for_tag = arch_state.bits_for_cache_tag;
    int n_way = curr_two_way_Cache.n_way;
  
    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int set_index = get_piece_of_a_word(address,bits_for_offset(block_size),bits_for_index(cache_type,cache_size,block_size));
    
    //printf("offset : %d \nindex : %d \ntag : %d\n",block_offset,block_index,block_tag);
    
    struct set test_set = curr_two_way_Cache.sets[set_index];
    
    for (int i = 0 ; i < n_way ; ++i){
    
        struct cache_block test_block = curr_two_way_Cache.sets[set_index].blocks[i];
       
        bool valid = test_block.valid_bit;
      
        if ((test_block.tag == block_tag) && valid){

            //printf("Hit\n");
            int test_queue = getLastQueueValue(curr_two_way_Cache.sets[set_index].setQueue);
            if (test_queue != i){
                dequeue(&(curr_two_way_Cache.sets[set_index].setQueue));
                enqueue(&(curr_two_way_Cache.sets[set_index].setQueue),i);
            }
            //enqueue( &(curr_two_way_Cache.sets[set_index].setQueue) , i );
            arch_state.mem_stats.lw_cache_hits++;
            return test_block.data[block_offset];

        }
    }
    
    //printf("Miss\n");
    return two_way_cache_read_miss(address);
    
}

int two_way_cache_read_miss(int address){

    int bits_for_tag = arch_state.bits_for_cache_tag;
    int n_way = curr_two_way_Cache.n_way;
    
    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int set_index = get_piece_of_a_word(address,bits_for_offset(block_size),bits_for_index(cache_type,cache_size,block_size));
    
    //printf("offset : %d \nindex : %d \ntag : %d\n",block_offset,block_index,block_tag);
    
    struct set test_set = curr_two_way_Cache.sets[set_index];

    int set_size = test_set.curr_set_size;
    
    int required_memory;
    int offset = 0;
    
    if (set_size < n_way){

        curr_two_way_Cache.sets[set_index].blocks[set_size].tag = block_tag;
        curr_two_way_Cache.sets[set_index].blocks[set_size].valid_bit = 1;

        int baseAddOffset = address/4 - block_offset;
        for (offset = 0 ; offset < num_of_data ; ++offset){
        
            uint32_t read_data = (int) arch_state.memory[baseAddOffset + offset];
            curr_two_way_Cache.sets[set_index].blocks[set_size].data[offset] = read_data;   
            
            if (offset == block_offset){
                required_memory = read_data;
            }

        }
        //printf("set size : %d\n",set_size);
        enqueue(&(curr_two_way_Cache.sets[set_index].setQueue),set_size);
        curr_two_way_Cache.sets[set_index].curr_set_size++;
        //printf("req mem: %d \n", required_memory);
    }
    else {
       
        int removed_block_index = dequeue(&(curr_two_way_Cache.sets[set_index].setQueue));
    
        if (removed_block_index == -1) removed_block_index = 0;

        curr_two_way_Cache.sets[set_index].blocks[removed_block_index].tag = block_tag;
       
        curr_two_way_Cache.sets[set_index].blocks[removed_block_index].valid_bit = 1;
       
        int baseAddOffset = address/4 - block_offset;
        for (offset = 0 ; offset < num_of_data ; ++offset){
          
            uint32_t read_data = (int) arch_state.memory[baseAddOffset + offset];
     
            //printf("block num: %d, removed : %d , set index : %d\n",n_way,removed_block_index,set_index);
            curr_two_way_Cache.sets[set_index].blocks[removed_block_index].data[offset] = read_data;   
            
            if (offset == block_offset){
                required_memory = read_data;
            }

        }
     
        enqueue(&(curr_two_way_Cache.sets[set_index].setQueue),removed_block_index);
      
        //printf("req mem: %d \n", required_memory); 
    }

    
    return required_memory;
}

void two_way_cache_write(int address,int write_data){

    int bits_for_tag = arch_state.bits_for_cache_tag;
    int n_way = curr_two_way_Cache.n_way;
    
    int block_tag = get_piece_of_a_word(address,32-bits_for_tag,bits_for_tag);
    int block_offset = get_piece_of_a_word(address,0,bits_for_offset(block_size)) >> 2;
    int set_index = get_piece_of_a_word(address,bits_for_offset(block_size),bits_for_index(cache_type,cache_size,block_size));
    
    //printf("offset : %d \nindex : %d \ntag : %d\n",block_offset,block_index,block_tag);
    
    struct set test_set = curr_two_way_Cache.sets[set_index];

    for (int i = 0 ; i < n_way ; ++i){

        struct cache_block test_block = test_set.blocks[i];
        bool valid = test_block.valid_bit;

        if ((test_block.tag == block_tag) && valid){
            //printf("Hit\n");
            
            arch_state.mem_stats.sw_cache_hits++;
            curr_two_way_Cache.sets[set_index].blocks[i].data[block_offset] = write_data;
            int test_queue = getLastQueueValue(curr_two_way_Cache.sets[set_index].setQueue);
            if (test_queue != i){
                dequeue(&(curr_two_way_Cache.sets[set_index].setQueue));
                enqueue(&(curr_two_way_Cache.sets[set_index].setQueue),i);
            }
        }
    }
}