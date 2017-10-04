a number of small c utilities for sequential access processing of sorted and unsorted csv at bare metal speeds. they attempt to answer the question, if io is now faster than cpu, how fast can data be processed, and can it scale linearly? spoiler alert, turns out c is fast and constant factors matter a lot.

the results are exciting, and ongoing. sanity of the c programs is ensured by generative testing from python with hypothesis. this helps shake out all the corner cases of c, like complex buffer handling.

generative testing, ie quickcheck, has become an ideal design tool for any serious system. by modeling your system twice, once naively and once in a production ready manner, you get a number of things.

first, you get to test your system against what is effectively a naive rewrite by running thousands or millions of valid inputs through both systems. if the two implementations don't agree, you probably have a bug.

second, you get to build your system twice. as the two systems converge on a single specification, you are inescapably faced with your problem domain. any ambiguity in one's understanding of the domain, or in the specification, are revealed as a failure of convergence.

to sum up, things that matter:
 - constant factors
 - solid understanding of the domain, specification, and implementation[s]
 - correctness of implementation[s]

there is some exploration of performance and alternatives [here](https://github.com/nathants/c-utils/tree/master/rcut.alternatives).
