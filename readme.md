## why

it should be possible to process csv at speeds approaching that of network io. this would be ideal for [mapreduce the hard way](https://github.com/nathants/py-aws/blob/master/readme.md#more-what-aka-mapreduce-the-hard-way).

## what

a number of small c utilities for sequential access processing of sorted and unsorted csv at bare metal speeds. they are meant to be combined into processing pipelines.

sanity of the c programs is ensured by generative testing from python with hypothesis. this helps shake out all the corner cases of c, like complex buffer handling, and increase confidence in general correctness of implementation.

there is some exploration of performance and alternatives [here](https://github.com/nathants/c-utils/tree/master/rcut.alternatives).

## install

note: tested only on ubuntu

```
git clone https://github.com/nathants/c-utils
cd c-utils
make
```

## brief aside on generative testing

generative testing, ie quickcheck, has become an ideal design tool for any serious system. by modeling your system twice, once naively and once in a production ready manner, you get a number of things.

first, you get to test your system against what is effectively a naive rewrite by running thousands or millions of valid inputs through both systems. if the two implementations don't agree, you probably have a bug.

second, you get to build your system twice. as the two systems converge on a single specification, you are inescapably faced with your problem domain. any ambiguity in one's understanding of the domain, or in the specification, are revealed as a failure of convergence.
