# Troubleshooting 

Main errors that happened to me while solving the exercises:
- Check to have opened the file in binary mode (`std::ios::binary`)
- Check for **all** special cases
- When using the bitwriter, be careful to use **only** the bitwriter to write on a file (or, if you want to use `put` and/or `write`, be sure the bytes are *aligned*)
    - If the buffer is *not full* bitwriter **does not** write
    - Bitwriter operates on bit size, **be explicit** (**do not** use `sizeof(char)` since it will assume a value of 1)
- Remember that strings are *0-terminated* (if you don't want the 0 at the end write just the `len - 1`)
