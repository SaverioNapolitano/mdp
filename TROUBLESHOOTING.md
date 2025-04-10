# Troubleshooting 

Main errors that happened to me while solving the exercises:
- Check to have opened the file in binary mode (`std::ios::binary`)
- Check for **all** special cases
- When using the bitwriter, be careful to use **only** the bitwriter to write on a file (or, if you want to use `put` and/or `write`, be sure the bytes are *aligned*)
    - If the buffer is *not full* bitwriter **does not** write
    - Bitwriter operates on bit size, **be explicit** (**do not** use `sizeof(char)` since it will assume a value of 1)
- Remember that strings are *0-terminated* (if you don't want the 0 at the end write just the `len - 1`)
- Don't implement the decoder based on *your* encoder (it might be biased)
    - Try to implement the decoder as if the encoder was done by someone else to avoid error propagation
- **Avoid** `byteswap` (and in general any **c++23** feature)
    - Valid at the time I'm writing this file, maybe in the future it won't be a problem
- `assert` is your friend (use it to avoid debugging by hand to find errors with large outputs/inputs)
    - Look for **off-by-one errors**, **buffer-circularity** etc
    - **Always cast `char` to `uint8_t` before doing operations with it**
