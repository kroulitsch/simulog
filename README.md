# Simulog

Simulog is a digital logic library written in C. It allows users to build, interconnect, simulate and visualize digital circuits ranging from basic logic gates (AND, NOT) to complex composite blocks.

## Implementation details

The **entire project** is implemented in pure **C**, with no external overhead, although graphic visualization is planned in the [future](#future-works).

In the library, three main structures are used:
* `Pin` - Represents input and output pins, including the main inputs and outputs. The pins are differentiated as input and output pins. They can be named, flipped on and off and connected to other pins (output to input).

* `Operation` - Represents operation to be executed inside the basic logic gate to calculate the corresponding output of the gate. This structure can be named and given a function pointer in the following form:

    ```C
    bool (*func) (Pin *, Pin *); // function that takes two 'Pin' pointers and returns bool
    ```

    The `AND` function is used as and example:

    ```C
    bool and(Pin *a, Pin *b) { return a->on && b->on; } // true only if both 'a' and 'b' are on
    ```

* `Block` - Composition of the pins and the operation or inside components (other blocks). This structure can be named, can specify number of I/O pins, its inside blocks and connections. The block can also be specified using the basic gate operation, which the block then uses to calculate its output. In that case, the block must have at most two inputs and exactly one output.

## Working with the library
The library can be built using:
```bash
make
```

To run the simulation, use:
```bash
make run
```

To profile the binary and check for memory leaks using `valgrind`, use:
```bash
make v
```

To clean compiled binaries and object artifacts, use:
```bash
make clean
```

## Parser and file format

Apart from building the blocks manually in code, a `.simulog` file parser is available to use using the `parseFile()` function.
To correctly parse a file, it has to follow a format specified below:

```simulog
# this is a comment

# each block has to start with the 'block:' section, setting the block's name
block: XOR
# the 'block:' section is followed with the 'in:' and 'out:' sections,
# naming the inputs and outputs
    in: A, B
    out: Q

# 'components:' section follows, specifying inside components of the block,
# in the form *component_name* = *block_type*
    components:
        NOT_A = NOT
        NOT_B = NOT
        AND_1 = AND
        AND_2 = AND
        OR_FINAL = OR

# the last section is the 'connections:' section, setting up connections of pins
# in the form from_component.from_pin -> to_component.to_pin
    connections:
        # to reference the block's I/O pins, the component part is left out
        A -> NOT_A.IN
        B -> NOT_B.IN

        A -> AND_1.IN1
        NOT_B.OUT -> AND_1.IN2

        NOT_A.OUT -> AND_2.IN1
        B -> AND_2.IN2

        AND_1.OUT -> OR_FINAL.IN1
        AND_2.OUT -> OR_FINAL.IN2

        OR_FINAL.OUT -> Q

# after the last section, the next block specification can start
# block: ...
# in: ...
# ...
```

As a part of the `parseFile` function call, the database array of existing blocks is passed optionally, from which the components can be referenced. If a `NULL` pointer is passed or `the dbCount` is passed as `0`, the database of basic functions is imported and used. The function returns an array of the parsed and database blocks as one combined array.

## Future works
Planned future updates include:
* **Cycles handling:** Currently the calculation only allows acyclic circuits, adding cycles support would unlock new possibilities, including sequential circuits.
* **Graphic visualization:** Graphic front-end for better visualization and easier manipulation of the blocks.
* **Multi-bit pins support:** Pins that represent a binary number rather than just two states. This could make working with bigger units, such as a memory unit easier.

## Author
* **Jan Kroulík** - Main Developer
    * GitHub: [kroulitsch](https://github.com/kroulitsch)
    * Contact: xkroulj00@vutbr.cz