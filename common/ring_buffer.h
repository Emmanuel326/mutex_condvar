struct ring_buffer {
    void   **buffer;
    size_t   capacity;
    size_t   size;
    size_t   head;
    size_t   tail;
};

