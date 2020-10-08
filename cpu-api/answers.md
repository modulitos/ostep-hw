1.
`./fork.py -s 10`
a
-b
-c (exited)
-d
-e

2.
Run the simulator with a large number of actions (e.g.,-a100) and vary the `forkpercentage` from 0.1 to 0.9.
What do you think the resulting final process trees will look like as the percentage changes?

This higher the `forkpercentage`, the larger the tree gets.

3.
`./fork.py -t`
Given a set of process trees, can you tell which actions were taken?
yes.

4.
`./fork.py -A a+b,b+c,c+d,c+e,c-`
What happens when `c` exits? What should the process tree look like?

The processes d and e are orphaned, and get adopted by the init process, a (re-parenting)

-R seems to have orphans adopted by the next parent process, b, instead of the init process, a.

5.
Try with -F to eval the final tree.
`./fork.py -s 28 -F`
a
-b
 -d
-c
-e
 -f

`./fork.py -s 32 -F`
a
-b
-d
-e

6.
`./fork.py -s 36 -F -t`

use `-t` flag with `-F` to determine the steps taking place.
a
-c
 -d
 -e

action1: a forks b
action2: b exits
action3: a forks c
action4: c forks d
action5: c forks e
