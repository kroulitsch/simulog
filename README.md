# Simulog

Simulog is a digital logic simulator written in C. It allows users to build, interconnect, simulate and visualize digital circuits ranging from basic logic gates (AND, NOT) to complex composite blocks.

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

## Future works

In the current state, the library contains the three main structures and their corresponding functions. The pin state calculation is performed recursively upon each input change, therefore in the more complex scenarios some blocks can be recomputed many times over and over, slowing down the simulation.

Planned future updates include:
* **Calculation optimization:** As referenced above, calculation update and optimization could improve the simulation performance and enable cycles in the circuits.
* **Graphic visualization:** Graphic front-end for better visualization and easier manipulation of the blocks.
* **Multi-bit pins support:** Pins that represent a binary number rather than just two states. This could make working with bigger units, such as a memory unit easier.

## Author
* **Jan Kroulík** - Main Developer
    * GitHub: [kroulitsch](https://github.com/kroulitsch)
    * Contact: xkroulj00@vutbr.cz