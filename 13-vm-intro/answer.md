1. The first Linux tool you should check out is the very simple toolfree. First, typeman freeand read its entire manual page; it’s short, don’t worry!

(using vm_stat on macos, which shows memory in KB)
this script helps:
https://gist.github.com/vigo/1690045


2. Now, runfree, perhaps using some of the arguments that mightbe useful (e.g.,-m, to display memory totals in megabytes). Howmuch memory is in your system? How much is free? Do thesenumbers match your intuition?

Installed memory: 16.38G
Free memory: 2.28G


3. Next, create a little program that uses a certain amount of memory, called `memory-user.c`. This program should take one commandline argument: the number of megabytes of memory it will use. When run, it should allocate an array, and constantly stream through the array, touching each entry. The program should do this indefinitely, or, perhaps, for a certain amount of time also specified at the command line.


4. Now, while running your memory-user program, also (in a different terminal window, but on the same machine) run the `free` tool. How do the memory usage totals change when your program is running? How about when you kill the memory-user program? Do the numbers match your expectations?  Try this for different amounts of memory usage. What happens when you use really large amounts of memory?

When the program runs, the amount of free memory is reduced, presumably because it is being allocated to the process by the OS. When the program is stopped, the memory is free and added to the free memory in the OS. This matches my expectations.

For very large amounts of memory, the program allocates more memory that was not previously free. Eg:

starting memory:
> ❯ free_macos
> Installed memory: 16.38G
> Free memory: 3.78G

run the program, consuming 3GB for 5 seconds:
> ❯ ./memory-user.o 3000 5

while running:
> ❯ free_macos
> Installed memory: 16.38G
> Free memory: 1.68G

(note how there is still free memory left!)

Once the program has stopped:
> ❯ free_macos
> Installed memory: 16.38G
> Free memory: 4.86G

It seems like an extra GB was free by the OS to accommodate our process!


5. Let’s try one more tool, known as `pmap`. Spend some time, and readthepmapmanual page in detail.


6. To usepmap, you have to know theprocess IDof the process you’reinterested in. Thus, first runps auxwto see a list of all processes;then, pick an interesting one, such as a browser. You can also useyourmemory-userprogram in this case (indeed, you can evenhave that program callgetpid()and print out its PID for yourconvenience).


7. Now runpmapon some of these processes, using various flags (like-X) to reveal many details about the process. What do you see?How many different entities make up a modern address space, asopposed to our simple conception of code/stack/heap?


8. Finally, let’s runpmapon yourmemory-userprogram, with dif-ferent amounts of used memory. What do you see here? Does theoutput frompmapmatch your expectations?
