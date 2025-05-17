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
- **Avoid** `byteswap` (and in general any **c++23** feature)
    - Valid at the time I'm writing this file, maybe in the future it won't be a problem
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
- **Always initialize strings** when you create them (for instance, `std::string str(size, ' ')`) and remember to resize it using the corresponding function (`str.resize(new_size)`)
- **Always use `double` instead of `float`**
