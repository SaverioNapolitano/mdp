# Troubleshooting 

Main errors that happened to me while solving the exercises:
- Check to have opened the file in binary mode (`std::ios::binary`)
- Check for **all** special cases
- When using the bitwriter, be careful to use **only** the bitwriter to write on a file (or, if you want to use `put` and/or `write`, be sure the bytes are *aligned*)
    - If the buffer is *not full* bitwriter **does not** write
    - Bitwriter operates on bit size, **be explicit** (**do not** use `sizeof(char)` since it will assume a value of 1)
- Remember that strings are *0-terminated* (if you don't want the 0 at the end write just the `len - 1`) or use the `<<` operator
- Don't implement the decoder based on *your* encoder (it might be biased)
    - Try to implement the decoder as if the encoder was done by someone else to avoid error propagation
- `assert` is your friend (use it to avoid debugging by hand to find errors with large outputs/inputs)
    - Look for **off-by-one errors**, **buffer-circularity** etc
        - Buffer circularity: check for both `i > max_dict_capacity` (in that case, `i`, which is the index, should restart from the beginning, e.g. 0) and for `offset > size` where `size` is the current number of element in the dictionary (in that case, you should add to `i` `max_dict_capacity` until it gets positive)
    - **Always cast `char` to `uint8_t` before doing operations with it**
- To insert one character in a string **the only way** is to use `append(size_t count, char c)` 
    - **Do not** try the version with `char *` or you'll cry 
- When dealing with images, read **exactly the number of bytes you need to read** (use height, width and if necessary depth)
    - **Do not** rely on the input stream check
- When dealing with images, **compare the index you used to insert data with the expected size to be sure they match**
- When dealing with images, if you encounter an error, it's probably because you **misunderstood something about the standard: check carefully**
- **Always initialize strings** when you create them (for instance, `std::string str(size, ' ')`) and remember to resize them using the corresponding function (`str.resize(new_size)`) if you didn't use strings' own methods (for instance, if you used `is.read(str.data(), new_size)`)
- **Be careful when choosig what to use between `double` and `float`** (read the exam text carefully)
- **Do not initialize vectors' capacity** unless you plan to use them with index access or `algorithm` functions (if you use `push_back` do not give an initial size)
    - If you want to use `algorithm` functions then you **must** initialize the vector's capacity
- Reading the file content using `istream_iterator`can be done only if the file is opened in textual mode or if the data in binary are still compatible with operator `>>`
- When using `is.read(char *, size)` and `os.write(char *, size)` remember that `size` is the number of **bytes** to read/write
    - If you have a `vector<T> v` you need to specify `size` as `v.size() * sizeof(T)` to write all its content
