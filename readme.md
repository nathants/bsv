## why

it should be possible to process csv at speeds approaching that of network io.

## what

a number of small c utilities for sequential access processing of csv at bare metal speeds. they are meant to be combined into processing pipelines.

sanity of the c programs is ensured by generative testing from python with hypothesis. this helps shake out all the corner cases of c, like complex buffer handling, and increase confidence in general correctness of implementation.

## brief aside on generative testing

generative testing has become an ideal design tool for all serious work. by modeling your system twice, once naively and once production ready, you get a number of things.

first, you get to test your system against a naive rewrite by running millions of valid inputs through both systems. if the two implementations don't agree, you probably have a bug.

second, you get to build your system twice. as the two systems converge on a single specification, you are inescapably faced with your problem domain. any ambiguity is revealed as a failure of convergence.
